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
#include "wsserver.h"

namespace AstroAir
{
    AIRGUIDER *GUIDE;
    std::atomic_bool isGuideConnected;
    double Guide_RA,Guide_DEC;
    std::atomic_bool IsGuiding;

    AIRGUIDER::AIRGUIDER()
    {
        IsGuiding = false;
    }
    
    AIRGUIDER::~AIRGUIDER()
    {

    }

    //------------------------------常规------------------------------

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

    //------------------------------导星------------------------------

    bool AIRGUIDER::StartGuidingServer()
    {
        if(isGuideConnected && IsGuiding == false)
        {
            if(GUIDE->StartGuiding() != false)
            {
                IDLog_Error(_("Could not start guiding\n"));
                WebLog(_("Could not start guiding!"),3);
                return false;
            }
        }
        return true;
    }

    /*
     * name: StartGuiding()
     * describe: Start Guiding
     * 描述：开始导星（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    bool AIRGUIDER::StartGuiding()
    {
        return false;
    }

    bool AIRGUIDER::AbortGuidingServer()
    {
        if(!GUIDE->AbortGuiding())
        {
            IDLog_Error(_("Could not stop Guider,you telescope is in danger!!!\n"));
            WebLog(_("Could not stop Guider!"),3);
            return false;
        }
        return true;
    }

    /*
     * name: AbortGuiding()
     * describe: Stop Calibrating
     * 描述：停止导星（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    bool AIRGUIDER::AbortGuiding()
    {
        return false;
    }

    bool AIRGUIDER::DitherServer()
    {
        if(!GUIDE->Dither())
        {
            IDLog_Error(_("Failed to dither\n"));
            WebLog(_("Failed to Dither"),3);
            return false;
        }
        return true;
    }

    /*
     * name: Dither()
     * describe: Dither
     * 描述：抖动（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    bool AIRGUIDER::Dither()
    {
        return false;
    }
}