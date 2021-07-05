/*
 * air_filter.cpp
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
 
Date:2021-6-24
 
Description:Filter port

**************************************************/

#include "air_filter.h"
#include "wsserver.h"
#include "logger.h"

namespace AstroAir
{
    AIRFILTER *FILTER = nullptr;
    std::atomic_bool isFilterConnected;

    /*
     * name: Connect()
     * describe: Connect from camera
     * 描述：连接（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    bool AIRFILTER::Connect(std::string Device_name)
    {
        return true;
    }

    /*
     * name: Disconnect()
     * describe: Disconnect from camera
     * 描述：断开连接（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    bool AIRFILTER::Disconnect()
    {
        return true;
    }

    std::string AIRFILTER::ReturnDeviceName()
    {
        return "None";
    }

    /*
     * name: FilterMoveToServer(int TargetPosition)
     * @param TargetPosition:滤镜轮将运动至的位置
     * describe: Filter move （Server）
     * 描述：滤镜轮运动至   （服务器）
     * calls: FilterMoveTo(int TargetPosition)
     * calls: IDLog(const char *fmt, ...)
     * calls: WebLog()
	 * calls: FilterMoveToError(）
     * calls: FilterMoveToSuccess()
     */
    bool AIRFILTER::FilterMoveToServer(int TargetPosition)
    {
        if(TargetPosition < 0)
        {
            IDLog(_("Target position is less than 0, please input a reasonable data\n"));
            WebLog(_("Are you kidding me?!"),3);
            return false;
        }
        if(isFilterConnected == true)
        {
            if(InMoving == true)
            {
                IDLog(_("Filter is moving,try again later!\n"));
                WebLog(_("Filter is moving,try again later!"),3);
                return false;
            }
            if(FILTER->FilterMoveTo(TargetPosition) != true)
            {
                /*返回原因*/
				FilterMoveToError();
				IDLog(_("The filter failed to move to %d\n"),TargetPosition);
                WebLog(_("The filter failed to move to."),3);
				InMoving = false;
				return false;
            }
            InMoving = false;
            FilterMoveToSuccess();
            WebLog(_("Filter move to  ok"),2);
        }
        else
        {
            IDLog("There seems to be some unknown mistakes here.Maybe you need to check the filter connection\n");
			return false;
        }
        return true;
    }

    bool AIRFILTER::FilterMoveTo(int TargetPosition)
    {
        return true;
    }

    void AIRFILTER::FilterMoveToSuccess()
    {
        return ;
    }

    void AIRFILTER::FilterMoveToError()
    {
        return ;
    }
}