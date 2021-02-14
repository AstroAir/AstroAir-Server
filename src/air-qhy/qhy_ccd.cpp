/*
 * qhy_ccd.cpp
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

Date:2021-2-15

Description:QHY camera driver

**************************************************/

#include "qhy_ccd.h"
#include "../logger.h"
#include "../opencv.h"

namespace AstroAir
{
	/*
     * name: QHYCCD()
     * describe: Initialization, for camera constructor
     * 描述：构造函数，用于初始化相机参数
     */
	QHYCCD::QHYCCD()
	{
		CamNumber = 0;
		CamId = 0;
		CamBin = 0;
		isConnected = false;
		InVideo = false;
		InExposure = false;
		InCooling = false;
	}
	
	/*
     * name: ~QHYCCD()
     * describe: Destructor
     * 描述：析构函数
     * calls: Disconnect()
     * note: Ensure camera safety
     */
	QHYCCD::~QHYCCD()
	{
		if (isConnected == true)
		{
			Disconnect();
		}
	}
	
	/*
     * name: Connect(std::string Device_name)
     * @param Device_name:连接相机名称
     * describe: Connect the camera
     * 描述： 连接相机
     * calls: InitQHYCCDResource()
     * calls: ScanQHYCCD()
     * calls: GetQHYCCDId()
     * calls: OpenQHYCCD()
     * calls: InitQHYCCD()
     * calls: UpdateCameraConfig()
     * calls: IDLog()
     * @return true: Camera connected successfully
     * @return false: Failed to connect camera
     * note: Do not connect the camera repeatedly
     */
	bool QHYCCD::Connect(std::string Device_name)
	{
		if((retVal = InitQHYCCDResource()) != QHYCCD_SUCCESS)
		{
			IDLog("Unable to initialize SDK settings,error code is %d please check system settings\n",retVal);
			return false;
		}
		else
		{
			IDLog("Init SDK successfully!\n");
			if((CamNumber = ScanQHYCCD()) <= 0)
			{
				IDLog("QHY camera not found, please check the power supply or make sure the camera is connected.\n");
				return false;
			}
			else
			{
				for(int i = 0;i < CamNumber;i++)
				{
					GetQHYCCDId(i, iCamId);
					if(GetQHYCCDModel(iCamId,CamName[i]) != QHYCCD_SUCCESS)
					{
						IDLog("Unable to get %s name, please check program permissions.\n",CamName[i]);
						return false;
					}
					else
					{
						if(CamName[i] == Device_name)
						{
							IDLog("Find %s.\n",CamName[i]);
							CamId = i;
							if((pCamHandle = OpenQHYCCD(iCamId)) == NULL)
							{
								IDLog("Unable to turn on the %s.\n",CamName[CamId]);
								return false;
							}
							else
							{
								if(InitQHYCCD(pCamHandle) != QHYCCD_SUCCESS)
								{
									IDLog("Unable to initialize connection to camera.\n");
									return false;
								}
								else
								{
									isConnected = true;
									IDLog("Camera turned on successfully\n");
									/*获取连接相机配置信息，并存入参数*/
									UpdateCameraConfig();
									return true;
								}
							}
						}
						else
						{
							IDLog("This is not a designated camera, try to find the next one.\n");
						}
					}
				}
				IDLog("The specified camera was not found. Please check the camera connection");
				return false;
			}
		}
		return false;
	}
	
	/*
     * name: Disconnect()
     * describe: Disconnect from camera
     * 描述：与相机断开连接
     * calls: StopQHYCCDLive()
     * calls: CancelQHYCCDExposingAndReadout()
     * calls: SaveConfig()
     * calls: CloseQHYCCD()
     * calls: IDLog()
     * note: Please stop all work before turning off the camera
     */
	bool QHYCCD::Disconnect()
	{
		/*在关闭相机之前停止所有任务*/
		if(InVideo == true)
		{
			if(StopQHYCCDLive(pCamHandle) != QHYCCD_SUCCESS)		//停止视频拍摄
			{
				IDLog("Unable to stop video capture, please try again.\n");
				return false;
			}
			IDLog("Stop video capture.\n");
		}
		if(InExposure == true)
		{
			if(CancelQHYCCDExposingAndReadout(pCamHandle) != QHYCCD_SUCCESS)		//停止曝光
			{
				IDLog("Unable to stop exposure, please try again.\n");
				return false;
			}
			IDLog("Stop exposure.\n");
		}
		/*在关闭相机之前保存设置*/
		//SaveConfig();
		/*关闭相机*/
		if(CloseQHYCCD(pCamHandle) != QHYCCD_SUCCESS)		//关闭相机
		{
			IDLog("Unable to turn off the camera, please try again");
			return false;
		}
		if ((retVal = ReleaseQHYCCDResource()) != QHYCCD_SUCCESS)
		{
			printf("Cannot release SDK resources, error %d.\n", retVal);
		}
		else
		{
			printf("SDK resources released.\n"); 
		}
		IDLog("Disconnect from camera\n");
		return true;
    }
    
    bool QHYCCD::UpdateCameraConfig()
	{
		return true;
	}
}
