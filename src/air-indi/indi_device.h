/*
 * indi_device.h
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

Date:2020-2-15

Description:INDI driver

**************************************************/

#pragma once

#ifndef _INDI_DEVICE_H_
#define _INDI_DEVICE_H_

#include "../wsserver.h"
#include "indi_client.h"

#include <atomic>

namespace AstroAir
{
    class INDICCD : public WSSERVER
    {
        public:
            /*构造函数，重置参数*/
			explicit INDICCD();
            /*析构函数*/
            ~INDICCD();
            /*连接相机*/
            virtual bool Connect(std::string Device_name) override;
            /*断开连接*/
			virtual bool Disconnect() override;
        private:
            /*相机使用参数（使用原子变量）*/
			std::atomic_bool isConnected;
			std::atomic_bool InExposure;
			std::atomic_bool InVideo;
			std::atomic_bool InCooling;
    };
}

#endif