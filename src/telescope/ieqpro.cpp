/*
 * ieqpro.cpp
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
 
/************************************************* 

Copyright: 2020-2021 Max Qian. All rights reserved

Author:Max Qian

E-mail:astro_air@126.com

Date:2020-2-27

Description:IEQPro equipment driver

**************************************************/

#include "ieqpro.h"
#include "../logger.h"
#include "../aircom.h"

namespace AstroAir
{
    IEQPro::IEQPro()
    {

    }

    IEQPro::~IEQPro()
    {

    }

    bool IEQPro::Connect(std::string Device_name)
    {
        m_PortFD = 0;
        if ((rc = getModel()))
        {
            if ((rc = getMainFirmware()) && (rc = getRADEFirmware()))
            {
                for (const auto &oneMount : m_MountList)
                {
                    if (oneMount.model == m_FirmwareInfo.Model)
                    {
                        if (m_FirmwareInfo.MainBoardFirmware >= oneMount.firmware)
                            return true;
                        else
                            IDLog("Main board firmware is %s while minimum required firmware is %s. Please upgrade the mount firmware.",m_FirmwareInfo.MainBoardFirmware.c_str(), oneMount.firmware.c_str());
                    }
                }
                return false;
            }
        }
        return rc;
    }

    bool IEQPro::Disconnect()
    {
        return true;
    }

    bool IEQPro::Goto(std::string Target_RA,std::string Target_DEC)
    {
        StringList RA = splitstr(Target_RA, ':');
        StringList DEC = splitstr(Target_DEC, ':');
        int ra_h, ra_m, ra_s;
	    int dec_d, dec_m, dec_s;
        char Target__RA[DRIVER_LEN],Target__DEC[DRIVER_LEN];
        char res[DRIVER_LEN] = {0};
        const char dec_sign = DEC[0].at(0);
        DEC[0].substr(1,DEC[0].length());
        ra_h = atoi(RA[0].c_str());        dec_d = atoi(DEC[0].c_str());
        ra_m = atoi(RA[1].c_str());        dec_m = atoi(DEC[1].c_str());
        ra_s = atoi(RA[2].c_str());        dec_s = atoi(DEC[2].c_str());
        sprintf(Target__RA,":Sr%08d#",(ra_h*3600+ra_m*60+ra_s)*1000);
        if (dec_sign == '+')
            sprintf(Target__DEC,":Sd+%08d#",(dec_d*3600+dec_m*60+dec_s)*100);
        else
            sprintf(Target__DEC,":Sd-%08d#",(dec_d*3600+dec_m*60+dec_s)*100);
        if(sendCommand(Target__RA, res, -1, 1) == true && sendCommand(Target__DEC, res, -1, 1) == true)
        {
            IDLog("The equator began to move\n");
            WebLog("The equator began to move",2);
            if (sendCommand(":MS#", res, -1, 1))
            {
                return res[0] == '1';
            }
            return false;
        }
        else
        {
            IDLog("Cannot set right ascension and declination\n");
            WebLog("Cannot set right ascension and declination",3);
        }
        return false;
    }

    bool IEQPro::getModel()
    {
        char res[DRIVER_LEN] = {0};
        if (sendCommand(":MountInfo#", res, -1, 4))
        {
            std::string code = res;
            auto result = std::find_if(m_MountList.begin(), m_MountList.end(), [code](const MountInfo & oneMount)
            {
                return oneMount.code == code;
            });
            if (result != m_MountList.end())
            {
                m_FirmwareInfo.Model = result->model;
                IDLog("Find %s\n",m_FirmwareInfo.Model.c_str());
                getMainFirmware();
                getRADEFirmware();
                return true;
            }
            IDLog("Mount with code %s is not recognized.", res);
            return false;
        }
        return false;
    }

    bool IEQPro::getMainFirmware()
    {
        char res[DRIVER_LEN] = {0};
        if (sendCommand(":FW1#", res))
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

    bool IEQPro::getRADEFirmware()
    {
        char res[DRIVER_LEN] = {0};
        if (sendCommand(":FW2#", res))
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

    bool IEQPro::findHome()
    {
        if (!isCommandSupported("MSH"))
            return false;
        char res[DRIVER_LEN] = {0};
        return sendCommand(":MSH#", res, -1, 1);
    }

    bool IEQPro::sendCommand(const char * cmd, char * res, int cmd_len, int res_len)
    {
        int nbytes_written = 0, nbytes_read = 0, rc = -1;
        tcflush(m_PortFD, TCIOFLUSH);
        if (cmd_len > 0)
        {
            char hex_cmd[DRIVER_LEN * 3] = {0};
            hexDump(hex_cmd, cmd, cmd_len);
            IDLog("CMD <%s>", hex_cmd);
            rc = tty_write(m_PortFD, cmd, cmd_len, &nbytes_written);
        }
        else
        {
            IDLog("CMD <%s>", cmd);
            rc = tty_write_string(m_PortFD, cmd, &nbytes_written);
        }
        if (rc != TTY_OK)
        {
            char errstr[MAXRBUF] = {0};
            tty_error_msg(rc, errstr, MAXRBUF);
            IDLog("Serial write error: %s.", errstr);
            return false;
        }
        if (res == nullptr)
            return true;
        if (res_len > 0)
            rc = tty_read(m_PortFD, res, res_len, DRIVER_TIMEOUT, &nbytes_read);
        else
            rc = tty_nread_section(m_PortFD, res, DRIVER_LEN, DRIVER_STOP_CHAR, DRIVER_TIMEOUT, &nbytes_read);
        if (rc != TTY_OK)
        {
            char errstr[MAXRBUF] = {0};
            tty_error_msg(rc, errstr, MAXRBUF);
            IDLog("Serial read error: %s.", errstr);
            return false;
        }
        if (res_len > 0)
        {
            char hex_res[DRIVER_LEN * 3] = {0};
            hexDump(hex_res, res, res_len);
            IDLog("RES <%s>", hex_res);
        }
        else
        {
            IDLog("RES <%s>", res);
        }
        tcflush(m_PortFD, TCIOFLUSH);
        return true;
    }

    bool IEQPro::isCommandSupported(const std::string &command, bool silent)
    {
        // Find Home
        if (command == "MSH")
        {
            if (m_FirmwareInfo.Model.find("CEM60") == std::string::npos &&
                    m_FirmwareInfo.Model.find("CEM40") == std::string::npos &&
                    m_FirmwareInfo.Model.find("GEM45") == std::string::npos)
            {
                if (!silent)
                    IDLog("Finding home is only supported on CEM40, GEM45 and CEM60 mounts.");
                return false;

            }
        }
        else if (command == "RR")
        {
            if (m_FirmwareInfo.Model.find("AA") != std::string::npos)
            {
                if (!silent)
                    IDLog("Tracking rate is not supported on Altitude-Azimuth mounts.");
                return false;
            }
        }
        else if (command == "RG" || command == "AG")
        {
            if (m_FirmwareInfo.Model.find("AA") != std::string::npos)
            {
                if (!silent)
                    IDLog("Guide rate is not supported on Altitude-Azimuth mounts.");
                return false;
            }
        }
        if (command == "MP0" || command == "MP1" || command == "SPA" || command == "SPH")
        {
            if (m_FirmwareInfo.Model.find("CEM60") == std::string::npos &&
                    m_FirmwareInfo.Model.find("CEM40") == std::string::npos &&
                    m_FirmwareInfo.Model.find("GEM45") == std::string::npos &&
                    m_FirmwareInfo.Model.find("iEQ") == std::string::npos)
            {
                if (!silent)
                    IDLog("Parking only supported on CEM40, GEM45, CEM60, iEQPro 30 and iEQ Pro 45.");
                return false;
            }
        }

        return true;
    }

    void IEQPro::hexDump(char * buf, const char * data, int size)
    {
        for (int i = 0; i < size; i++)
            sprintf(buf + 3 * i, "%02X ", static_cast<uint8_t>(data[i]));
        if (size > 0)
            buf[3 * size - 1] = '\0';
    }
}