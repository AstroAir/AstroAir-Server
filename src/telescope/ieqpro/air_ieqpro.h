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
 
Date:2021-6-27
 
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
        public:     //以下函数为统一接口
            explicit IEQPRO();
            ~IEQPRO();
            virtual bool Connect(std::string Device_name)override;
            virtual bool Disconnect()override;
            std::string ReturnDeviceName()override;

        private:
            /*获取赤道仪名称*/
            bool GetModel();
            /*获取赤道仪主板固件版本*/
            bool GetMainFirmware();
            /*获取赤道仪赤经赤纬电机固件版本*/
            bool GetRADECFirmware();
            /*更新赤道仪的设置*/
            bool UpdateMountConfigure();

            AIRCOM *TTY;

            int m_PortFD;

            static const uint8_t DRIVER_TIMEOUT { 3 };      //赤道仪超时时间
            static const uint8_t DRIVER_LEN { 64 };         //信息长度
            static const char DRIVER_STOP_CHAR { '#' };     //默认指令前置

            bool canParkNatively = false;       //是否支持归位
            bool canFindHome = false;           //是否支持寻找归位位置
            bool canGuideRate = false;          //是否支持导星
            bool slewDirty = false;             //转向

            typedef struct      //赤道仪基础信息
            {
                std::string code;
                std::string model;
                std::string firmware;
            } MountInfo;

            typedef enum        //赤道仪功能状态
            {
                ST_STOPPED,
                ST_TRACKING_PEC_OFF,
                ST_SLEWING,
                ST_GUIDING,
                ST_MERIDIAN_FLIPPING,
                ST_TRACKING_PEC_ON,
                ST_PARKED,
                ST_HOME
            } SystemStatus;

            typedef enum { GPS_OFF, GPS_ON, GPS_DATA_OK } GPSStatus;        //GPS状态
            typedef enum { TR_SIDEREAL, TR_LUNAR, TR_SOLAR, TR_KING, TR_CUSTOM } TrackRate;
            typedef enum { SR_1, SR_2, SR_3, SR_4, SR_5, SR_6, SR_7, SR_8, SR_MAX } SlewRate;
            typedef enum { TS_RS232, TS_CONTROLLER, TS_GPS } TimeSource;
            typedef enum { HEMI_SOUTH, HEMI_NORTH } Hemisphere;
            typedef enum { FW_MODEL, FW_BOARD, FW_CONTROLLER, FW_RA, FW_DEC } FirmwareItem;
            typedef enum { RA_AXIS, DEC_AXIS } Axis;
            typedef enum { IEQ_N, IEQ_S, IEQ_W, IEQ_E } Direction;
            typedef enum { IEQ_SET_HOME, IEQ_GOTO_HOME, IEQ_FIND_HOME } HomeOperation;
            typedef enum { IEQ_PIER_UNKNOWN = -1, IEQ_PIER_WEST = 0, IEQ_PIER_EAST = 1, IEQ_PIER_UNCERTAIN = 2 } IEQ_PIER_SIDE;

            typedef struct
            {
                GPSStatus gpsStatus;
                SystemStatus systemStatus;
                SystemStatus rememberSystemStatus;
                TrackRate trackRate;
                SlewRate slewRate;
                TimeSource timeSource;
                Hemisphere hemisphere;
                double longitude;
                double latitude;
            } Info;
            
            /*赤道仪信息*/
            double haAxis;
            double decAxis;
            double Ra;
            double Dec;
            Info info;

            const std::vector<MountInfo> m_MountList =      //赤道仪列表
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
            FirmwareInfo firmwareInfo;

        protected:      //以下函数为功能性函数，是这种赤道仪所特有的，在统一接口中不会包括
            virtual bool SendCMD(const char * cmd, char * res, int cmd_len = -1, int res_len = -1);
            void hexDump(char * buf, const char * data, int size);
            virtual bool isCMDSupported(const std::string &command, bool silent = false);

            virtual bool getGuideRate(double *raRate, double *deRate);
            virtual bool getUTCDateTime(double *utc_hours, int *yy, int *mm, int *dd, int *hh, int *minute, int *ss);
            virtual bool getStatus(Info *info);

            const FirmwareInfo &getFirmwareInfo() const
            {
                return m_FirmwareInfo;
            }

            const char * pierSideStr(IEQ_PIER_SIDE ps);
            virtual bool getPierSide(IEQ_PIER_SIDE *pierSide);

            constexpr static const double ieqDegrees { 60.0 * 60.0 * 100.0 };
            constexpr static const double ieqHours { 60.0 * 60.0 * 1000.0 };
    };
}

#endif