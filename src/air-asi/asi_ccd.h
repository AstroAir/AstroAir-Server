/*
 * asi_ccd.h
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
 
Date:2021-1-4
 
Description:ZWO camera driver
 
**************************************************/

#pragma once

#ifndef _ASICCD_H_
#define _ASICCD_H_

#include "../wsserver.h"
#include "../air_camera.h"
#include "../libasi/ASICamera2.h"

#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include <string.h>

#define MAXDEVICENUM 5

namespace AstroAir
{

	class ASICCD: public AIRCAMERA
	{
		public:
			/*构造函数，重置参数*/
			explicit ASICCD();
			/*析构函数*/
			virtual ~ASICCD();
			/*连接相机*/
			virtual bool Connect(std::string Device_name) override;
			/*断开连接*/
			virtual bool Disconnect() override;
			/*返回相机名称*/
			virtual std::string ReturnDeviceName() override;
			/*更新相机配置信息*/
			virtual bool UpdateCameraConfig();
			/*设置相机制冷温度*/
			virtual bool SetTemperature(double temperature);
			/*开始曝光*/
			virtual bool StartExposure(int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset) override;
			/*停止曝光*/
			virtual bool AbortExposure() override;
			/*设置相机参数*/
			virtual bool SetCameraConfig(long Bin,long Gain,long Offset);
			/*存储图像*/
			virtual bool SaveImage(std::string FitsName);
			/*制冷*/
			virtual bool Cooling(bool SetPoint,bool CoolDown,bool ASync,bool Warmup,bool CoolerOFF,int CamTemp) override;
		private:
			/*打开制冷*/
			virtual bool ActiveCool(bool enable);
			/*保存相机设置*/
			virtual bool SaveCameraConfig();
			
			std::mutex condMutex;
			std::mutex ccdBufferLock;
			/*基础参数*/
			int CamNumber;
			int CamId;
			char *CamName[MAXDEVICENUM];
			int CamBin;
			
			double ExposureRequest;
			double TemperatureRequest;
			/*相机配置参数*/
			int Image_type = 0;
			int CamWidth = 0;
			int CamHeight = 0;
			int iMaxWidth = 0;		//最大高度
			int iMaxHeight = 0;		//最大宽度
			bool isCoolCamera = false;
			bool isColorCamera = false;
			bool isGuideCamera = false;
			/*FitsIO*/
			//fitsfile *fptr;		//cFitsIO定义
			long nelements;
			long fpixel = 1;
			char datatype[40];		//数据格式
			char keywords[40];		//相机品牌
			char value[20];		//相机名称
			char description[40];		//相机描述
			/*相机使用参数（使用原子变量）*/
			std::atomic_bool isConnected;
			std::atomic_bool InExposure;
			std::atomic_bool InVideo;
			std::atomic_bool InCooling;
			
			/*ASI相机参数*/
			ASI_CAMERA_INFO ASICameraInfo;
			ASI_ERROR_CODE errCode;
			ASI_EXPOSURE_STATUS expStatus;
	};
	
}

#endif



