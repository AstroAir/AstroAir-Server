/*
 * qhy_cfw.h
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

#ifndef _QHY_CFW_H_
#define _QHY_CFW_H_

#include "../../air_filter.h"

#include <libqhy/qhyccd.h>

namespace AstroAir
{
    class CFW:public AIRFILTER
    {
        public:
            explicit CFW();
            ~CFW();
            virtual bool Connect(std::string Device_name)override;
            virtual bool Disconnect()override;
            std::string ReturnDeviceName()override;
            virtual bool FilterMoveTo(int TargetPosition)override;
        private:
            uint32_t retVal;
            struct CFWInfo
            {
                double MaxSlot = 0;
                char CFW_position_now[64] = {0};
            };
            CFWInfo Info;

            qhyccd_handle *pFilterHandle;
    };
}

#endif