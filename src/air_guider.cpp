/*
 * air_guider.cpp
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
 
Date:2021-6-26
 
Description:Guider offical port

**************************************************/

#include "air_guider.h"
#include "logger.h"

namespace AstroAir
{
    AIRGUIDER *GUIDE;
    std::atomic_bool isGuideConnected;

    AIRGUIDER::AIRGUIDER()
    {

    }
    
    AIRGUIDER::~AIRGUIDER()
    {

    }

    /*
     * name: Connect()
     * describe: Connect from guider
     * 描述：连接（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    bool AIRGUIDER::Connect(std::string Device_name)
    {
        return true;
    }

    /*
     * name: Disconnect()
     * describe: Disconnect from guider
     * 描述：断开连接（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    bool AIRGUIDER::Disconnect()
    {
        return true;
    }

    /*
     * name: ReturnDeviceName()
     * describe: Return device's name
     * 描述：返回设备名称（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    std::string AIRGUIDER::ReturnDeviceName()
    {
        return "None";
    }
}