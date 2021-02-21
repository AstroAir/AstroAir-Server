/*
 * qhy_ccd.h
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

Date:2021-1-3

Description:QHY camera driver

**************************************************/

#pragma once

#ifndef _QHYCCD_H_
#define _QHYCCD_H_

#include "../wsserver.h"
#include "../libqhy/qhyccd.h"

#include <atomic>

#define MAXDEVICENUM 5

namespace AstroAir
{
	class QHYCCD: public WSSERVER
	{
		public:
			/*构造函数，重置参数*/
			explicit QHYCCD();
			/*析构函数*/
			~QHYCCD();
			/*连接相机*/
			virtual bool Connect(std::string Device_name) override;
			/*断开连接*/
			virtual bool Disconnect() override;
			/*更新相机配置信息*/
			virtual bool UpdateCameraConfig();
			/*开始曝光*/
			virtual bool StartExposure(int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset) override;
			/*停止曝光*/
			virtual bool AbortExposure() override;
			/*设置相机参数*/
			virtual bool SetCameraConfig(double Bin,double Gain,double Offset);
			/*存储图像*/
			virtual bool SaveImage(std::string FitsName);
			/*相机制冷*/
			virtual bool Cooling(bool SetPoint,bool CoolDown,bool ASync,bool Warmup,bool CoolerOFF,int CamTemp) override;
		private:
			int CamNumber = 0;
			char *CamName[MAXDEVICENUM];
			int CamBin;
			char iCamId[32] = {0};
			char CamId[32] = {0};
			unsigned int retVal;

			std::mutex condMutex;
			std::mutex ccdBufferLock;

			/*相机配置参数*/
			double chipWidth;
			double chipHeight;
			double pixelWidth;
			double pixelHeight;

			unsigned int iMaxWidth = 0;		//最大高度
			unsigned int iMaxHeight = 0;		//最大宽度
			unsigned int CamWidth = 0;
			unsigned int CamHeight = 0;
			unsigned int channels = 1; 		//通道，默认为黑白相机
			unsigned int Image_type = 0;

			bool isCoolCamera = false;
			bool isColorCamera = false;
			bool isGuideCamera = false;
			
			qhyccd_handle *pCamHandle;
			
			/*相机使用参数*/
			std::atomic_bool isConnected;
			std::atomic_bool InExposure;
			std::atomic_bool InVideo;
			std::atomic_bool InCooling;
	};
}

#endif
