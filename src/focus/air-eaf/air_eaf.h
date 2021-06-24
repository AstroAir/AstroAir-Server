/*
 * air_eaf.h
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

#ifndef _AIR_EAF_H_
#define _AIR_EAF_H_

#include "../../air_focus.h"
//#include "../../wsserver.h"

#include <unistd.h>

#include <libasi/EAF_focuser.h>

namespace AstroAir
{
    class EAF:public AIRFOCUS
    {
        public:
            explicit EAF();
            ~EAF();
            virtual bool Connect(std::string Device_name)override;
            virtual bool Disconnect()override;
            std::string ReturnDeviceName()override;
            virtual bool MoveTo(int TargetPosition)override;
        protected:
            void EAF_temperature_Thread();
        private:
            int EAF_count = 0;
            int EAF_position_now = 0;
            bool InMoving = false;
            float EAF_temp = 0;

            EAF_INFO EAFInfo;
            EAF_ERROR_CODE errCode;

    };
}

#endif