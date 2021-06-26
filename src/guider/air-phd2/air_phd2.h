/*
 * air_phd2.h
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
 
Description:PD2 offical port

**************************************************/

#ifndef _AIR_PHD2_H_
#define _AIR_PHD2_H_

#include "../../air_guider.h"

namespace AstroAir
{
    class PHD2:public AIRGUIDER
    {
        public:
            explicit PHD2();
            ~PHD2();
            virtual bool Connect(std::string Device_name)override;      //连接PHD2
		    virtual bool Disconnect()override;                          //断开连接
		    virtual std::string ReturnDeviceName()override;             //返回设备名称
        private:

    };
}

#endif