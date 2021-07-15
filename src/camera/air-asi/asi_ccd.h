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

#ifndef _ASICCD_H_
#define _ASICCD_H_

#include "../../air_camera.h"
#include <libasi/ASICamera2.h>

namespace AstroAir
{
	class ASICCD: public AIRCAMERA
	{
		public:
			/*构造函数，重置参数*/
			explicit ASICCD(CameraInfo *NEW);
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
			/**/
			virtual void CameraGUI(bool* p_open)override;
		protected:
			/*打开或停止制冷*/
			virtual bool ActiveCool(bool enable);
			/*保存相机设置*/
			virtual bool SaveCameraConfig();
		private:
			CameraInfo *ASICAMERA;
			
			/*ASI相机参数*/
			ASI_CAMERA_INFO ASICameraInfo;
			ASI_ERROR_CODE errCode;
			ASI_EXPOSURE_STATUS expStatus;
	};
}

#endif



