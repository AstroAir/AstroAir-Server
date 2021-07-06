/*
 * gphoto2_ccd.cpp
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

Date:2021-2-21

Description:DSLR driver

**************************************************/

#include "gphoto2_ccd.h"
#include "../../logger.h"

static GPContext * context = gp_context_new();

namespace AstroAir
{
    /*
     * name: GPhotoCCD()
     * describe: Initialization, for camera constructor
     * 描述：构造函数，用于初始化相机参数
     */
    GPhotoCCD::GPhotoCCD(CameraInfo *NEW)
    {
		GPHOTOINFO = NEW;
		GPHOTOINFO->Exposure = 0;
		GPHOTOINFO->ExposureUsed = 0;
		GPHOTOINFO->Gain = 0;
		GPHOTOINFO->Offset = 0;
		GPHOTOINFO->Temperature = 0;
		GPHOTOINFO->ID = 0;
		GPHOTOINFO->Image_Height = 0;
		GPHOTOINFO->Image_Width = 0;
		GPHOTOINFO->ImageMaxHeight = 0;
		GPHOTOINFO->ImageMaxWidth = 0;
		GPHOTOINFO->InExposure = false;
		GPHOTOINFO->isCameraConnected = false;
		GPHOTOINFO->isCameraCoolingOn = false;
		GPHOTOINFO->isColorCamera = false;
		GPHOTOINFO->isCoolCamera = false;
		GPHOTOINFO->isGuidingCamera = false;

        context = gp_context_new();
    }

    /*
     * name: ~ASICCD()
     * describe: Destructor
     * 描述：析构函数
     * calls: Disconnect()
     * note: Ensure camera safety
     */
    GPhotoCCD::~GPhotoCCD()
    {
		if (GPHOTOINFO->isCameraConnected)
			Disconnect();
    }

    bool GPhotoCCD::Connect(std::string Device_name)
    {
        CameraList * list;
        /*选择所有可以被自动选中的相机*/
        if (gp_list_new(&list) < GP_OK)
        {
            IDLog_Error(_("Unable to initialize camera connection, please check device connection\n"));
            return false;
        }
        else
        {
            gp_list_reset(list);
            GPHOTOINFO->Count = gp_camera_autodetect(list, context);
            if(GPHOTOINFO->Count <= 0)
            {
                IDLog_Error(_("Gphoto2 camera not found, please check the power supply or make sure the camera is connected.\n"));
                return false;
            }
            else
            {
                const char * model, *port;
                for(int i = 0;i < GPHOTOINFO->Count ;i++)
                {
                    gp_list_get_name(list, i, &model);
                    gp_list_get_value(list, i, &port);
                    if(Device_name.c_str() == model)
                    {
                        IDLog(_("Find %s on port %s\n"), model, port);
                        GPHOTOINFO->ID = i;
                        GPHOTOINFO->Name[GPHOTOINFO->ID] = const_cast<char *>(model);
                        gp_camera_new(&camera);
                        if(gp_camera_init(camera, context) != GP_OK)
                        {
                            IDLog_Error(_("Unable to initialize connection to camera\n"));
                            return false;
                        }
                        else
                        {
                            GPHOTOINFO->isCameraConnected = true;
                            UpdateCameraConfig();
                            return true;
                        }
                    }
                }
                IDLog("The specified camera was not found. Please check the camera connection");
                return false;
            }
        }
		return false;
    }

    bool GPhotoCCD::Disconnect()
    {
        return true;
    }

    std::string GPhotoCCD::ReturnDeviceName()
    {
        return "test";
    }
    
    bool GPhotoCCD::UpdateCameraConfig()
    {
        return true;
    }

    bool GPhotoCCD::SetTemperature(double temperature)
    {
        return true;
    }

    bool GPhotoCCD::StartExposure(int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset)
    {
        return true;
    }

    bool GPhotoCCD::AbortExposure()
    {
        return true;
    }

    bool GPhotoCCD::SetCameraConfig(long Bin,long Gain,long Offset)
    {
        return true;
    }

    bool GPhotoCCD::SaveImage(std::string FitsName)
    {
        return true;
    }

    bool GPhotoCCD::Cooling(bool SetPoint,bool CoolDown,bool ASync,bool Warmup,bool CoolerOFF,int CamTemp)
    {
        return true;
    }
}
