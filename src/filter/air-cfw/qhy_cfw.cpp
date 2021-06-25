/*
 * qhy_cfw.cpp
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
 
Date:2021-6-25
 
Description:CFW Filter offical port

**************************************************/

#include "qhy_cfw.h"
#include "../../logger.h"

#include <unistd.h>

namespace AstroAir
{
    CFW::CFW()
    {
        retVal = QHYCCD_SUCCESS;
    }

    CFW::~CFW()
    {

    }

    /*
     * name: Connect(std::string Device_name)
     * @param Device_name:连接滤镜轮名称
     * describe: Connect the filter
     * 描述： 连接滤镜轮
     * calls: IsQHYCCDCFWPlugged()
     * calls: GetQHYCCDParam()
     * calls: GetQHYCCDCFWStatus()
     * calls: IDLog()
     * @return true: Filter connected successfully
     * @return false: Failed to connect filter
     * note: Do not connect the filter repeatedly
     */
    bool CFW::Connect(std::string Device_name)
    {
        if(IsQHYCCDCFWPlugged(pFilterHandle) != QHYCCD_SUCCESS)     //寻找滤镜轮
        {
            IDLog(_("Could not found any qhy filter active\n"));
            return false;
        }
        Info.MaxSlot = GetQHYCCDParam(pFilterHandle, CONTROL_CFWSLOTSNUM);       //获取滤镜轮的最大位置
        if(GetQHYCCDCFWStatus(pFilterHandle,Info.CFW_position_now) != QHYCCD_SUCCESS)
        {
            IDLog(_("Could not get filter's infomation\n"));
            return false;
        }
        isFilterConnected = true;
        return true;
    }

    bool CFW::Disconnect()
    {
        return true;
    }

    /*
     * name: ReturnDeviceName()
     * describe: Return filter's name
     * 描述：返回设备名称
     */
    std::string CFW::ReturnDeviceName()
    {
        return "QHYCFW";
    }

    /*
     * name: FilterMoveTo(int TargetPosition)
     * @param TargetPosition:将运动至的位置
     * describe: Filter move to specially position
     * 描述： 滤镜轮移动至指定位置
     * calls: SendOrder2QHYCCDCFW()
     * calls: GetQHYCCDCFWStatus()
     * calls: GetQHYCCDCFWStatus()
     * calls: IDLog()
     * @return true: Filter connected successfully
     * @return false: Failed to connect filter
     * note: Do not use some strange number
     */
    bool CFW::FilterMoveTo(int TargetPosition)
    {
        if((Info.CFW_position_now[0] - '0') == TargetPosition)      //滤镜轮已在指定位置
        {
            IDLog(_("Filter has already moved to target position\n"));
            return true;
        }
        if(TargetPosition < 0 || TargetPosition > 8)        //检测数据是否合理
        {
            IDLog(_("Target position is less than 0, please input a reasonable data\n"));
            return false;
        }
        char targetpos = 0;
        int checktimes = 0;
        targetpos = '0' + (TargetPosition - 1);
        if(SendOrder2QHYCCDCFW(pFilterHandle,&targetpos,1) != QHYCCD_SUCCESS)       //滤镜轮运动
        {
            IDLog(_("Filter could not move!\n"));
            return false;
        }
        while(checktimes <50)       //检测滤镜轮是否运动到指定位置
        {
            if((retVal = GetQHYCCDCFWStatus(pFilterHandle,Info.CFW_position_now)) == QHYCCD_SUCCESS && Info.CFW_position_now[0] == targetpos)
            {
                IDLog(_("Filter move to %d position\n"),TargetPosition);
                return true;
            }
            usleep(1000*500);
            checktimes++;
        }
        IDLog(_("Changing filter failed (%d)"),retVal);
        return false;
    }
}