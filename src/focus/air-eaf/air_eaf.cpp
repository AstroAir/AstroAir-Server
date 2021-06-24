/*
 * air_eaf.cpp
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
 
Description:EAF Focus offical port

**************************************************/

#include "air_eaf.h"
#include "../../logger.h"

namespace AstroAir
{
    /*
     * name: EAF()
     * describe: Constructor for initializing focuser parameters
     * 描述：构造函数，用于初始化电动调焦座参数
     */
    EAF::EAF()
    {
        EAF_count = 0;
        EAF_position_now = 0;
        InMoving = false;
        EAF_temp = 0;
    }

    /*
     * name: ~EAF()
     * describe: Destructor
     * 描述：析构函数
     */
    EAF::~EAF()
    {
        if(isFocusConnected == true)
         Disconnect();
    }

    /*
     * name: Connect(std::string Device_name)
     * @param Device_name:连接电动调焦座名称
     * describe: Connect the focuser
     * 描述： 连接电动调焦座
     * calls: EAFGetNum()
     * calls: EAFGetID()
     * calls: EAFGetProperty()
     * calls: EAFOpen()
     * calls: EAFGetPosition()
     * calls: IDLog()
     * @return true: Focuser connected successfully
     * @return false: Failed to connect focuser
     * note: Do not connect the focuser repeatedly
     */
    bool EAF::Connect(std::string Device_name)
    {
        if((EAF_count = EAFGetNum()) <= 0)
        {
            IDLog(_("No focuser connected\n"));
            return false;
        } 
        for(int i = 0; i < EAF_count; i++)
        {
            if((errCode = EAFGetID(i, &EAFInfo.ID)) != EAF_SUCCESS || (errCode = EAFGetProperty(EAFInfo.ID, &EAFInfo)) != EAF_SUCCESS)
            {
                IDLog(_("Unable to get information,the error code is %d,please check program permissions.\n"),errCode);
				return false;
            }
            if(EAFInfo.Name == Device_name)
            {
                IDLog(_("Find %s"),EAFInfo.Name);
                if((errCode = EAFOpen(EAFInfo.ID)) != EAF_SUCCESS)
                {
                    IDLog(_("Could not open the focuser!\n"));
                    return false;
                }
                isFocusConnected = true;
                IDLog(_("Focuser turned on successfully!\n"));
                EAFGetPosition(EAFInfo.ID, &EAF_position_now);
                return true;
            }
        }
        return false;
    }

    /*
     * name: Disconnect()
     * describe: Disconnect from focuser
     * 描述：与电动调焦座断开连接
     * calls: EAFStop()
     * calls: EAFClose()
     * calls: IDLog()
     * note: Please stop all work before turning off the focuser
     */
    bool EAF::Disconnect()
    {
        if(InMoving == true)
        {
            if((errCode = EAFStop(EAFInfo.ID)) != EAF_SUCCESS)    //停止任务
            {
                IDLog(_("Could not stop focuser\n"));
                return false;
            }
        }
        if((errCode = EAFClose(EAFInfo.ID)) != EAF_SUCCESS)
        {
            IDLog(_("Failed to close focuser with error code %d"),errCode);
            return false;
        }
        return true;
    }

    /*
     * name: ReturnDeviceName()
     * describe: Return focuser's name
     * 描述：返回设备名称
     */
    std::string EAF::ReturnDeviceName()
    {
        return EAFInfo.Name;
    }

    /*
     * name: MoveTo(int TargetPosition)
     * @param TargetPosition:将运动至的位置
     * describe: Focuser move to specially position
     * 描述： 电动调焦座移动至指定位置
     * calls: EAFGetProperty()
     * calls: EAFMove()
     * calls: EAFGetPosition()
     * calls: EAFIsMoving()
     * calls: IDLog()
     * @return true: Focuser connected successfully
     * @return false: Failed to connect focuser
     * note: Do not use some strange number
     */
    bool EAF::MoveTo(int TargetPosition)
    {
        /*检查是否正在移动*/
        if((errCode = EAFGetProperty(EAFInfo.ID, & EAFInfo)) == EAF_ERROR_MOVING)
        {
            IDLog(_("Focuser is busy\n"));
            return false;
        }
        /*检查输入的数据是不是很离谱*/
        if(TargetPosition < 0 || TargetPosition > EAFInfo.MaxStep)
        {
            IDLog(_("Target position is less than 0, please input a reasonable data\n"));
            return false;
        }
        /*EAF运动*/
        if((errCode = EAFMove(EAFInfo.ID,TargetPosition)) != EAF_SUCCESS)
        {
            IDLog(_("Focuser failed to move\n"));
            return false;
        }
        while(1)
		{
			EAFGetPosition(EAFInfo.ID, &EAF_position_now);
			bool pbHandControl;
			if((errCode = EAFIsMoving(EAFInfo.ID, &InMoving, &pbHandControl)) != EAF_SUCCESS || !InMoving)
                break;
            usleep(500);
		} 
        EAFGetPosition(EAFInfo.ID, &EAF_position_now);
        if(EAF_position_now == TargetPosition)
        {
            IDLog(_("Focuser's moving finished\n"));
            return true;
        }
        else
            IDLog(_("Focuser has something wrong!\n"));
        return false;
    }

    void EAF::EAF_temperature_Thread()
    {
        while(isFocusConnected)
        {
            EAFGetTemp(EAFInfo.ID,&EAF_temp);
            sleep(1);
        } 
    }
}