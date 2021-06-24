/*
 * air_efw.cpp
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
 
Description:EFW Filter offical port

**************************************************/

#include "air_efw.h"
#include "../../logger.h"

#include <unistd.h>

namespace AstroAir
{
    /*
     * name: EFW()
     * describe: Constructor for initializing focuser parameters
     * 描述：构造函数，用于初始化滤镜轮参数
     */
    EFW::EFW()
    {
        EFW_count = 0;
        EFW_position_now = 0;
        InMoving = false;
        EFW_temp = 0;
    }

    /*
     * name: ~EFW()
     * describe: Destructor
     * 描述：析构函数
     */
    EFW::~EFW()
    {
        if(isFilterConnected == true)
            Disconnect();
    }

    /*
     * name: Connect(std::string Device_name)
     * @param Device_name:连接滤镜轮名称
     * describe: Connect the filter
     * 描述： 连接电动调焦座
     * calls: EFWGetNum()
     * calls: EFWGetID()
     * calls: EFWGetProperty()
     * calls: EFWOpen()
     * calls: EFWGetPosition()
     * calls: IDLog()
     * @return true: filter connected successfully
     * @return false: Failed to connect filter
     * note: Do not connect the filter repeatedly
     */
    bool EFW::Connect(std::string Device_name)
    {
        if((EFW_count = EFWGetNum()) <= 0)
        {
            IDLog(_("No filter connected\n"));
            return false;
        } 
        for(int i = 0; i < EFW_count; i++)
        {
            if((errCode = EFWGetID(i, &EFWInfo.ID)) != EFW_SUCCESS || (errCode = EFWGetProperty(EFWInfo.ID, &EFWInfo)) != EFW_SUCCESS)
            {
                IDLog(_("Unable to get information,the error code is %d,please check program permissions.\n"),errCode);
				return false;
            }
            if(EFWInfo.Name == Device_name)
            {
                IDLog(_("Find %s"),EFWInfo.Name);
                if((errCode = EFWOpen(EFWInfo.ID)) != EFW_SUCCESS)
                {
                    IDLog(_("Could not open the filter!\n"));
                    return false;
                }
                isFilterConnected = true;
                IDLog(_("Filter turned on successfully!\n"));
                EFWGetPosition(EFWInfo.ID, &EFW_position_now);
                return true;
            }
        }
        return false;
    }

    /*
     * name: Disconnect()
     * describe: Disconnect from filter
     * 描述：与滤镜轮断开连接
     * calls: EFWClose()
     * calls: IDLog()
     * note: Please stop all work before turning off the filter
     */
    bool EFW::Disconnect()
    {
        if((errCode = EFWClose(EFWInfo.ID)) != EFW_SUCCESS)
        {
            IDLog(_("Failed to close Filter with error code %d"),errCode);
            return false;
        }
        return true;
    }

    std::string EFW::ReturnDeviceName()
    {
        return EFWInfo.Name;
    }

    bool EFW::FilterMoveTo(int TargetPosition)
    {
        /*检查是否正在移动*/
        if((errCode = EFWGetProperty(EFWInfo.ID, & EFWInfo)) == EFW_ERROR_MOVING)
        {
            IDLog(_("Filter is busy\n"));
            return false;
        }
        /*检查输入的数据是不是很离谱*/
        if(TargetPosition < 0 || TargetPosition > 9)
        {
            IDLog(_("Target position is less than 0, please input a reasonable data\n"));
            return false;
        }
        /*EFW运动*/
        if((errCode = EFWSetPosition(EFWInfo.ID,TargetPosition)) != EFW_SUCCESS)
        {
            IDLog(_("Filter failed to move\n"));
            return false;
        }
        while(1)
		{
			if((errCode = EFWGetPosition(EFWInfo.ID, &EFW_position_now)) != EFW_SUCCESS || EFW_position_now != -1)
                break;
            usleep(500);
		} 
        EFWGetPosition(EFWInfo.ID, &EFW_position_now);
        if(EFW_position_now == TargetPosition)
        {
            IDLog(_("Filter's moving finished\n"));
            return true;
        }
        else
            IDLog(_("Filter has something wrong!\n"));
        return false;
    }
}