/*
 * inditelescope.h
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

Date:2020-7-19

Description:INDI Telescope driver

**************************************************/

#ifndef _INDI_TELESCOPE_H_
#define _INDI_TELESCOPE_H_

#include "../air_mount.h"

#include <libindi/baseclient.h>

namespace AstroAir
{
    class INDISCOPE:public AIRMOUNT,public INDI::BaseClient
    {
        public:
            explicit INDISCOPE(MountInfo *NEW);
            ~INDISCOPE();
        protected:
            void newDevice(INDI::BaseDevice *dp) override {}
            #ifndef INDI_PRE_1_0_0
            void removeDevice(INDI::BaseDevice *dp) override {}
            #endif
            void newProperty(INDI::Property *property) override {}
            void removeProperty(INDI::Property *property) override {}
            void newBLOB(IBLOB *bp) override {}
            void newSwitch(ISwitchVectorProperty *svp) override {}
            void newNumber(INumberVectorProperty *nvp) override {}
            void newMessage(INDI::BaseDevice *dp, int messageID) override {}
            void newText(ITextVectorProperty *tvp) override {}
            void newLight(ILightVectorProperty *lvp) override {}
            void serverConnected() override {}
    };
}

#endif