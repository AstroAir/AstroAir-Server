/*
 * air_ieqpro.h
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
 
Date:2021-7-18
 
Description:IEQ Mount Offical Port

**************************************************/

#ifndef _AIR_IEQPRO_H_
#define _AIR_IEQPRO_H_

#include "../../air_mount.h"
#include "../air_com.h"
#include <vector>

namespace AstroAir
{
    class IEQPRO:public AIRMOUNT
    {
        public:
            explicit IEQPRO(MountInfo *NEW);
            ~IEQPRO();
            virtual bool Connect(std::string Device_name)override;
            virtual bool Disconnect()override;
            std::string ReturnDeviceName()override;

            virtual bool Goto(std::string Target_RA,std::string Target_DEC)override;
            virtual bool IsMountPark(bool status)override;
            virtual bool IsMountTrack(bool status)override;
            virtual bool IsMountAbort(int status)override;
            virtual bool ReadScopeStatus();

        protected:
            /*更新赤道仪设置*/
            virtual bool UpdateMountConfigure();
            /*获取赤道仪名称*/
            virtual bool GetModel();
            /*获取赤道仪主板固件版本*/
            virtual bool GetMainFirmware();
            /*获取赤道仪赤经赤纬电机固件版本*/
            virtual bool GetRADECFirmware();
            virtual bool getStatus(MountInfo *info);
            virtual bool getUTCDateTime(double *utc_hours, int *yy, int *mm, int *dd, int *hh, int *minute, int *ss);
            /*发送指令*/
            virtual bool SendCMD(const char * cmd, char * res, int cmd_len = -1, int res_len = -1);
            virtual void hexDump(char * buf, const char * data, int size);
            /*检查指令是否支持*/
            virtual bool isCMDSupported(const std::string &command, bool silent = false);

            virtual bool setRA(double ra);
            virtual bool setDE(double dec);
            virtual bool slew();

        private:
            MountInfo *IEQInfo;
            AIRCOM *TTY;
            int m_PortFD;

            typedef struct 
            {
                std::string Model;
                std::string MainBoardFirmware;
                std::string ControllerFirmware;
                std::string RAFirmware;
                std::string DEFirmware;
            } MountFriware; MountFriware FirmwareInfo;
            

            typedef struct      //赤道仪基础信息
            {
                std::string code;
                std::string model;
                std::string firmware;
            } iMountInfo;

            const std::vector<iMountInfo> MountList =      //赤道仪列表
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
    };
}

#endif