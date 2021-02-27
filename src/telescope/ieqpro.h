/*
 * ieqpro.h
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

#pragma once

#ifndef _IEQPRO_H_
#define _IEQPRO_H_

#include "../wsserver.h"

namespace AstroAir
{
    class IEQPro:public WSSERVER
    {
        public:
            explicit IEQPro();
            ~IEQPro();
            virtual bool Connect(std::string Device_name) override;
            virtual bool Disconnect() override;
            virtual bool Goto(std::string Target_RA,std::string Target_DEC) override;
        protected:
            void hexDump(char * buf, const char * data, int size);
            bool sendCommand(const char * cmd, char * res = nullptr, int cmd_len = -1, int res_len = -1);
            bool getModel();
            bool getMainFirmware();
            bool getRADEFirmware();
            bool findHome();
            bool isCommandSupported(const std::string &command, bool silent = false);
        private:
            typedef struct
            {
                std::string code;
                std::string model;
                std::string firmware;
            } MountInfo;

            const std::vector<MountInfo> m_MountList =
            {
                {"0010", "Cube II EQ", "160610"},
                {"0011", "Smart EQ Pro+", "161028"},
                {"0025", "CEM25", "170106"},
                {"0026", "CEM25-EC", "170518"},
                {"0030", "iEQ30 Pro", "161101"},
                {"0040", "CEM40", "181018"},
                {"0041", "CEM40-EC", "181018"},
                {"0043", "GEM45", "191018"},
                {"0044", "GEM45-EC", "191018"},
                {"0045", "iEQ45 Pro EQ", "161101"},
                {"0046", "iEQ45 Pro AA", "161101"},
                {"0060", "CEM60", "161101"},
                {"0061", "CEM60-EC", "161101"},
                {"5010", "Cube II AA", "160610"},
                {"5035", "AZ Mount Pro", "170410"},
            };

            typedef struct
            {
                std::string Model;
                std::string MainBoardFirmware;
                std::string ControllerFirmware;
                std::string RAFirmware;
                std::string DEFirmware;
            } FirmwareInfo;

            FirmwareInfo m_FirmwareInfo;

            int m_PortFD {0};
            bool rc = false;
            std::string m_DeviceName { "iEQ" };
            bool m_IsDebug { false };
            static const uint8_t DRIVER_TIMEOUT { 3 };
            static const uint8_t DRIVER_LEN { 64 };
            static const char DRIVER_STOP_CHAR { '#' };
    };
}

#endif