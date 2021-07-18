/*
 * air_mount.h
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
 
Date:2021-6-23
 
Description:Mount Offical Port

**************************************************/

#ifndef _AIR_MOUNT_H_
#define _AIR_MOUNT_H_

#include <string>
#include <atomic>

namespace AstroAir
{
    class AIRMOUNT
    {
        public:
            /*连接设备*/
            virtual bool Connect(std::string Device_name);
            /*与设备断开连接*/
			virtual bool Disconnect();
            /*返回设备名称*/
			virtual std::string ReturnDeviceName();
            /*Goto（服务器）*/
            virtual bool GotoServer(std::string Target_RA,std::string Target_DEC);
            /*Goto（本地）*/
            virtual bool Goto(std::string Target_RA,std::string Target_DEC);
            /*Park（服务器）*/
            virtual bool ParkServer(bool status);
            /*Park（本地）*/
            virtual bool IsMountPark(bool status);
            virtual bool TrackServer(bool status);
            virtual bool IsMountTrack(bool status);
            virtual bool AbortServer(int status);
            virtual bool IsMountAbort(int status);
            virtual bool IsMountHomeServer(bool status);
            virtual bool IsMountHome(bool status);
        private:

    };
    extern AIRMOUNT *MOUNT;

    typedef enum { GPS_OFF, GPS_ON, GPS_DATA_OK } GPSStatus;        //GPS状态
    typedef enum { TR_SIDEREAL, TR_LUNAR, TR_SOLAR, TR_KING, TR_CUSTOM } TrackRate;
    enum TelescopeStatus
    {
        MOUNT_IDLE,
        MOUNT_SLEWING,
        MOUNT_TRACKING,
        MOUNT_PARKING,
        MOUNT_PARKED,
        MOUNT_ERROR
    };

    struct MountInfo
    {
        bool isMountConnected;

        TelescopeStatus Status;
        GPSStatus gpsStatus;
        TrackRate trackRate;

        std::string Name;
        std::string MountRa;
        std::string MountDEC;
        std::string MountTime;

        double latitude;
        double longitude;
        std::string UTCtime;

        bool canParkNatively = false;       //是否支持归位
        bool canFindHome = false;           //是否支持寻找归位位置
        bool canGuideRate = false;          //是否支持导星
        bool slewDirty = false;             //转向
    };extern MountInfo *AIRMOUNTINFO;

    void RAConvert(std::string Target_RA,int *h,int *m,int *s);
    void DECConvert(std::string Target_DEC,int *h,int *m,int *s);
}

#endif