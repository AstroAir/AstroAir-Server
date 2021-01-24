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

Copyright: 2020-2021 Max Qian. All rights reserved

Author:Max Qian

E-mail:astro_air@126.com

Date:2020-12-11

Description:ZWO camera driver

**************************************************/

#include "asi_ccd.h"
#include "../logger.h"

namespace AstroAir
{
    /*
     * name: ASICCD()
     * describe: Initialization, for camera constructor
     * 描述：构造函数，用于初始化相机参数
     */
    ASICCD::ASICCD()
    {
		CamNumber = 0;
		CamId = 0;
		isConnected = false;
		InVideo = false;
		InExposure = false;
		InCooling = false;
    }
    
    /*
     * name: ~ASICCD()
     * describe: Destructor
     * 描述：析构函数
     * calls: Disconnect()
     * note: Ensure camera safety
     */
    ASICCD::~ASICCD()
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
     * calls: ASIGetNumOfConnectedCameras()
     * calls: ASIGetCameraProperty()
     * calls: ASIOpenCamera()
     * calls: ASIInitCamera()
     * calls: UpdateCameraConfig()
     * calls: IDLog()
     * @return true: Camera connected successfully
     * @return false: Failed to connect camera
     * note: Do not connect the camera repeatedly
     */
    bool ASICCD::Connect(std::string Device_name)
    {
		/*获取已连接相机数量*/
		CamNumber = ASIGetNumOfConnectedCameras();
		if(CamNumber <= 0)
		{
			IDLog("ASI camera not found, please check the power supply or make sure the camera is connected.\n");
			return false;
		}
		else
		{
			for(int i = 0;i < CamNumber;i++)
			{
			/*获取相机信息*/
			if(ASIGetCameraProperty(&ASICameraInfo, i) != ASI_SUCCESS)
			{
				IDLog("Unable to get %s configuration information, please check program permissions.\n",ASICameraInfo.Name);
				return false;
			}
			else
			{
				if(ASICameraInfo.Name == Device_name)
				{
				IDLog("Find %s.\n",ASICameraInfo.Name);
				CamId = ASICameraInfo.CameraID;
				CamName[CamId] = ASICameraInfo.Name;
				/*打开相机*/
				if(ASIOpenCamera(CamId) != ASI_SUCCESS)		
				{
					IDLog("Unable to turn on the %s.\n",CamName[CamId]);
					return false;
				}
				else
				{
					/*初始化相机*/
					if(ASIInitCamera(CamId) != ASI_SUCCESS)	
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
		return false;
    }
    
    /*
     * name: Disconnect()
     * describe: Disconnect from camera
     * 描述：与相机断开连接
     * calls: ASIStopVideoCapture()
     * calls: ASIStopExposure()
     * calls: SaveConfig()
     * calls: ASICloseCamera()
     * calls: IDLog()
     * note: Please stop all work before turning off the camera
     */
    bool ASICCD::Disconnect()
    {
		/*在关闭相机之前停止所有任务*/
		if(InVideo == true)
		{
			if(ASIStopVideoCapture(CamId) != ASI_SUCCESS);		//停止视频拍摄
			{
			IDLog("Unable to stop video capture, please try again.\n");
			return false;
			}
			IDLog("Stop video capture.\n");
		}
		if(InExposure == true)
		{
			if(ASIStopExposure(CamId) != ASI_SUCCESS)		//停止曝光
			{
			IDLog("Unable to stop exposure, please try again.\n");
			return false;
			}
			IDLog("Stop exposure.\n");
		}
		/*在关闭相机之前保存设置*/
		//SaveConfig();
		/*关闭相机*/
		if(ASICloseCamera(CamId) != ASI_SUCCESS)		//关闭相机
		{
			IDLog("Unable to turn off the camera, please try again");
			return false;
		}
		IDLog("Disconnect from camera\n");
		return true;
    }
    
    /*
     * name: UpdateCameraConfig()
     * describe: Get the required parameters of the camera
     * 描述：获取相机所需参数
     * @return ture: meaningless
     * note: These parameters are very important and related to the following program
     */
    bool ASICCD::UpdateCameraConfig()
    {
		/*判断是否为彩色相机*/
		if(ASICameraInfo.IsColorCam == true)
			isColorCamera = true;
		/*判断是否为制冷相机*/
		if(ASICameraInfo.IsCoolerCam == true)
			isCoolCamera = true;
		/*判断是否为导星相机*/
		if(ASICameraInfo.ST4Port == true)
			isGuideCamera = true;
		/*获取相机最大画幅*/
		iMaxWidth = ASICameraInfo.MaxWidth;
		iMaxHeight = ASICameraInfo.MaxHeight;
		IDLog("Camera information obtained successfully.\n");
		return true;
    }
    
    /*
     * name: SetTemperature(double temperature)
     * describe: Set camera cooling temperature
     * 描述：设置相机制冷温度
     * @param temperature: 目标温度
     * @return ture: 成功设置温度
     * @return false：无法设置温度
     * calls: ActiveCool()
     * calls: ASISetControlValue()
     * calls: IDLog()
     * note: The temperature of the camera is unlimited. Please pay attention to the limit
     */
    bool ASICCD::SetTemperature(double temperature)
    {
		/*判断输入温度是否合理*/
		if(temperature < -50 ||temperature > 40)
		{
			IDLog("The temperature setting is unreasonable, please reset it.\n");
			return false;
		}
		/*检查是否可以制冷*/
		if(ActiveCool(true) == false)
		{
			IDLog("Unable to start camera cooling, please check the power connection.\n");
			return false;
		}
		/*转化温度参数*/
		long TargetTemp;
		if (temperature > 0.5)
			TargetTemp = static_cast<long>(temperature + 0.49);
		else if (temperature  < 0.5)
			TargetTemp = static_cast<long>(temperature - 0.49);
		else
			TargetTemp = 0;
		/*设置相机温度*/
		if(ASISetControlValue(CamId,ASI_TEMPERATURE,TargetTemp,ASI_FALSE) != ASI_SUCCESS)
		{
			IDLog("Unable to set camera temperature.\n");
			return false;
		}
		TemperatureRequest = temperature;
		IDLog("Camera cooling temperature set successfully.\n");
		return true;
    }
    
    /*
     * name: ActiveCool(bool enable)
     * describe: Start camera cooling
     * 描述：启动相机制冷
     * @param enable: 制冷状态
     * @return ture: 成功开启制冷
     * @return false：无法开启制冷
     * calls: ASISetControlValue()
     * calls: IDLog()
     */
    bool ASICCD::ActiveCool(bool enable)
    {
		if(isCoolCamera == true)
		{
			if(ASISetControlValue(CamId,ASI_COOLER_ON,enable ? ASI_TRUE : ASI_FALSE,ASI_FALSE) != ASI_SUCCESS)
			{
			IDLog("Unable to turn on refrigeration, please check the power supply.\n");
			return false;
			}
			InCooling = true;
			IDLog("Cooling is in progress. Please wait.\n");
		}
		return true;
    }
    
    bool ASICCD::StartExposure(float exp,int bin,bool is_roi,int roi_type,int roi_x,int roi_y,bool is_save,std::string fitsname,int gain,int offset)
    {
	
		return true;
    }
    
    /*
     * name: AbortExposure()
     * describe: Stop camera exposure
     * 描述：停止相机曝光
     * @return ture: 成功停止曝光
     * @return false：无法停止曝光
     * calls: setThreadRequest()
     * calls: waitUntil()
     * calls: ASIStopExposure()
     * calls: IDLog()
     */
    bool ASICCD::AbortExposure()
    {
		IDLog("Aborting camera exposure...");
		//setThreadRequest(StateAbort);
		//waitUntil(StateIdle);
		if(ASIStopExposure(CamId) != ASI_SUCCESS)
		{
			IDLog("Unable to stop camera exposure,please try again.\n");
			return false;
		}
		InExposure = false;
		return true;
    }
    
    bool ASICCD::UpdateCCDFrame(int x, int y, int w, int h)
    {
		return true;
    }
    
    bool ASICCD::SetCameraConfig()
    {
		return true;
    }
}




