/*
 * indiccd.h
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

#ifndef _INDI_CCD_H_
#define _INDI_CCD_H_

#include "../air_camera.h"

#include <json/json.h>
#include <libindi/baseclient.h>
#include <libindi/basedevice.h>
#include <libindi/indiproperty.h>

namespace AstroAir
{
    class INDICCD:public AIRCAMERA , public INDI::BaseClient
    {
        public:
            explicit INDICCD(CameraInfo *NEW);
            ~INDICCD();
            /*连接相机*/
            virtual bool Connect(std::string Device_name)override;
            /*断开连接*/
			virtual bool Disconnect() override;
			/*返回相机名称*/
			virtual std::string ReturnDeviceName() override;
            /*开始曝光*/
			virtual bool StartExposure(int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset) override;
			/*停止曝光*/
			virtual bool AbortExposure() override;
            /*制冷*/
			virtual bool Cooling(bool SetPoint,bool CoolDown,bool ASync,bool Warmup,bool CoolerOFF,int CamTemp) override;
        
        protected:
            virtual void InitINDICamera();
            virtual void ClearStatus();

            inline static const char *StateStr(IPState st)
            {
                switch (st) {
                default: case IPS_IDLE: return "Idle";
                case IPS_OK: return "Ok";
                case IPS_BUSY: return "Busy";
                case IPS_ALERT: return "Alert";
                }
            }

            void newDevice(INDI::BaseDevice *dp) override;
            #ifndef INDI_PRE_1_0_0
            void removeDevice(INDI::BaseDevice *dp) override;
            #endif
            void newProperty(INDI::Property *property) override;
            void removeProperty(INDI::Property *property) override {}
            void newBLOB(IBLOB *bp) override;
            void newSwitch(ISwitchVectorProperty *svp) override;
            void newNumber(INumberVectorProperty *nvp) override;
            void newMessage(INDI::BaseDevice *dp, int messageID) override;
            void newText(ITextVectorProperty *tvp) override;
            void newLight(ILightVectorProperty *lvp) override {}
            void serverConnected() override {}

        private:
            CameraInfo *INDICAMERA;
            Json::Value root;
			Json::String errs;
			Json::CharReaderBuilder reader;

            struct INDIStatus
            {
                bool ready;
                volatile bool modal;
                volatile bool stacking;
            }INDISS;
            

            struct INDISetting
            {
                IBLOB    *cam_bp;
                ISwitchVectorProperty *connection_prop = nullptr;
                INumberVectorProperty *expose_prop = nullptr;
                INumberVectorProperty *frame_prop = nullptr;
                INumber               *frame_x = nullptr;
                INumber               *frame_y = nullptr;
                INumber               *frame_width = nullptr;
                INumber               *frame_height = nullptr;
                ISwitchVectorProperty *frame_type_prop = nullptr;
                INumberVectorProperty *ccdinfo_prop = nullptr;
                INumberVectorProperty *binning_prop = nullptr;
                INumber               *binning_x = nullptr;
                INumber               *binning_y = nullptr;
                ISwitchVectorProperty *video_prop = nullptr;
                ITextVectorProperty   *camera_port = nullptr;
                INDI::BaseDevice      *camera_device = nullptr;
            }INDIS;

            
            
    };
}

#endif