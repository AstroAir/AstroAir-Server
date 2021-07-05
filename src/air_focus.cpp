/*
 * air_focus.cpp
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
 
Description:Focus offical port

**************************************************/

#include "air_focus.h"
#include "wsserver.h"
#include "logger.h"

namespace AstroAir
{
    AIRFOCUS *FOCUS = nullptr;
    std::atomic_bool isFocusConnected;
    double FocusTemp;
    std::atomic_int FocusPosition;
    
    /*
     * name: AIRFOCUS()
     * describe: Constructor for initializing focuser parameters
     * 描述：构造函数，用于初始化电动调焦座参数
     */
    AIRFOCUS::AIRFOCUS()
    {
        InMoving = false;
    }

    /*
     * name: ~AIRFOCUS()
     * describe: Destructor
     * 描述：析构函数
     */
    AIRFOCUS::~AIRFOCUS()
    {
        InMoving = false;
        if(isFocusConnected)
            FOCUS->Disconnect();
    }

    /*
     * name: Connect()
     * describe: Connect from camera
     * 描述：连接（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    bool AIRFOCUS::Connect(std::string Device_name)
    {
        return true;
    }

    /*
     * name: Disconnect()
     * describe: Disconnect from camera
     * 描述：断开连接（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    bool AIRFOCUS::Disconnect()
    {
        return true;
    }

    /*
     * name: ReturnDeviceName()
     * describe: Return device's name
     * 描述：返回设备名称（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    std::string AIRFOCUS::ReturnDeviceName()
    {
        return "None";
    }

    /*
     * name: MoveToServer(int TargetPosition)
     * @param TargetPosition:电动调焦座将运动至的位置
     * describe: Focuser move （Server）
     * 描述：电动调焦座运动至   （服务器）
     * calls: MoveTo(int TargetPosition)
     * calls: IDLog(const char *fmt, ...)
     * calls: WebLog()
	 * calls: MoveToError(）
     * calls: MoveToSuccess()
     */
    bool AIRFOCUS::MoveToServer(int TargetPosition)
    {
        if(TargetPosition < 0)
        {
            IDLog(_("Target position is less than 0, please input a reasonable data\n"));
            WebLog(_("Are you kidding me?!"),3);
            return false;
        }
        if(isFocusConnected)
        {
            if(InMoving)
            {
                IDLog(_("Focus is moving,try again later!\n"));
                WebLog(_("Focus is moving,try again later!"),3);
                return false;
            }
            if(!FOCUS->MoveTo(TargetPosition))
            {
                /*返回原因*/
				MoveToError();
				IDLog(_("The focus failed to move to %d\n"),TargetPosition);
                WebLog(_("The focus failed to move to."),3);
				InMoving = false;
				return false;
            }
            InMoving = false;
            MoveToSuccess();
            WebLog(_("Focus move to  ok"),2);
        }
        else
        {
            IDLog("There seems to be some unknown mistakes here.Maybe you need to check the focus connection\n");
			return false;
        }
        return true;
    }

    /*
     * name: MoveTo(int TargetPosition)
     * describe: Move to position
     * 描述：运动至（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    bool AIRFOCUS::MoveTo(int TargetPosition)
    {
        return true;
    }

    void AIRFOCUS::MoveToSuccess()
    {
        return ;
    }

    void AIRFOCUS::MoveToError()
    {
        return ;
    }

    /*
     * name: MoveServer(int Steps)
     * @param TargetPosition:电动调焦座将运动至的位置
     * describe: Focuser move （Server）
     * 描述：电动调焦座运动至   （服务器）
     * calls: Move(int Steps)
     * calls: IDLog(const char *fmt, ...)
     * calls: WebLog()
	 * calls: MoveError(）
     * calls: MoveSuccess()
     */
    bool AIRFOCUS::MoveServer(int Steps)
    {
        if(isFocusConnected)
        {
            if(InMoving)
            {
                IDLog(_("Focus is moving,try again later!\n"));
                WebLog(_("Focus is moving,try again later!"),3);
                return false;
            }
            if(!FOCUS->MoveTo(Steps))
            {
                /*返回原因*/
				MoveError();
				IDLog(_("The focus failed to move %d\n"),Steps);
                WebLog(_("The focus failed to move."),3);
				InMoving = false;
				return false;
            }
            InMoving = false;
            MoveSuccess();
            WebLog(_("Focus move ok"),2);
        }
        else
        {
            IDLog_Error(_("There seems to be some unknown mistakes here.Maybe you need to check the focus connection\n"));
			return false;
        }
        return true;
    }

    /*
     * name: MoveTo(int Steps)
     * describe: Move to position
     * 描述：运动至（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    bool AIRFOCUS::Move(int Steps)
    {
        return true;
    }

    void AIRFOCUS::MoveSuccess()
    {

    }

    void AIRFOCUS::MoveError()
    {

    }
}