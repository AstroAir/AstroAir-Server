/*
 * air_ieqpro.cpp
 * 
 * Copyright (C) 2020-2021 Max Qian
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  ifnot, see <https://www.gnu.org/licenses/>.
 */
 
/************************************************* 
 
Copyright: 2020-2021 Max Qian. All rights reserved
 
Author:Max Qian

E-mail:astro_air@126.com
 
Date:2021-7-18
 
Description:IEQ Mount Offical Port

**************************************************/

#include "air_ieqpro.h"
#include "../../logger.h"
#include "../../tools/ConvertTools.h"
#include "../air_nova.h"

#include <string.h>
#include <libnova/julian_day.h>
#include <thread>
#include <chrono>

#define MAXRBUF 2048

static const uint8_t DRIVER_TIMEOUT { 3 };      //赤道仪超时时间
static const uint8_t DRIVER_LEN { 64 };         //信息长度
static const char DRIVER_STOP_CHAR { '#' };     //默认指令前置

namespace AstroAir
{
    
    IEQPRO::IEQPRO(MountInfo *NEW)
    {
        IEQInfo = NEW;
    }

    IEQPRO::~IEQPRO()
    {
        if(IEQInfo->isMountConnected)
            Disconnect();
    }

    /*
     * name: Connect(std::string Device_name)
     * @param Device_name:连接赤道仪名称
     * describe: Connect the mount
     * 描述： 连接赤道仪
     * calls: TTY->Open()
     * calls: TTY->Initialization()
     * calls: GetModel()
     * calls: GetRADECFirmware()
     * calls: GetMainFirmware()
     * calls: IDLog()
     * @return true: filter connected successfully
     * @return false: Failed to connect filter
     * note: Do not connect the mount repeatedly
     */
    bool IEQPRO::Connect(std::string Device_name)
    {
        if(Device_name == "CEM40" || Device_name == "GEM45")
        {
            if(!TTY->Open("/dev/ttyUSB0",115200,8, 2, 2,reinterpret_cast<int*>(m_PortFD)))       //这里需要增加多端口支持
                return false;
        }
        else
        {
            if(!TTY->Open("/dev/ttyUSB0",9600,8, 2, 2,reinterpret_cast<int*>(m_PortFD)))       //这里需要增加多端口支持
                return false;
        }
        if(GetModel())      //获取主板信息
        {
            if(GetRADECFirmware() && GetMainFirmware())     //获取其他板子的信息
            {
                if(IEQInfo->Name == Device_name)     //判断赤道仪是否为需要连接的赤道仪
                {
                    IDLog(_("Found %s\n"),IEQInfo->Name.c_str());
                    for (const auto &oneMount : MountList)
                    {
                        if(oneMount.model == IEQInfo->Name)     //判断赤道仪名称
                        {
                            if(FirmwareInfo.MainBoardFirmware >= oneMount.firmware)      //判断版本是否低于最低版本
                            {
                                UpdateMountConfigure();
                                IEQInfo->isMountConnected = true;
                                return true;
                            }    
                            else
                            {
                                IDLog_Error(_("Main board firmware is %s while minimum required firmware is %s. Please upgrade the mount firmware."),FirmwareInfo.MainBoardFirmware.c_str(), oneMount.firmware.c_str());
                                return false;
                            }
                        }
                    }
                }
                else
                {
                    IDLog_Error(_("Could not found %s,please make sure you've already connected mount\n"),IEQInfo->Name.c_str());
                    return false;
                }
            }
        }
        else
            IDLog_Error(_("Could not get mount's name\n"));
        return false;
    }

    bool IEQPRO::Disconnect()
    {
        return true;
    }

    std::string IEQPRO::ReturnDeviceName()
    {
        return IEQInfo->Name;
    }

    bool IEQPRO::Goto(std::string Target_RA,std::string Target_DEC)
    {
        int temp_h,temp_m,temp_s;
        RAConvert(Target_RA,&temp_h,&temp_m,&temp_s);
        int targetRA = (temp_h *3600 + temp_m * 60 + temp_s) * 1000;
        DECConvert(Target_DEC,&temp_h,&temp_m,&temp_s);
        int targetDEC = (temp_h *3600 + temp_m * 60 + temp_s) * 1000;

        char RAStr[64] = {0}, DecStr[64] = {0};
        fs_sexa(RAStr, targetRA, 2, 3600);
        fs_sexa(DecStr, targetDEC, 2, 3600);
        if(!setRA(targetRA) || !setDE(targetDEC))
        {
            IDLog_Error(_("Error setting RA/DEC.\n"));
            return false;
        }
        if (!slew())
        {
            IDLog_Error(_("Failed to slew.\n"));
            return false;
        }
        MountInfo newInfo;
        for (int i = 0; i < 5; i++)
        {
            bool rc = getStatus(&newInfo);
            if (rc && newInfo.Status == MOUNT_SLEWING)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (newInfo.Status == MOUNT_SLEWING)
        {
            IEQInfo->Status = MOUNT_SLEWING;
            IDLog(_("Slewing to RA: %s - DEC: %s\n"), RAStr, DecStr);
            return true;
        }
        else
        {
            IDLog_Error(_("Mount status failed to update to slewing.\n"));
            return false;
        }
    }

    bool IEQPRO::IsMountPark(bool status)
    {
        if(status)
        {
            if(!isCMDSupported("MP1"))      //判断是否支持归位
                return false;
            char res[DRIVER_LEN] = {0};
            if (SendCMD(":MP1#", res, -1, 1))
                return res[0] == '1';
            return false;
        }
        else
        {
            if (!isCMDSupported("MP0"))
            return false;
            char res[DRIVER_LEN] = {0};
            return SendCMD(":MP0#", res, -1, 1);
        }
        return false;
    }

    bool IEQPRO::IsMountTrack(bool status)
    {
        char res[DRIVER_LEN] = {0};
        return SendCMD(status ? ":ST1#" : ":ST0#", res, -1, 1);
    }

    bool IEQPRO::IsMountAbort(int status)
    {
        if(status == 1)
            return IsMountTrack(false);
        char res[DRIVER_LEN] = {0};
        return SendCMD(":Q#", res, -1, 1);
    }

    bool IEQPRO::ReadScopeStatus()
    {
        double utc_offset;
        int yy, dd, mm, hh, minute, ss;
        if (getUTCDateTime(&utc_offset, &yy, &mm, &dd, &hh, &minute, &ss))
        {
            char isoDateTime[32] = {0};
            snprintf(isoDateTime, 32, "%04d-%02d-%02dT%02d:%02d:%02d", yy, mm, dd, hh, minute, ss);
            IEQInfo->UTCtime = isoDateTime;
        }

        //获取赤道仪所在经纬度
        if(getStatus(IEQInfo))
            if (IEQInfo->longitude < 0)
                IEQInfo->longitude += 360;
        return true;
    }

    bool IEQPRO::UpdateMountConfigure()
    {
        /*检查赤道仪是否支持一下功能*/
        IEQInfo->canParkNatively = isCMDSupported("MP1", true);
        IEQInfo->canFindHome = isCMDSupported("MSH", true);
        IEQInfo->canGuideRate = isCMDSupported("RG", true);

        //更新UTC时间
        double utc_offset;
        int yy, dd, mm, hh, minute, ss;
        if (getUTCDateTime(&utc_offset, &yy, &mm, &dd, &hh, &minute, &ss))
        {
            char isoDateTime[32] = {0};
            char utcOffset[8] = {0};
            snprintf(isoDateTime, 32, "%04d-%02d-%02dT%02d:%02d:%02d", yy, mm, dd, hh, minute, ss);
            snprintf(utcOffset, 8, "%4.2f", utc_offset);
            IEQInfo->UTCtime = isoDateTime;
            IDLog(_("Mount UTC offset is %s. UTC time is %s"), utcOffset, isoDateTime);
        }

        //获取赤道仪所在经纬度
        if(getStatus(IEQInfo))
        {
            if (IEQInfo->longitude < 0)
                IEQInfo->longitude += 360;
            IDLog(_("Mount Longitude %g Latitude %g"), IEQInfo->longitude, IEQInfo->latitude);
        }
        return true;
    }

    bool IEQPRO::setRA(double ra)
    {
        char cmd[DRIVER_LEN] = {0};
        char res[DRIVER_LEN] = {0};
        int ieqValue = static_cast<int>(ra * 60 * 60 * 1000);
        snprintf(cmd, DRIVER_LEN, ":Sr%08d#", ieqValue);
        return SendCMD(cmd, res, -1, 1);
    }

    bool IEQPRO::setDE(double dec)
    {
        char cmd[DRIVER_LEN] = {0};
        char res[DRIVER_LEN] = {0};
        int ieqValue = static_cast<int>(fabs(dec) * 60 * 60 * 100);
        snprintf(cmd, DRIVER_LEN, ":Sd%c%08d#", (dec >= 0) ? '+' : '-', ieqValue);
        return SendCMD(cmd, res, -1, 1);
    }

    bool IEQPRO::slew()
    {
        char res[DRIVER_LEN] = {0};
        if (SendCMD(":MS#", res, -1, 1))
        {
            return res[0] == '1';
        }
        return false;
    }

    /*
     * name: GetModel()
     * describe: Get mount model
     * 描述：获取赤道仪名称
     * calls: SendCMD()
     */
    bool IEQPRO::GetModel()
    {
        char res[64] = {0};
        if(SendCMD(":MountInfo#", res, -1, 4))     //获取赤道仪名称
        {
            std::string code = res;
            /*比较获取赤道仪名称是否为已知的*/
            auto result = std::find_if(MountList.begin(), MountList.end(), [code](const iMountInfo & oneMount)
            {
                return oneMount.code == code;
            });
            if(result != MountList.end())        //如果结果是已知的
            {
                IEQInfo->Name = result->model;
                return true;
            }
            IDLog(_("Mount with code %s is not recognized."),res);      //未知的赤道仪
            return false;
        }
        return false;       //命令发送失败
    }

    /*
     * name: GetMainFirmware()
     * describe: Get mount's main board version
     * 描述：获取赤道仪主板版本
     * calls: SendCMD()
     */
    bool IEQPRO::GetMainFirmware()
    {
        char res[DRIVER_LEN] = {0};
        if(SendCMD(":FW1#", res))
        {
            char board[8] = {0}, controller[8] = {0};
            strncpy(board, res, 6);
            strncpy(controller, res + 6, 6);
            FirmwareInfo.MainBoardFirmware.assign(board, 6);
            FirmwareInfo.ControllerFirmware.assign(controller, 6);
            return true;
        }
        return false;
    }

    /*
     * name: GetRADECFirmware()
     * describe: Get mount's RA and DEC boards' board version
     * 描述：获取赤道仪RA、DEC轴主板版本
     * calls: SendCMD()
     */
    bool IEQPRO::GetRADECFirmware()
    {
        char res[DRIVER_LEN] = {0};
        if(SendCMD(":FW2#", res))      //获取RADEC主板信息
        {
            char ra[8] = {0}, de[8] = {0};
            strncpy(ra, res, 6);
            strncpy(de, res + 6, 6);
            FirmwareInfo.RAFirmware.assign(ra, 6);
            FirmwareInfo.DEFirmware.assign(de, 6);
            return true;
        }
        return false;
    }

    /*
     * name: getStatus(Info *info)
     * @param info: 状态信息
     * describe: Get status information
     * 描述： 获取状态信息
     * calls: SendCMD()
     * calls: strncpy()
     * calls: DecodeString()
     * @return true: Get status information successfully
     * @return false: Failed to get status information
     */
    bool IEQPRO::getStatus(MountInfo *info)
    {
        char res[DRIVER_LEN] = {0};
        if (SendCMD(":GLS#", res))
        {
            char longitude[8] = {0}, latitude[8] = {0}, status[8] = {0};
            strncpy(longitude, res, 7);
            strncpy(latitude, res + 7, 6);
            strncpy(status, res + 13, 6);
            IEQInfo->longitude     = DecodeString(res, 7, 3600.0);                 //经度
            IEQInfo->latitude      = DecodeString(res + 7, 6, 3600.0) - 90;        //维度
            IEQInfo->gpsStatus     = static_cast<GPSStatus>(status[0] - '0');      //GPS状态
            IEQInfo->trackRate     = static_cast<TrackRate>(status[2] - '0');      //跟踪状态
            this->IEQInfo = info;     //本地存储
            return true;
        }
        return false;
    }

    /*
     * name: getUTCDateTime(double *utc_hours, int *yy, int *mm, int *dd, int *hh, int *minute, int *ss)
     * @param utc_hours: UTC时间
     * @param yy: 年
     * @param mm: 月
     * @param dd: 日
     * @param hh: 小时
     * @param minute: 分钟
     * @param ss: 秒
     * describe: Get UTC time from mount
     * 描述： 从赤道仪读取UTC时间
     * calls: SendCMD()
     * calls: DecodeString()
     * calls: ln_zonedate_to_date()
     * @return true: Get UTC time successfully
     * @return false: Failed to get UTC time
     */
    bool IEQPRO::getUTCDateTime(double *utc_hours, int *yy, int *mm, int *dd, int *hh, int *minute, int *ss)
    {
        char res[DRIVER_LEN] = {0};
        if (SendCMD(":GLT#", res))
        {
            *utc_hours = DecodeString(res, 4, 60.0);
            *yy = DecodeString(res + 5, 2) + 2000;
            *mm = DecodeString(res + 7, 2);
            *dd = DecodeString(res + 9, 2);
            *hh = DecodeString(res + 11, 2);
            *minute = DecodeString(res + 13, 2);
            *ss = DecodeString(res + 15, 2);

            ln_zonedate localTime;
            ln_date utcTime;

            localTime.years = *yy;
            localTime.months = *mm;
            localTime.days = *dd;
            localTime.hours = *hh;
            localTime.minutes = *minute;
            localTime.seconds = *ss;
            localTime.gmtoff = static_cast<long>(*utc_hours * 3600);

            ln_zonedate_to_date(&localTime, &utcTime);

            *yy = utcTime.years;
            *mm = utcTime.months;
            *dd = utcTime.days;
            *hh = utcTime.hours;
            *minute = utcTime.minutes;
            *ss = static_cast<int>(utcTime.seconds);
            return true;
        }
        return false;
    }

    /*
     * name: SendCMD(const char * cmd, char * res, int cmd_len, int res_len)
     * @param cmd: Command to be sent. Can be either NULL TERMINATED or just byte buffer.
     * @param res: ifnot nullptr, the function will wait for a response from the device. ifnullptr, it returns true immediately after the command is successfully sent.
     * @param cmd_len: if-1, it is assumed that the @a cmd is a null-terminated string. Otherwise, it would write @a cmd_len bytes from the @a cmd buffer.
     * @param res_len if-1 and if@a res is not nullptr, the function will read until it detects the default delimeter DRIVER_STOP_CHAR up to DRIVER_LEN length. Otherwise, the function will read @a res_len from the device and store it in @a res.
     * describe: Send a string command to device.
     * 描述：发送指令
     * calls: tcflush()
     * calls: hexDump()
     * calls: Write()
     * calls: WriteString()
     * calls: COM_ERRORMSG()
     * calls: Read()
     * calls: ReadNSection()
     * @return True ifsuccessful, false otherwise.
     */
    bool IEQPRO::SendCMD(const char * cmd, char * res, int cmd_len, int res_len)
    {
        int nbytes_written = 0, nbytes_read = 0, rc = -1;
        tcflush(m_PortFD, TCIOFLUSH);
        if(cmd_len > 0)        //发送普通指令
        {
            char hex_cmd[DRIVER_LEN * 3] = {0};
            hexDump(hex_cmd, cmd, cmd_len);
            IDLog(_("CMD <%s>"), hex_cmd);
            rc = TTY->Write(m_PortFD, cmd, cmd_len, &nbytes_written);
        }
        else        //发送字符串
        {
            IDLog(_("CMD <%s>"), cmd);
            rc = TTY->WriteString(m_PortFD, cmd, &nbytes_written);
        }
        if(rc != TTY_OK)       //发送指令失败
        {
            char errstr[MAXRBUF] = {0};
            TTY->COM_ERRORMSG(rc, errstr, MAXRBUF);
            IDLog(_("Serial write error: %s."),errstr);
            return false;
        }
        if(res == nullptr)     //没有发挥信息
            return true;
        if(res_len > 0)        //读取成功
            rc = TTY->Read(m_PortFD, res, res_len, DRIVER_TIMEOUT, &nbytes_read);
        else
            rc = TTY->ReadNSection(m_PortFD, res, DRIVER_LEN, DRIVER_STOP_CHAR, DRIVER_TIMEOUT, &nbytes_read);
        if(rc != TTY_OK)       //串口读取失败
        {
            char errstr[MAXRBUF] = {0};
            TTY->COM_ERRORMSG(rc, errstr, MAXRBUF);
            IDLog(_("Serial read error: %s."), errstr);
            return false;
        }
        if(res_len > 0)
        {
            char hex_res[64 * 3] = {0};
            hexDump(hex_res, res, res_len);
        }
        tcflush(m_PortFD, TCIOFLUSH);
        return true;
    }

    /*
     * name: hexDump(char * buf, const char * data, int size)
     * @param buf: buffer to format the command into hex strings.
     * @param data: the command
     * @param size: length of the command in bytes.
     * describe: Helper function to print non-string commands to the logger so it is easier to debug
     * 描述：将不是字符串的指令转化为字符串
     * note: This is called internally by sendCommand, no need to call it directly.
     */
    void IEQPRO::hexDump(char * buf, const char * data, int size)
    {
        for (int i = 0; i < size; i++)
            sprintf(buf + 3 * i, "%02X ", static_cast<uint8_t>(data[i]));
        if(size > 0)
            buf[3 * size - 1] = '\0';
    }

    /*
     * name: isCMDSupported(const std::string &command, bool silent)
     * @param command: command code
     * @param slient:if false (default), it will report why command is not supported. If true, it will not print any messages.
     * describe: Check commands which are supported
     * 描述：判断命令是否支持
     * calls: IDLog_Error()
     * @return: True when it is supported and false unsupported
     */
    bool IEQPRO::isCMDSupported(const std::string &command, bool silent)
    {
        if(command == "MSH")        //寻找归位位置
        {
            if(IEQInfo->Name.find("CEM60") == std::string::npos &&
                    IEQInfo->Name.find("CEM40") == std::string::npos &&
                    IEQInfo->Name.find("GEM45") == std::string::npos)
            {
                if(!silent)
                    IDLog_Error(_("Finding home is only supported on CEM40, GEM45 and CEM60 mounts."));
                return false;
            }
        }
        else if(command == "RR")     //跟踪频率
        {
            if(IEQInfo->Name.find("AA") != std::string::npos)
            {
                if(!silent)
                    IDLog_Error(_("Tracking rate is not supported on Altitude-Azimuth mounts."));
                return false;
            }
        }
        else if(command == "RG" || command == "AG")     //导星
        {
            if(IEQInfo->Name.find("AA") != std::string::npos)
            {
                if(!silent)
                    IDLog_Error(_("Guide rate is not supported on Altitude-Azimuth mounts."));
                return false;
            }
        }
        if(command == "MP0" || command == "MP1" || command == "SPA" || command == "SPH")    //归为
        {
            if(IEQInfo->Name.find("CEM60") == std::string::npos &&
                    IEQInfo->Name.find("CEM40") == std::string::npos &&
                    IEQInfo->Name.find("GEM45") == std::string::npos &&
                    IEQInfo->Name.find("iEQ") == std::string::npos)
            {
                if(!silent)
                    IDLog_Error(_("Parking only supported on CEM40, GEM45, CEM60, iEQPro 30 and iEQ Pro 45."));
                return false;
            }
        }
        return true;
    }
}