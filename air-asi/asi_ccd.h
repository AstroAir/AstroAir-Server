/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

/************************************************* 
 
Copyright: 2020 Max Qian. All rights reserved
 
Author:Max Qian

E-mail:astro_air@126.com
 
Date:2020-12-11
 
Description:ZWO camera driver
 
**************************************************/

#pragma once

#ifndef _ASICCD_H_
#define _ASICCD_H_

#include "../libasi/ASICamera2.h"

#include <condition_variable>
#include <mutex>
#include <thread>

#define MAXDEVICENUM 5

namespace AstroAir
{
	class ASICCD
	{
		public:
			/*构造函数，重置参数*/
			explicit ASICCD();
			/*析构函数*/
			~ASICCD();
			/*连接相机*/
			virtual bool Connect(std::string Device_name);
			/*断开连接*/
			virtual bool Disconnect();
			/*更新相机配置信息*/
			virtual bool UpdateCameraConfig();
			/*设置相机制冷温度*/
			virtual bool SetTemperature(double temperature);
			/*开始曝光*/
			virtual bool StartExposure(float duration);
			/*停止曝光*/
			virtual bool AbortExposure();
		protected:
			/*设置相机画幅大小*/
			virtual bool UpdateCCDFrame(int x, int y, int w, int h);
			/*添加Fits头部信息*/
			//virtual void addFITSKeywords(fitsfile *fptr);
		private:
			/*打开制冷*/
			virtual bool ActiveCool(bool enable);
			
			typedef enum ImageState
			{
				StateNone = 0,
				StateIdle,
				StateStream,
				StateExposure,
				StateRestartExposure,
				StateAbort,
				StateTerminate,
				StateTerminated
			} ImageState;
			
			//void setThreadRequest(ImageState request);
			//void waitUntil(ImageState request);
			
			ImageState threadRequest;
			ImageState threadState;
			
			std::thread imagingThread;
			std::mutex condMutex;
			std::condition_variable cv;
			
			int CamNumber;
			int CamId;
			char *CamName[MAXDEVICENUM];
			
			double ExposureRequest;
			double TemperatureRequest;
			/*相机配置参数*/
			int iMaxWidth = 0;		//最大高度
			int iMaxHeight = 0;		//最大宽度
			bool isCoolCamera = false;
			bool isColorCamera = false;
			bool isGuideCamera = false;
			
			/*相机使用参数*/
			bool isConnected = false;
			bool InExposure = false;
			bool InVideo = false;
			bool InCooling = false;
			
			/*ASI相机参数*/
			ASI_CAMERA_INFO ASICameraInfo;
	};
}

#endif



