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
			/*存储图像*/
			virtual std::string SaveImage(std::string FitsName) override;
		private:
			int CamNumber = 0;
			char *CamName[MAXDEVICENUM];
			int CamId;
			int CamBin;
			char iCamId[32] = {0};
			unsigned int retVal;

			/*相机配置参数*/
			int iMaxWidth = 0;		//最大高度
			int iMaxHeight = 0;		//最大宽度
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
