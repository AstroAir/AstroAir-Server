/*
 * air_efw.h
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

#ifndef _AIR_EFW_H_
#define _AIR_EFW_H_

#include <libasi/EFW_filter.h>

#include "../../air_filter.h"

namespace AstroAir
{
    class EFW:public AIRFILTER
    {
        public:
            explicit EFW();
            ~EFW();
            virtual bool Connect(std::string Device_name)override;
            virtual bool Disconnect()override;
            std::string ReturnDeviceName()override;
            virtual bool FilterMoveTo(int TargetPosition)override;
        private:
            int EFW_count = 0;
            int EFW_position_now = 0;
            bool InMoving = false;
            float EFW_temp = 0;

            EFW_INFO EFWInfo;
            EFW_ERROR_CODE errCode;
    };
}

#endif