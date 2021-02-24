/*
 * asi_ccd.cpp
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

Date:2020-2-14

Description:ZWO camera driver

**************************************************/

#include "asi_ccd.h"
#include "../logger.h"
#include "../opencv.h"

#include <fitsio.h>

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
		CamBin = 0;
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
				if((errCode = ASIGetCameraProperty(&ASICameraInfo, i)) != ASI_SUCCESS)
				{
					IDLog("Unable to get %s configuration information,the error code is %d,please check program permissions.\n",ASICameraInfo.Name,errCode);
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
						if((errCode = ASIOpenCamera(CamId)) != ASI_SUCCESS)		
						{
							IDLog("Unable to turn on the %s,error code is %d.\n",CamName[CamId],errCode);
							return false;
						}
						else
						{
							/*初始化相机*/
							if((errCode = ASIInitCamera(CamId)) != ASI_SUCCESS)	
							{
								IDLog("Unable to initialize connection to camera,the error code is %d.\n",errCode);
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
			if((errCode = ASIStopVideoCapture(CamId)) != ASI_SUCCESS)		//停止视频拍摄
			{
				IDLog("Unable to stop video capture,error code is %d,please try again.\n",errCode);
				return false;
			}
			IDLog("Stop video capture.\n");
		}
		if(InExposure == true)
		{
			if((errCode = ASIStopExposure(CamId)) != ASI_SUCCESS)		//停止曝光
			{
				IDLog("Unable to stop exposure,error code is %d,please try again.\n",errCode);
				return false;
			}
			IDLog("Stop exposure.\n");
		}
		/*在关闭相机之前保存设置*/
		//SaveConfig();
		/*关闭相机*/
		if((errCode = ASICloseCamera(CamId)) != ASI_SUCCESS)		//关闭相机
		{
			IDLog("Unable to turn off the camera,error code is %d,please try again\n",errCode);
			return false;
		}
		IDLog("Disconnect from camera\n");
		return true;
    }
    
	std::string ASICCD::ReturnDeviceName()
	{
		return CamName[CamId];
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
		/*获取相机格式*/
		Image_type = ASICameraInfo.SupportedVideoFormat[7];
		/*获取相机最大画幅*/
		iMaxWidth = ASICameraInfo.MaxWidth;
		iMaxHeight = ASICameraInfo.MaxHeight;
		CamWidth = iMaxWidth;
		CamHeight = iMaxHeight;
		IDLog("Camera information obtained successfully.\n");
		return true;
    }
    
	/*
     * name: Cooling(bool SetPoint,bool CoolDown,bool ASync,bool Warmup,bool CoolerOFF,int CamTemp)
     * describe: Start or stop cooling
     * 描述：开始或停止制冷
     * @return ture: refrigeration successful
     */
	bool ASICCD::Cooling(bool SetPoint,bool CoolDown,bool ASync,bool Warmup,bool CoolerOFF,int CamTemp)
	{
		bool camera_ok = false;
		if(isCoolCamera == true)
		{
			if((camera_ok = SetTemperature(CamTemp)) != true)
				return false;
			if(CoolerOFF == true)
				ActiveCool(false);
			else
				ActiveCool(true);
		}
		else
		{
			IDLog("This is not a cooling camera. The cooling mode cannot be turned on. Please choose another camera\n");
			return false;
		}
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
		if((errCode = ASISetControlValue(CamId,ASI_TEMPERATURE,TargetTemp,ASI_FALSE)) != ASI_SUCCESS)
		{
			IDLog("Unable to set camera temperature,error code is %d.\n",errCode);
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
			if((errCode = ASISetControlValue(CamId,ASI_COOLER_ON,enable ? ASI_TRUE : ASI_FALSE,ASI_FALSE)) != ASI_SUCCESS)
			{
				IDLog("Unable to turn on refrigeration,error code is %d,please check the power supply.\n",errCode);
				return false;
			}
			InCooling = true;
			IDLog("Cooling is in progress. Please wait.\n");
		}
		return true;
    }
    
    /*
     * name: StartExposure(int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset)
     * describe: Start camera exposure
     * 描述：相机开始曝光
     * @param exp: 曝光时间
     * @param bin:像素合并模式
     * @param IsSave:是否保存图像
     * @param FitsName:图像名称
     * @param Gain:相机增益
     * @param Offset:相机偏置
     * calls: ASISetControlValue()
	 * calls: SetCameraConfig()
     * calls: IDLog()
     * calls: ASIStartExposure()
     * calls: ASIGetExpStatus()
     */
    bool ASICCD::StartExposure(int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset)
    {
		std::unique_lock<std::mutex> guard(condMutex);
		const long blink_duration = exp * 1000000;
		CamBin = bin;
		IDLog("Blinking %ld time(s) before exposure\n", blink_duration);
		if((errCode = ASISetControlValue(CamId, ASI_EXPOSURE, blink_duration, ASI_FALSE)) != ASI_SUCCESS)
		{
			IDLog("Failed to set blink exposure to %ldus, error %d\n", blink_duration, errCode);
			return false;
		}
		else
		{
			if(SetCameraConfig(bin,Gain,Offset) != true)
			{
				IDLog("Failed to set camera configure\n");
				return false;
			}
			else
			{
				if((errCode = ASIStartExposure(CamId, ASI_FALSE)) != ASI_SUCCESS)
				{
					IDLog("Failed to start blink exposure, error %d,try it again\n", errCode);
					AbortExposure();
					return false;
				}
				else
				{
					InExposure = true;
					do
					{
						usleep(10000);
						errCode = ASIGetExpStatus(CamId, &expStatus);
						
					}
					while (errCode == ASI_SUCCESS && expStatus == ASI_EXP_WORKING);
					if (errCode != ASI_SUCCESS)
					{
						IDLog("Blink exposure failed, error %d, status %d\n", errCode, expStatus);
						AbortExposure();
						return false;
					}
					InExposure = false;
                }
            }
        }
		guard.unlock();
        if(IsSave == true)
        {
			IDLog("Finished exposure and save image locally\n");
			if(SaveImage(FitsName) != true)
			{
				IDLog("Could not save image correctly,please check the config\n");
				return false;
			}
			else
			{
				IDLog("Saved Fits and JPG images %s successfully in locally\n",FitsName.c_str());
			}
		}
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
		if((errCode = ASIStopExposure(CamId)) != ASI_SUCCESS)
		{
			IDLog("Unable to stop camera exposure,error id is %d,please try again.\n",errCode);
			return false;
		}
		InExposure = false;
		return true;
    }
    
    /*
     * name: SetCameraConfig(long Bin,long Gain,long Offset)
     * describe: set camera cinfig
     * 描述：设置相机参数
     * calls: IDLog()
     * calls: ASISetControlValue()
     * calls: ASISetROIFormat()
     */
    bool ASICCD::SetCameraConfig(long Bin,long Gain,long Offset)
    {
		if((errCode = ASISetControlValue(CamId, ASI_GAIN, Gain, ASI_FALSE)) != ASI_SUCCESS)
		{
			IDLog("Unable to set camera gain,error code is %d\n",errCode);
			return false;
		}
		if((errCode = ASISetControlValue(CamId, ASI_BRIGHTNESS, Offset, ASI_FALSE)) != ASI_SUCCESS)
		{
			IDLog("Unable to set camera offset,error code is %d\n",errCode);
			return false;
		}
		CamWidth = iMaxWidth/Bin;
		CamHeight = iMaxHeight/Bin;
		if((errCode = ASISetROIFormat(CamId, CamWidth , CamHeight , Bin, (ASI_IMG_TYPE)Image_type)) != ASI_SUCCESS)
		{
			IDLog("Unable to set camera offset,error code is %d\n",errCode);
			return false;
		}
		return true;
    }

    /*
     * name: SaveImage(std::string FitsName)
     * describe: Save images
     * 描述：存储图像
     * calls: ASIGetDataAfterExp()
     * calls: fits_create_file()
     * calls: fits_create_img()
     * calls: fits_update_key()
     * calls: fits_write_img()
     * calls: fits_close_file()
     * calls: fits_report_error()
	 * calls: SaveImage()
	 * calls: SaveFitsImage()
	 * calls: clacHistogram()
     */
    bool ASICCD::SaveImage(std::string FitsName)
    {
		if(InExposure == false && InVideo == false)
		{	
			std::unique_lock<std::mutex> guard(ccdBufferLock);
			long imgSize = CamWidth*CamHeight*(1 + (Image_type==ASI_IMG_RAW16));		//设置图像大小
			unsigned char * imgBuf = new unsigned char[imgSize];		//图像缓冲区大小
			long naxis = 2;
			/*曝光后获取图像信息*/
			if ((errCode = ASIGetDataAfterExp(CamId, imgBuf, imgSize)) != ASI_SUCCESS)
			{
				/*获取图像失败*/
				IDLog("ASIGetDataAfterExp error (%d)\n",errCode);
				return false;
			}
			guard.unlock();
			IDLog("Download complete.\n");
			/*将图像写入本地文件*/
			#ifdef HAS_FITSIO
				char datatype[40];		//相机品牌
				char keywords[40];		//相机品牌
				char value[20];		//相机名称
				char description[40];		//相机描述
				strcpy(datatype, "TSTRING");
				strcpy(keywords, "Camera");
				strcpy(value,CamName[CamId]);
				strcpy(description,"ZWOASI");

				fitsfile *fptr;		//cFitsIO定义
				int FitsStatus;		//cFitsio状态
				long naxes[2] = {CamWidth,CamHeight};
				long nelements;
				long fpixel = 1;

				fits_create_file(&fptr, FitsName.c_str(), &FitsStatus);		//创建Fits文件
				if(Image_type == 1)		//创建Fits图像
					fits_create_img(fptr, USHORT_IMG, naxis, naxes, &FitsStatus);		//16位
				else
					fits_create_img(fptr, BYTE_IMG,   naxis, naxes, &FitsStatus);		//8位或12位
				if(strcmp(datatype, "TSTRING") == 0)		//写入Fits图像头文件
				{
					fits_update_key(fptr, TSTRING, keywords, value, description, &FitsStatus);
				}
				if(Image_type == 1)		//将缓存图像写入SD卡
					fits_write_img(fptr, TUSHORT, fpixel, imgSize, &imgBuf[0], &FitsStatus);		//16位
				else
					fits_write_img(fptr, TBYTE, fpixel, imgSize, &imgBuf[0], &FitsStatus);		//8位或12位
				fits_close_file(fptr, &FitsStatus);		//关闭Fits图像
				fits_report_error(stderr, FitsStatus);		//如果有错则返回错误信息
			#endif
			#ifdef HAS_OPENCV
				OPENCV::SaveImage(imgBuf,FitsName,isColorCamera,CamHeight,CamWidth);
				OPENCV::clacHistogram(imgBuf,isColorCamera,CamHeight,CamWidth);
			#endif
			if(imgBuf)
				delete[] imgBuf;		//删除图像缓存
		}
		return true;
	}
}




