/*
 * air_mount.h
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

#ifndef _AIR_MOUNT_H_
#define _AIR_MOUNT_H_

#include <string>

namespace AstroAir
{
    /*这里的大部分函数都是没有任何实际用途的
      只能作为一个模板参考
      不过基本的控制逻辑就是这个样子
      所有的_Server函数都是有用的
      在派生类中不用重新写*/
    class AIRMOUNT
    {
        public:
            virtual bool Connect(std::string Device_name);
			virtual bool Disconnect();
			virtual std::string ReturnDeviceName();
            virtual bool GotoServer(std::string Target_RA,std::string Target_DEC);
            virtual bool Goto(std::string Target_RA,std::string Target_DEC);
            virtual bool ParkServer();
            virtual bool Park();
            virtual bool Cancel_Park();
            virtual bool Track();
            virtual bool Cancel_Track();
        private:

    };
    extern AIRMOUNT *MOUNT;
}

#endif