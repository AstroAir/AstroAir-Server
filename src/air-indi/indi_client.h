/*
 * indi_client.h
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

#ifndef _INDI_CLIENT_H_
#define _INDI_CLIENT_H_

#include <libindi/baseclient.h>

namespace AstroAir
{
    class INDIClient : public INDI::BaseClient
    {
    public:
        INDIClient();
        ~INDIClient();
        void setTemperature();
    protected:
        virtual void newDevice(INDI::BaseDevice *dp);
        virtual void removeDevice(INDI::BaseDevice *dp) {}
        virtual void newProperty(INDI::Property *property);
        virtual void removeProperty(INDI::Property *property) {}
        virtual void newBLOB(IBLOB *bp) {}
        virtual void newSwitch(ISwitchVectorProperty *svp) {}
        virtual void newNumber(INumberVectorProperty *nvp);
        virtual void newMessage(INDI::BaseDevice *dp, int messageID);
        virtual void newText(ITextVectorProperty *tvp) {}
        virtual void newLight(ILightVectorProperty *lvp) {}
        virtual void serverConnected() {}
        virtual void serverDisconnected(int exit_code) {}
    private:
        INDI::BaseDevice * CCD;
    };
}

#endif