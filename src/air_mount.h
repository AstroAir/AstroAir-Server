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
    /*这里的大部分函数都是没有任何实际用途的
      只能作为一个模板参考
      不过基本的控制逻辑就是这个样子
      所有的_Server函数都是有用的
      在派生类中不用重新写*/
    class AIRMOUNT
    {
        public:
            virtual bool Connect(std::string Device_name);
			virtual bool Disconnect();
			virtual std::string ReturnDeviceName();
            virtual bool GotoServer(std::string Target_RA,std::string Target_DEC);
            virtual bool Goto(std::string Target_RA,std::string Target_DEC);
            virtual bool ParkServer(bool status);
            virtual bool Park();
            virtual bool Unpark();
            virtual bool TrackServer(bool status);
            virtual bool Track(bool status);
            virtual bool AbortServer(int status);
            virtual bool Abort(int status);
            virtual double DecodeString(const char * data, size_t size, double factor);
            virtual int DecodeString(const char *data, size_t size);
        private:

    };
    extern AIRMOUNT *MOUNT;
    extern std::atomic_bool isMountConnected;
    extern std::atomic_bool isMountSlewing;
    extern std::atomic_bool isMountTracking;
    extern std::atomic_bool isMountParked;
    
    void RAConvert(std::string Target_RA,int *h,int *m,int *s);
    void DECConvert(std::string Target_DEC,int *h,int *m,int *s);

    enum
        {
            TELESCOPE_CAN_GOTO                    = 1 << 0,  /** Can the telescope go to to specific coordinates? */
            TELESCOPE_CAN_SYNC                    = 1 << 1,  /** Can the telescope sync to specific coordinates? */
            TELESCOPE_CAN_PARK                    = 1 << 2,  /** Can the telescope park? */
            TELESCOPE_CAN_ABORT                   = 1 << 3,  /** Can the telescope abort motion? */
            TELESCOPE_HAS_TIME                    = 1 << 4,  /** Does the telescope have configurable date and time settings? */
            TELESCOPE_HAS_LOCATION                = 1 << 5,  /** Does the telescope have configuration location settings? */
            TELESCOPE_HAS_PIER_SIDE               = 1 << 6,  /** Does the telescope have pier side property? */
            TELESCOPE_HAS_PEC                     = 1 << 7,  /** Does the telescope have PEC playback? */
            TELESCOPE_HAS_TRACK_MODE              = 1 << 8,  /** Does the telescope have track modes (sidereal, lunar, solar..etc)? */
            TELESCOPE_CAN_CONTROL_TRACK           = 1 << 9,  /** Can the telescope engage and disengage tracking? */
            TELESCOPE_HAS_TRACK_RATE              = 1 << 10, /** Does the telescope have custom track rates? */
            TELESCOPE_HAS_PIER_SIDE_SIMULATION    = 1 << 11, /** Does the telescope simulate the pier side property? */
            TELESCOPE_CAN_TRACK_SATELLITE         = 1 << 12, /** Can the telescope track satellites? */
        } TelescopeCapability;

    enum TelescopeStatus
    {
        SCOPE_IDLE,
        SCOPE_SLEWING,
        SCOPE_TRACKING,
        SCOPE_PARKING,
        SCOPE_PARKED
    };
    enum TelescopeMotionCommand
    {
        MOTION_START = 0,
        MOTION_STOP
    };
    enum TelescopeSlewRate
    {
        SLEW_GUIDE,
        SLEW_CENTERING,
        SLEW_FIND,
        SLEW_MAX
    };
    enum TelescopeTrackMode
    {
        TRACK_SIDEREAL,
        TRACK_SOLAR,
        TRACK_LUNAR,
        TRACK_CUSTOM
    };
    enum TelescopeTrackState
    {
        TRACK_ON,
        TRACK_OFF,
        TRACK_UNKNOWN
    };
    enum TelescopeParkData
    {
        PARK_NONE,
        PARK_RA_DEC,
        PARK_HA_DEC,
        PARK_AZ_ALT,
        PARK_RA_DEC_ENCODER,
        PARK_AZ_ALT_ENCODER
    };
    enum TelescopeLocation
    {
        LOCATION_LATITUDE,
        LOCATION_LONGITUDE,
        LOCATION_ELEVATION
    };
    enum TelescopePierSide
    {
        PIER_UNKNOWN = -1,
        PIER_WEST = 0,
        PIER_EAST = 1
    };

    enum TelescopePECState
    {
        PEC_UNKNOWN = -1,
        PEC_OFF = 0,
        PEC_ON = 1
    };
}

#endif