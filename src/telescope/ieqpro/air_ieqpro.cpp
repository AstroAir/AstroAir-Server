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
 
Date:2021-6-26
 
Description:IEQ Mount Offical Port

**************************************************/

#include "air_ieqpro.h"
#include "../../logger.h"
#include "../../libastro.h"

#include "../air_nova.h"

#include <string.h>
#include <libnova/julian_day.h>

#define MAXRBUF 2048

namespace AstroAir
{
    IEQPRO::IEQPRO()
    {

    }

    IEQPRO::~IEQPRO()
    {

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
            if(TTY->Open("/dev/ttyUSB0",115200,8, 2, 2,reinterpret_cast<int*>(m_PortFD)) != true)       //这里需要增加多端口支持
                return false;
        }
        else
        {
            if(TTY->Open("/dev/ttyUSB0",9600,8, 2, 2,reinterpret_cast<int*>(m_PortFD)) != true)       //这里需要增加多端口支持
                return false;
        }
        if(GetModel())      //获取主板信息
        {
            if(GetRADECFirmware() && GetMainFirmware())     //获取其他板子的信息
            {
                if(m_FirmwareInfo.Model == Device_name)     //判断赤道仪是否为需要连接的赤道仪
                {
                    IDLog(_("Found %s\n"),m_FirmwareInfo.Model.c_str());
                    for (const auto &oneMount : m_MountList)
                    {
                        if(oneMount.model == m_FirmwareInfo.Model)     //判断赤道仪名称
                        {
                            if(m_FirmwareInfo.MainBoardFirmware >= oneMount.firmware)      //判断版本是否低于最低版本
                            {
                                UpdateMountConfigure();
                                return true;
                            }    
                            else
                                IDLog(_("Main board firmware is %s while minimum required firmware is %s. Please upgrade the mount firmware."),m_FirmwareInfo.MainBoardFirmware.c_str(), oneMount.firmware.c_str());
                        }
                    }
                }
                else
                {
                    IDLog(_("Could not found %s,please make sure you've already connected mount\n"),m_FirmwareInfo.Model.c_str());
                    return false;
                }
            }
        }
        else
            IDLog(_("Could not get mount's name\n"));
        return false;
    }

    bool IEQPRO::Disconnect()
    {
        return true;
    }

    std::string IEQPRO::ReturnDeviceName()
    {
        return m_FirmwareInfo.Model;
    }

    bool IEQPRO::UpdateMountConfigure()
    {
        /*检查赤道仪是否支持一下功能*/
        canParkNatively = isCMDSupported("MP1", true);
        canFindHome = isCMDSupported("MSH", true);
        canGuideRate = isCMDSupported("RG", true);

        firmwareInfo = getFirmwareInfo();

        double utc_offset;
        int yy, dd, mm, hh, minute, ss;
        if (getUTCDateTime(&utc_offset, &yy, &mm, &dd, &hh, &minute, &ss))
        {
            char isoDateTime[32] = {0};
            char utcOffset[8] = {0};
            snprintf(isoDateTime, 32, "%04d-%02d-%02dT%02d:%02d:%02d", yy, mm, dd, hh, minute, ss);
            snprintf(utcOffset, 8, "%4.2f", utc_offset);
            IDLog(_("Mount UTC offset is %s. UTC time is %s"), utcOffset, isoDateTime);
        }
        return true;
    }

    /*
     * name: GetModel()
     * describe: Get mount model
     * 描述：获取赤道仪名称
     * calls: SendCMD()
     */
    bool IEQPRO::GetModel()
    {
        char res[DRIVER_LEN] = {0};
        if(SendCMD(":MountInfo#", res, -1, 4))     //获取赤道仪名称
        {
            std::string code = res;
            /*比较获取赤道仪名称是否为已知的*/
            auto result = std::find_if(m_MountList.begin(), m_MountList.end(), [code](const MountInfo & oneMount)
            {
                return oneMount.code == code;
            });
            if(result != m_MountList.end())        //如果结果是已知的
            {
                m_FirmwareInfo.Model = result->model;
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
            m_FirmwareInfo.MainBoardFirmware.assign(board, 6);
            m_FirmwareInfo.ControllerFirmware.assign(controller, 6);
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
            m_FirmwareInfo.RAFirmware.assign(ra, 6);
            m_FirmwareInfo.DEFirmware.assign(de, 6);
            return true;
        }
        return false;
    }

    /*
     * name: getGuideRate(double *raRate, double *deRate)
     * @param raRate: RA轴导星速率
     * @param decRate: DEC轴导星速率
     * describe: Get mount's guide setting
     * 描述： 获取赤道仪导星设置
     * calls: isCMDSupported()
     * calls: DecodeString()
     * calls: SendCMD()
     * @return true: Get ra and dec setting successfully
     * @return false: Failed to get ra and dec setting
     */
    bool IEQPRO::getGuideRate(double *raRate, double *deRate)
    {
        if (!isCMDSupported("AG"))      //判断是否支持导星
            return false;
        char res[DRIVER_LEN] = {0};
        if (SendCMD(":AG#", res))
        {
            *raRate = DecodeString(res, 2, 100.0);
            *deRate = DecodeString(res + 2, 2, 100.0);
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
    bool IEQPRO::getStatus(Info *info)
    {
        char res[DRIVER_LEN] = {0};
        if (SendCMD(":GLS#", res))
        {
            char longitude[8] = {0}, latitude[8] = {0}, status[8] = {0};

            strncpy(longitude, res, 7);
            strncpy(latitude, res + 7, 6);
            strncpy(status, res + 13, 6);

            info->longitude     = DecodeString(res, 7, 3600.0);                 //经度
            info->latitude      = DecodeString(res + 7, 6, 3600.0) - 90;        //维度
            info->gpsStatus     = static_cast<GPSStatus>(status[0] - '0');      //GPS状态
            info->systemStatus  = static_cast<SystemStatus>(status[1] - '0');   //功能状态
            info->trackRate     = static_cast<TrackRate>(status[2] - '0');      //跟踪状态
            info->slewRate      = static_cast<SlewRate>(status[3] - '0' - 1);   //移动速度
            info->timeSource    = static_cast<TimeSource>(status[4] - '0');     //时间
            info->hemisphere    = static_cast<Hemisphere>(status[5] - '0');     //南/北半球
            this->info = *info;     //本地存储
            return true;
        }
        return false;
    }

    const char * IEQPRO::pierSideStr(IEQ_PIER_SIDE ps)
    {
        switch(ps)
        {
            case IEQ_PIER_EAST:
                return _("EAST");
            case IEQ_PIER_WEST:
                return _("WEST");
            case IEQ_PIER_UNKNOWN:
                return _("UNKNOWN");
            case IEQ_PIER_UNCERTAIN:
                return _("UNCERTAIN");
        }
        return _("Impossible");
    }

    bool IEQPRO::getPierSide(IEQ_PIER_SIDE *pierSide)
    {
        char res[DRIVER_LEN] = {0};
        //使用GEA命令，希望它能返回dec和极轴的位置，这一部分是搬运自https://www.indilib.org/forum/mounts/6720-ioptron-cem60-question.html#52154
        if (SendCMD(":GEA#", res))     
        {
            haAxis = DecodeString(res + 9, 8, ieqHours);        //获取一段时间内的旋转角度
            decAxis = DecodeString(res, 9, ieqDegrees);         //获取北极星旋转的解读
            double axisHa = 0;
            if (decAxis >= 0)
            {
                *pierSide = IEQ_PIER_WEST;
                axisHa = 18 - haAxis;
            }
            else
            {
                *pierSide = IEQ_PIER_EAST;
                axisHa = haAxis - 6;
            }
            //当dec为90时，极轴角度不完全为0，这会导致靠极轴校准不正确的问题。
            //尝试通过使用HA可依赖的时角来处理此问题-远离经络
            //在子午线2小时内使用极角。
            //如果杆角小于极轴角度和dec之间的差值，则报告未知桥墩侧
            //我知道，很可怕，但是mount报告的数据很难解释，所以这似乎是最少的
            //最坏的解决方案，不管怎样，让我们看看它是否有效
            //以上翻译自INDI对于艾顿赤道仪bug的解决方案
            double lst = get_local_sidereal_time(info.longitude);
            double ha = rangeHA(get_local_hour_angle(lst, Ra));

            const char *reason;
            double decPA = info.latitude >= 0 ? 90 - Dec : 90 + Dec;    //用磁偏角确定的到极点的距离，对两个半球都合适
            if ((ha > 2 && ha < 10) || (ha < -2 && ha > -10))
            {
                *pierSide = ha > 0 ? IEQ_PIER_EAST : IEQ_PIER_WEST;     //使用HA来确定
                reason = _("Hour Angle");
            }
            else
            {
                double decDiff = std::fabs(decPA - std::fabs(decAxis));         //对南半球的情况不太清楚
                if (decPA > decDiff)
                {
                    
                    *pierSide = decAxis > 0 ? IEQ_PIER_WEST : IEQ_PIER_EAST;        //使用极轴旋转角度
                    reason = _("pole angle");
                }
                else
                {
                    *pierSide = IEQ_PIER_UNCERTAIN;         //未知的校准方式
                    reason = _("uncertain");
                }
            }
            IDLog(_("getPierSide pole Axis %f, haAxis %f, axisHa %f, ha %f, decPa %f, %s pierSide %s"), decAxis, haAxis, axisHa, ha, decPA, reason,
            pierSideStr(*pierSide));
            return true;
        }
        *pierSide = IEQ_PIER_UNKNOWN;
        return false;
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
            if(m_FirmwareInfo.Model.find("CEM60") == std::string::npos &&
                    m_FirmwareInfo.Model.find("CEM40") == std::string::npos &&
                    m_FirmwareInfo.Model.find("GEM45") == std::string::npos)
            {
                if(!silent)
                    IDLog_Error(_("Finding home is only supported on CEM40, GEM45 and CEM60 mounts."));
                return false;
            }
        }
        else if(command == "RR")     //跟踪频率
        {
            if(m_FirmwareInfo.Model.find("AA") != std::string::npos)
            {
                if(!silent)
                    IDLog_Error(_("Tracking rate is not supported on Altitude-Azimuth mounts."));
                return false;
            }
        }
        else if(command == "RG" || command == "AG")     //导星
        {
            if(m_FirmwareInfo.Model.find("AA") != std::string::npos)
            {
                if(!silent)
                    IDLog_Error(_("Guide rate is not supported on Altitude-Azimuth mounts."));
                return false;
            }
        }
        if(command == "MP0" || command == "MP1" || command == "SPA" || command == "SPH")    //归为
        {
            if(m_FirmwareInfo.Model.find("CEM60") == std::string::npos &&
                    m_FirmwareInfo.Model.find("CEM40") == std::string::npos &&
                    m_FirmwareInfo.Model.find("GEM45") == std::string::npos &&
                    m_FirmwareInfo.Model.find("iEQ") == std::string::npos)
            {
                if(!silent)
                    IDLog_Error(_("Parking only supported on CEM40, GEM45, CEM60, iEQPro 30 and iEQ Pro 45."));
                return false;
            }
        }
        return true;
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
            char hex_res[DRIVER_LEN * 3] = {0};
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
}