/*
 * air_camera.h
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
 
Date:2021-6-23
 
Description:Camera Offical Port

**************************************************/

#ifndef _AIR_CAMERA_H_
#define _AIR_CAMERA_H_

#include <string>
#include <atomic>

#include "tools/ImgTools.h"
#include "tools/ImgFitsIO.h"

namespace AstroAir
{
    class AIRCAMERA
    {
        public:
            explicit AIRCAMERA();
            ~AIRCAMERA();
            virtual bool Connect(std::string Device_name);      //连接相机
			virtual bool Disconnect();                          //断开连接
			virtual std::string ReturnDeviceName();             //返回设备名称
            virtual bool StartExposure(int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset);
            virtual bool StartExposureServer(int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset);
            virtual bool StartExposureSeq(int loop,int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset);
            virtual void ImagineThread();
            virtual bool AbortExposure();
            virtual bool Cooling(bool SetPoint,bool CoolDown,bool ASync,bool Warmup,bool CoolerOFF,int CamTemp);
            virtual bool CoolingServer(bool SetPoint,bool CoolDown,bool ASync,bool Warmup,bool CoolerOFF,int CamTemp);
            virtual void StartExposureSuccess();
            virtual void AbortExposureSuccess();
            virtual void StartExposureError();
            virtual void AbortExposureError();
            virtual void ShotRunningSend(int ElapsedPerc,int id);
            virtual void newJPGReadySend();
        public:
            int CameraBin = 0;
			int CameraExpo = 0;
			int CameraExpoUsed = 0;
			int CameraTemp = 0;
			
        private:
            std::string json_message,Image_Name;
            std::atomic_bool InExposure;
			std::atomic_bool InSequenceRun;
    };
    extern AIRCAMERA *CCD;
    extern std::atomic_bool isCameraConnected;
    extern std::atomic_bool isCameraCoolingOn;
    
    extern std::string CameraImageName,img_data;
    extern int Image_Height,Image_Width,StarIndex;
    extern double HFD;
}

#endif