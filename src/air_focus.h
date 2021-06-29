/*
 * air_focus.h
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
 
Description:Focus offical port

**************************************************/

#ifndef _AIR_FOCUS_H_
#define _AIR_FOCUS_H_

#include <string>
#include <atomic>

namespace AstroAir
{
    class AIRFOCUS
    {
        public:
            explicit AIRFOCUS();
            ~AIRFOCUS();
            virtual bool Connect(std::string Device_name);      //连接电动调焦座
			virtual bool Disconnect();                          //断开连接
			virtual std::string ReturnDeviceName();             //返回设备名称
            virtual bool MoveToServer(int TargetPosition);
            virtual bool MoveTo(int TargetPosition);
            virtual bool MoveServer(int Steps);
            virtual bool Move(int Steps);

        private:
            virtual void MoveToError();
            virtual void MoveToSuccess();
            virtual void MoveError();
            virtual void MoveSuccess();

            std::atomic_bool InMoving;
    };
    extern AIRFOCUS *FOCUS;
    extern std::atomic_bool isFocusConnected;
    extern double FocusTemp;
    extern std::atomic_int FocusPosition;
}
#endif