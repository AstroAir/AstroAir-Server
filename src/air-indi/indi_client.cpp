/*
 * indi_client.cpp
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

#include "indi_client.h"
#include "../logger.h"

#include <string.h>

#define MYCCD "CCD Simulator"

namespace AstroAir
{
    INDIClient::INDIClient()
    {

    }

    INDIClient::~INDIClient()
    {

    }

    void INDIClient::newDevice(INDI::BaseDevice *dp)
    {
        if (!strcmp(dp->getDeviceName(), MYCCD))
        {
            IDLog("Receiving %s Device...\n", dp->getDeviceName());
            CCD = dp;
        }
    }

    void INDIClient::newProperty(INDI::Property *property)
    {
        if (!strcmp(property->getDeviceName(), MYCCD) && !strcmp(property->getName(), "CONNECTION"))
        {
            connectDevice(MYCCD);
            return;
        }

        if (!strcmp(property->getDeviceName(), MYCCD) && !strcmp(property->getName(), "CCD_TEMPERATURE"))
        {
            if (CCD->isConnected())
            {
                IDLog("CCD is connected. Setting temperature to -20 C.\n");
                setTemperature();
            }
            return;
        }
    }

    void INDIClient::newNumber(INumberVectorProperty *nvp)
    {
        // Let's check if we get any new values for CCD_TEMPERATURE
        if (!strcmp(nvp->name, "CCD_TEMPERATURE"))
        {
            IDLog("Receving new CCD Temperature: %g C\n", nvp->np[0].value);
            if (nvp->np[0].value == -20)
                IDLog("CCD temperature reached desired value!\n");
        }
    }

    void INDIClient::setTemperature()
    {
        INumberVectorProperty *ccd_temperature = NULL;
        ccd_temperature = CCD->getNumber("CCD_TEMPERATURE");
        if (ccd_temperature == NULL)
        {
            IDLog("Error: unable to find CCD Simulator CCD_TEMPERATURE property...\n");
            return;
        }
        ccd_temperature->np[0].value = -20;
        sendNewNumber(ccd_temperature);
    }
}