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
 
Date:2021-6-23
 
Description:Mount Offical Port

**************************************************/

#include "air_mount.h"
#include "wsserver.h"
#include "logger.h"


namespace AstroAir
{
    AIRMOUNT *MOUNT;
    std::atomic_bool isMountConnected;

    bool AIRMOUNT::Connect(std::string Device_name)
    {
        return true;
    }

    bool AIRMOUNT::Disconnect()
    {
        return true;
    }

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
        bool mount_ok = false;
        if((mount_ok = MOUNT->Goto(Target_RA,Target_DEC)) != true)
        {
            IDLog("The equator doesn't work properly\n");
            WebLog("赤道仪无法正常工作",3);
            return false;
        }
        IDLog("The equator moves to the designated position\n");
        WebLog("赤道仪转动到指定位置",3);
        return true;
    }

    /*
	 * name: Goto(std::string Target_RA,std::string Target_DEC)
     * @param Target_RA:天体RA轴坐标
     * @param Target_DEC:天体DEC轴坐标
	 * describe: slew
	 * 描述：赤道仪Goto(这只是一个模板，并没有任何实际用处)
	 */
    bool AIRMOUNT::Goto(std::string Target_RA,std::string Target_DEC)
    { 
        return true;
    }

    /*
	 * name: ParkServer()
	 * describe: park
	 * 描述：赤道仪Park,处理来自客户端的信息
	 * calls: IDLog()
     * calls: WebLog()
     * calls: Goto()
	 */
    bool AIRMOUNT::ParkServer()
    {
        bool mount_ok = false;
        if((mount_ok = MOUNT->Park()) != true)
        {
            IDLog("The equator doesn't work properly\n");
            WebLog("赤道仪无法正常工作",3);
            return false;
        }
        IDLog("The equator moves to the designated position\n");
        WebLog("赤道仪转动到指定位置",3);
        return true;
    }

    /*
	 * name: Park()
	 * describe: park
	 * 描述：赤道仪Park(这只是一个模板，并没有任何实际用处)
	 */
    bool AIRMOUNT::Park()
    { 
        return true;
    }

    /*
	 * name: Cancel_Park()
	 * describe: park
	 * 描述：赤道仪Park(这只是一个模板，并没有任何实际用处)
	 */
    bool AIRMOUNT::Cancel_Park()
    { 
        return true;
    }

    /*
	 * name: Track()
	 * describe: Track
	 * 描述：赤道仪Track(这只是一个模板，并没有任何实际用处)
	 */
    bool AIRMOUNT::Track()
    { 
        return true;
    }

    /*
	 * name: Cancel_Track()
	 * describe: Track
	 * 描述：赤道仪Track(这只是一个模板，并没有任何实际用处)
	 */
    bool AIRMOUNT::Cancel_Track()
    { 
        return true;
    }
}