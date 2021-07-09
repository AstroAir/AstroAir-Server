/*
 * air_mount.cpp
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
 
Date:2021-6-28
 
Description:Mount Offical Port

**************************************************/

#include "air_mount.h"
#include "wsserver.h"
#include "logger.h"

static const uint8_t DRIVER_LEN { 64 };

namespace AstroAir
{
    AIRMOUNT *MOUNT = nullptr;
    std::atomic_bool isMountConnected;
    std::atomic_bool isMountSlewing;
    std::atomic_bool isMountTracking;
    std::atomic_bool isMountParked;

    /*
     * name: Connect()
     * describe: Connect from camera
     * 描述：连接（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    bool AIRMOUNT::Connect(std::string Device_name)
    {
        return true;
    }

    /*
     * name: Disconnect()
     * describe: Disconnect from camera
     * 描述：断开连接（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    bool AIRMOUNT::Disconnect()
    {
        return true;
    }

    /*
     * name: ReturnDeviceName()
     * describe: Return device's name
     * 描述：返回设备名称（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    std::string AIRMOUNT::ReturnDeviceName()
    {
        return "None";
    }

    /*
	 * name: GotoServer(std::string Target_RA,std::string Target_DEC)
     * @param Target_RA:天体RA轴坐标
     * @param Target_DEC:天体DEC轴坐标
	 * describe: slew
	 * 描述：赤道仪Goto,处理来自客户端的信息
	 * calls: IDLog() 
     * calls: WebLog()
     * calls: Goto()
	 */
    bool AIRMOUNT::GotoServer(std::string Target_RA,std::string Target_DEC)
    {
        isMountSlewing = true;
        if(MOUNT->Goto(Target_RA,Target_DEC) != true)
        {
            isMountSlewing = false;
            IDLog_Error(_("The equator doesn't work properly\n"));
            WebLog(_("The equator doesn't work properly"),3);
            return false;
        }
        isMountSlewing = false;
        IDLog("The equator moves to the designated position\n");
        WebLog("赤道仪转动到指定位置",3);
        return true;
    }

    /*
	 * name: Goto(double r, double d)
     * @param r:天体RA轴坐标
     * @param d:天体DEC轴坐标
	 * describe: slew
	 * 描述：赤道仪Goto(这只是一个模板，并没有任何实际用处)
	 */
    bool AIRMOUNT::Goto(std::string Target_RA,std::string Target_DEC)
    { 
        return false;
    }

    /*
	 * name: ParkServer(bool status)
	 * describe: park
	 * 描述：赤道仪Park,处理来自客户端的信息
	 * calls: IDLog()
     * calls: WebLog()
     * calls: Goto()
	 */
    bool AIRMOUNT::ParkServer(bool status)
    {
        if(status)      //需要归位
        {
            if(MOUNT->Park() != true)
            {
                IDLog_Error(_("The equator doesn't work properly\n"));
                WebLog(_("赤道仪无法正常工作"),3);
                return false;
            }
            isMountParked = true;
            IDLog(_("The equator moves to the designated position\n"));
            WebLog(_("赤道仪归位"),3);
            return true;
        }
        else        //解除归位
        {
            if(MOUNT->Unpark() != true)
            {
                IDLog_Error(_("The equator doesn't work properly\n"));
                WebLog(_("赤道仪无法解除归位"),3);
                return false;
            }
            isMountParked = false;
            IDLog(_("The equator moves to the designated position\n"));
            WebLog(_("赤道仪解除归位状态"),3);
            return true;
        }
        return false;
    }

    /*
	 * name: Park()
	 * describe: park
	 * 描述：赤道仪Park(这只是一个模板，并没有任何实际用处)
	 */
    bool AIRMOUNT::Park()
    { 
        return false;
    }

    /*
	 * name: Unpark()
	 * describe: Unpark
	 * 描述：赤道仪Unpark(这只是一个模板，并没有任何实际用处)
	 */
    bool AIRMOUNT::Unpark()
    { 
        return false;
    }

    /*
	 * name: TrackServer(bool status)
	 * describe: track
	 * 描述：赤道仪跟踪,处理来自客户端的信息
	 * calls: IDLog()
     * calls: WebLog()
     * calls: Track()
	 */
    bool AIRMOUNT::TrackServer(bool status)
    {
        if(MOUNT->Track(status) != true)
        {
            IDLog_Error(_("The equator doesn't work properly\n"));
            WebLog(_("赤道仪无法正常工作"),3);
            return false;
        }
        if(status == true)
        {
            IDLog(_("The equator started tracking\n"));
            WebLog(_("赤道仪开始跟踪"),3);
            isMountTracking = true;
        }
        else
        {
            IDLog(_("The equator stopped tracking\n"));
            WebLog(_("赤道仪停止跟踪"),3);
            isMountTracking = false;
        }
        return true;
    }

    /*
	 * name: Track(bool status)
	 * describe: Track
	 * 描述：赤道仪Track(这只是一个模板，并没有任何实际用处)
	 */
    bool AIRMOUNT::Track(bool status)
    { 
        return false;
    }

    /*
	 * name: AbortServer(int status)
	 * describe: Abort
	 * 描述：赤道仪停止,处理来自客户端的信息
	 * calls: IDLog()
     * calls: WebLog()
     * calls: Abort()
     * note:if status =1,it means stop tracking;2 means stop.
	 */
    bool AIRMOUNT::AbortServer(int status)
    {
        if(MOUNT->Abort(status) != true)
        {
            IDLog_Error(_("The equator doesn't work properly\n"));
            WebLog(_("赤道仪无法停止"),3);
            return false;
        }
        if(status == 1)
        {
            IDLog(_("The equator stopped tracking\n"));
            WebLog(_("赤道仪停止跟踪"),3);
        }
        else
        {
            IDLog(_("The equator stopped\n"));
            WebLog(_("赤道仪停止"),3);
        }
        return true;
    }

    /*
	 * name: Abort(int status)
	 * describe: Abort
	 * 描述：赤道仪停止(这只是一个模板，并没有任何实际用处)
	 */
    bool AIRMOUNT::Abort(int status)
    { 
        return false;
    }

    double AIRMOUNT::DecodeString(const char * data, size_t size, double factor)
    {
        return DecodeString(data, size) / factor;
    }

    int AIRMOUNT::DecodeString(const char *data, size_t size)
    {
        char str[DRIVER_LEN / 2] = {0};
        strncpy(str, data, size);
        int iVal = atoi(str);
        return iVal;
    }

    void RAConvert(std::string Target_RA,int *h,int *m,int *s)
    {
        int p_h_ra = Target_RA.find("h");
        int p_m_ra = Target_RA.find("m");
        int p_s_ra = Target_RA.find("s");
        for(int i=0;i < p_h_ra;i++)
        {
            std::string temp;
            temp += Target_RA[i];
            h = reinterpret_cast<int*>(atoi(temp.c_str()));
        }
        for(int i=p_h_ra+1;i < p_m_ra;i++)
        {
            std::string temp;
            temp += Target_RA[i];
            m = reinterpret_cast<int*>(atoi(temp.c_str()));
        }
        for(int i=p_m_ra+1;i < p_s_ra;i++)
        {
            std::string temp;
            temp += Target_RA[i];
            s = reinterpret_cast<int*>(atoi(temp.c_str()));
        }
    }

    void DECConvert(std::string Target_DEC,int *h,int *m,int *s)
    {
        int p_h_dec = Target_DEC.find("h");
        int p_m_dec = Target_DEC.find("m");
        int p_s_dec = Target_DEC.find("s");
        for(int i=0;i < p_h_dec;i++)
        {
            std::string temp;
            temp += Target_DEC[i];
            h = reinterpret_cast<int*>(atoi(temp.c_str()));
        }
        for(int i=p_h_dec+1;i < p_m_dec;i++)
        {
            std::string temp;
            temp += Target_DEC[i];
            m = reinterpret_cast<int*>(atoi(temp.c_str()));
        }
        for(int i=p_m_dec+1;i < p_s_dec;i++)
        {
            std::string temp;
            temp += Target_DEC[i];
            s = reinterpret_cast<int*>(atoi(temp.c_str()));
        }
    }
}