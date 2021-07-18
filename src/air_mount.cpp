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
    MountInfo NEW_M;
    MountInfo *AIRMOUNTINFO = &NEW_M;

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
        AIRMOUNTINFO->Status = MOUNT_SLEWING;
        if(!MOUNT->Goto(Target_RA,Target_DEC))
        {
            AIRMOUNTINFO->Status = MOUNT_ERROR;
            IDLog_Error(_("The equator doesn't work properly\n"));
            WebLog(_("The equator doesn't work properly"),3);
            return false;
        }
        AIRMOUNTINFO->Status = MOUNT_IDLE;
        IDLog("The equator moves to the designated position\n");
        WebLog("赤道仪转动到指定位置",2);
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
     * calls: IsMountPark()
	 */
    bool AIRMOUNT::ParkServer(bool status)
    {
        if(AIRMOUNTINFO->Status == MOUNT_PARKED || AIRMOUNTINFO->Status == MOUNT_SLEWING)
        {
            IDLog_Error(_("Scopo is busy,please try again later\n"));
            WebLog(_("赤道仪繁忙中，请稍后重新尝试"),3);
            return false;
        }
        else
        {
            if (!MOUNT->IsMountPark(status))
            {
                IDLog_Error(_("The equator doesn't work properly\n"));
                WebLog(_("赤道仪无法正常工作"), 3);
                AIRMOUNTINFO->Status = MOUNT_ERROR;
                return false;
            }
            if (status)
            {
                AIRMOUNTINFO->Status = MOUNT_PARKED;
                IDLog(_("The equator moves to the designated position\n"));
                WebLog(_("赤道仪归位"), 2);
            }
            else
            {
                AIRMOUNTINFO->Status = MOUNT_IDLE;
                IDLog(_("The equator moves to the designated position\n"));
                WebLog(_("赤道仪解除归位状态"),2);
            }
        }
        return true;
    }

    /*
	 * name: IsMountPark()
	 * describe: park
	 * 描述：赤道仪Park(这只是一个模板，并没有任何实际用处)
	 */
    bool AIRMOUNT::IsMountPark(bool status)
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
        if(AIRMOUNTINFO->Status == MOUNT_PARKED || AIRMOUNTINFO->Status == MOUNT_SLEWING)
        {
            IDLog_Error(_("Scopo is busy,please try again later\n"));
            WebLog(_("赤道仪繁忙中，请稍后重新尝试"),3);
            return false;
        }
        else
        {
            if(!MOUNT->IsMountTrack(status))
            {
                IDLog_Error(_("The equator doesn't work properly\n"));
                WebLog(_("赤道仪无法正常工作"),3);
                AIRMOUNTINFO->Status = MOUNT_ERROR;
                return false;
            }
            if(status)
            {
                IDLog(_("The equator started tracking\n"));
                WebLog(_("赤道仪开始跟踪"),2);
                AIRMOUNTINFO->Status = MOUNT_TRACKING;
            }
            else
            {
                IDLog(_("The equator stopped tracking\n"));
                WebLog(_("赤道仪停止跟踪"),2);
                AIRMOUNTINFO->Status = MOUNT_IDLE;
            }
        }
        return true;
    }

    /*
	 * name: Track(bool status)
	 * describe: Track
	 * 描述：赤道仪Track(这只是一个模板，并没有任何实际用处)
	 */
    bool AIRMOUNT::IsMountTrack(bool status)
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
        if(AIRMOUNTINFO->Status == MOUNT_PARKED)
        {
            IDLog_Error(_("Scope had already parked,don't do this!\n"));
            WebLog(_("赤道仪已归位，不需要停止"),3);
            return false;
        }
        else
        {
            if(!MOUNT->IsMountAbort(status))
            {
                IDLog_Error(_("The equator doesn't work properly\n"));
                WebLog(_("赤道仪无法停止"),3);
                AIRMOUNTINFO->Status = MOUNT_ERROR;
                return false;
            }
            if(status == 1)
            {
                IDLog(_("The equator stopped tracking\n"));
                WebLog(_("赤道仪停止跟踪"),2);
            }
            else
            {
                IDLog(_("The equator stopped\n"));
                WebLog(_("赤道仪停止"),2);
            }
            AIRMOUNTINFO->Status = MOUNT_IDLE;
        }
        return true;
    }

    /*
	 * name: IsMountAbort(int status)
	 * describe: Abort
	 * 描述：赤道仪停止(这只是一个模板，并没有任何实际用处)
	 */
    bool AIRMOUNT::IsMountAbort(int status)
    { 
        return false;
    }

    bool AIRMOUNT::IsMountHomeServer(bool status)
    {
        if(AIRMOUNTINFO->Status == MOUNT_PARKED)
        {
            IDLog_Error(_("Scope had already parked,don't do this!\n"));
            WebLog(_("赤道仪已归位，不需要停止"),3);
            return false;
        }
        else
        {
            if (!MOUNT->IsMountHome(status))
            {
                IDLog_Error(_("The equator doesn't work properly\n"));
                WebLog(_("赤道仪无法正常工作"), 3);
                AIRMOUNTINFO->Status = MOUNT_ERROR;
                return false;
            }
            AIRMOUNTINFO->Status = MOUNT_IDLE;
            IDLog(_("The equator moves to the home position\n"));
            WebLog(_("赤道仪已回到家位置"), 2);
        }
        return true;
    }

    /*
	 * name: IsMountHome(int status)
	 * describe: Home
	 * 描述：赤道仪回位(这只是一个模板，并没有任何实际用处)
	 */
    bool AIRMOUNT::IsMountHome(bool status)
    {
        return false;
    }
}