/*
 * indiccd.cpp
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

Description:INDI camera driver

**************************************************/

#include "indiccd.h"
#include "../logger.h"

#include <cmath>
#include <fstream>

namespace AstroAir
{
    INDICCD::INDICCD(CameraInfo *NEW)
    {
        INDICAMERA = NEW;
        InitINDICamera();
    }

    INDICCD::~INDICCD()
    {
        if(INDICAMERA->isCameraConnected)
            Disconnect();
    }

    bool INDICCD::Connect(std::string Device_name)
    {
        InitINDICamera();
        IDLog(_("INDI Camera connecting to device [%s]\n"), INDICAMERA->Name[0]);
        // define server to connect to.
        setServer(INDICAMERA->host.c_str(), INDICAMERA->port);
        // Receive messages only for our camera.
        watchDevice(INDICAMERA->Name[0]);
        // Connect to server.
        if (connectServer())
        {
            IDLog(_("INDI Camera: connectServer done ready = %d\n"), INDISS.ready);
            connectDevice(INDICAMERA->Name[0]);
            return true;
        }
        return false;
    }

    bool INDICCD::Disconnect()
    {
        disconnectServer();
        return true;
    }

    std::string INDICCD::ReturnDeviceName()
    {
        return INDICAMERA->Name[0];
    }

    /*
     * name: StartExposure(int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset)
     * @param exp:相机曝光时间
     * @param bin:像素合并
     * @param IsSave:是否保存图像
     * @param FitsName:保存图像名称
     * @param Gain:相机增益
     * @param Offset:相机偏置
     * describe: Start exposure
     * 描述：开始曝光（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    bool INDICCD::StartExposure(int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset)
    {
        return true;
    }

    /*
     * name: AbortExposure()
     * describe: Abort exposure
     * 描述：停止曝光
	 * note:This function should not be executed normally
     */
    bool INDICCD::AbortExposure()
    {
        return false;
    }

    /*
     * name: Cooling(bool SetPoint,bool CoolDown,bool ASync,bool Warmup,bool CoolerOFF,int CamTemp)
     * describe: Camera Cooling Settings
     * 描述：相机制冷设置,只是一个模板
     * note:This function should not be executed normally
     */
    bool INDICCD::Cooling(bool SetPoint,bool CoolDown,bool ASync,bool Warmup,bool CoolerOFF,int CamTemp)
    {
        return false;
    }

    void INDICCD::InitINDICamera()
    {
        std::string line,jsonStr;
        std::ifstream in("config/indi/indiccd.json", std::ios::binary);
        if (!in.is_open())
        {
            IDLog_Error(_("Unable to open configuration file,we'll create one\n"));
            system("tocuh config/indi/indiccd.json");
        }
        else
        {
            while (getline(in, line))
            jsonStr.append(line);
            in.close();
            std::unique_ptr<Json::CharReader>const json_read(reader.newCharReader());
            json_read->parse(jsonStr.c_str(), jsonStr.c_str() + jsonStr.length(), &root,&errs);
            INDICAMERA->host = root["Host"].asString();
            INDICAMERA->port = root["Port"].asInt();
            INDICAMERA->type = root["Type"].asInt();
            if(INDICAMERA->type == 0)
            {
                INDICAMERA->BlobName = "CCD1";
                INDICAMERA->CCDCmd = "CCD_";
            }
            else
            {
                INDICAMERA->BlobName = "CCD2";
                INDICAMERA->CCDCmd = "GUIDER_";
            }
            INDICAMERA->Name[0] = const_cast<char *>(root["Name"].asString().c_str());
        }
    }

    void INDICCD::ClearStatus()
    {
        INDIS.connection_prop = nullptr;
        INDIS.expose_prop = nullptr;
        INDIS.frame_prop = nullptr;
        INDIS.frame_type_prop = nullptr;
        INDIS.ccdinfo_prop = nullptr;
        INDIS.binning_prop = nullptr;
        INDIS.video_prop = nullptr;
        INDIS.camera_port = nullptr;
        INDIS.camera_device = nullptr;
    }

    void INDICCD::newDevice(INDI::BaseDevice *dp)
    {
        if (strcmp(dp->getDeviceName(), const_cast<char *>(root["Name"].asString().c_str())) == 0)
        {
            INDIS.camera_device = dp;
        }
    }

    void INDICCD::newSwitch(ISwitchVectorProperty *svp)
    {
        if (strcmp(svp->name, "CONNECTION") == 0)
        {
            ISwitch *connectswitch = IUFindSwitch(svp, "CONNECT");
            if (connectswitch->s == ISS_ON)
            {
                INDICAMERA->isCameraConnected = true;
            }
            else
            {
                if (INDISS.ready)
                {
                    ClearStatus();
                }
            }
        }
    }

    void INDICCD::newMessage(INDI::BaseDevice *dp, int messageID)
    {
        IDLog(_("INDI Camera Received message: %s\n"), dp->messageQueue(messageID).c_str());
    }

    void INDICCD::newNumber(INumberVectorProperty *nvp)
    {
        if (strcmp(nvp->name, "CCD_EXPOSURE") == 0)
        {
            // rate limit this one, it's too noisy
            static double s_lastval;
            if (nvp->np->value > 0.0 && std::fabs(nvp->np->value - s_lastval) < 0.5)
                return;
            s_lastval = nvp->np->value;
        }
        std::ostringstream os;
        for (int i = 0; i < nvp->nnp; i++)
        {
            if (i)
                os << ',';
            os << nvp->np[i].name << ':' << nvp->np[i].value;
        }
        IDLog(_("INDI Camera Received Number: %s = %s state = %s\n"), nvp->name, os.str().c_str(), StateStr(nvp->s));

        if (nvp == INDIS.ccdinfo_prop)
        {
            INDICAMERA->PixSize = IUFindNumber(INDIS.ccdinfo_prop, "CCD_PIXEL_SIZE")->value;
            INDICAMERA->PixSizeX = IUFindNumber(INDIS.ccdinfo_prop, "CCD_PIXEL_SIZE_X")->value;
            INDICAMERA->PixSizeY = IUFindNumber(INDIS.ccdinfo_prop, "CCD_PIXEL_SIZE_Y")->value;
            INDICAMERA->ImageMaxWidth = IUFindNumber(INDIS.ccdinfo_prop, "CCD_MAX_X")->value;
            INDICAMERA->ImageMaxHeight = IUFindNumber(INDIS.ccdinfo_prop, "CCD_MAX_Y")->value;
            INDICAMERA->BitsPerPixel = IUFindNumber(INDIS.ccdinfo_prop, "CCD_BITSPERPIXEL")->value;
        }
        else if (nvp == INDIS.binning_prop)
        {
            INDICAMERA->Bin = std::min(INDIS.binning_x->value, INDIS.binning_y->value);
        }
    }

    void INDICCD::newText(ITextVectorProperty *tvp)
    {
        IDLog(_("INDI Camera Received Text: %s = %s\n"), tvp->name, tvp->tp->text);
    }

    void INDICCD::newBLOB(IBLOB *bp)
    {
        IDLog(_("INDI Camera Received BLOB %s len=%d size=%d\n"), bp->name, bp->bloblen, bp->size);
        if (INDIS.expose_prop && !INDICAMERA->InVideo)
        {
            if (bp->name == INDICAMERA->BlobName)
            {
                INDIS.cam_bp = bp;
                INDISS.modal = false;
            }
        }
        else if (INDIS.video_prop)
        {
            INDIS.cam_bp = bp;
        }
    }

    void INDICCD::newProperty(INDI::Property *property)
    {

    }

    void INDICCD::removeDevice(INDI::BaseDevice *dp)
    {

    }
}