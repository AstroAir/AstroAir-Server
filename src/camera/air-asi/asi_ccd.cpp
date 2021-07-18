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

#include "../../logger.h"
#include "../../config.h"

#include <unistd.h>
#include <json/json.h>
#include <fstream>

namespace AstroAir
{
    /*
     * name: ASICCD()
     * describe: Initialization, for camera constructor
     * 描述：构造函数，用于初始化相机参数
     */
    ASICCD::ASICCD(CameraInfo *NEW)
    {
		ASICAMERA = NEW;
		ASICAMERA->Brand = "ZWOASI";
		ASICAMERA->Exposure = 0;
		ASICAMERA->ExposureUsed = 0;
		ASICAMERA->Gain = 0;
		ASICAMERA->Offset = 0;
		ASICAMERA->Temperature = 0;
		ASICAMERA->ID = 0;
		ASICAMERA->Image_Height = 0;
		ASICAMERA->Image_Width = 0;
		ASICAMERA->ImageMaxHeight = 0;
		ASICAMERA->ImageMaxWidth = 0;
		ASICAMERA->InExposure = false;
		ASICAMERA->isCameraConnected = false;
		ASICAMERA->isCameraCoolingOn = false;
		ASICAMERA->isColorCamera = false;
		ASICAMERA->isCoolCamera = false;
		ASICAMERA->isGuidingCamera = false;
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
		if (ASICAMERA->isCameraConnected == true)
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
		ASICAMERA->Count = ASIGetNumOfConnectedCameras();
		if(ASICAMERA->Count <= 0)
		{
			IDLog_Error(_("ASI camera not found, please check the power supply or make sure the camera is connected.\n"));
			return false;
		}
		else
		{
			for(int i = 0;i < ASICAMERA->Count;i++)
			{
				/*获取相机信息*/
				if((errCode = ASIGetCameraProperty(&ASICameraInfo, i)) != ASI_SUCCESS)
				{
					IDLog_Error(_("Unable to get %s configuration information,the error code is %d,please check program permissions.\n"),ASICameraInfo.Name,errCode);
					return false;
				}
				else
				{
					if(ASICameraInfo.Name == Device_name)
					{
						IDLog("Find %s.\n",ASICameraInfo.Name);
						ASICAMERA->ID = ASICameraInfo.CameraID;
						ASICAMERA->Name[ASICAMERA->ID] = ASICameraInfo.Name;
						/*打开相机*/
						if((errCode = ASIOpenCamera(ASICAMERA->ID)) != ASI_SUCCESS)		
						{
							IDLog_Error(_("Unable to turn on the %s,error code is %d.\n"),ASICAMERA->Name[ASICAMERA->ID],errCode);
							return false;
						}
						else
						{
							/*初始化相机*/
							if((errCode = ASIInitCamera(ASICAMERA->ID)) != ASI_SUCCESS)	
							{
								IDLog_Error(_("Unable to initialize connection to camera,the error code is %d.\n"),errCode);
								return false;
							}
							else 
							{
								ASICAMERA->isCameraConnected = true;
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
		if(ASICAMERA->InVideo == true)
		{
			if((errCode = ASIStopVideoCapture(ASICAMERA->ID)) != ASI_SUCCESS)		//停止视频拍摄
			{
				IDLog("Unable to stop video capture,error code is %d,please try again.\n",errCode);
				return false;
			}
			IDLog("Stop video capture.\n");
		}
		if(ASICAMERA->InExposure == true)
		{
			if((errCode = ASIStopExposure(ASICAMERA->ID)) != ASI_SUCCESS)		//停止曝光
			{
				IDLog("Unable to stop exposure,error code is %d,please try again.\n",errCode);
				return false;
			}
			IDLog("Stop exposure.\n");
		}
		/*在关闭相机之前保存设置*/
		SaveCameraConfig();
		/*关闭相机*/
		if((errCode = ASICloseCamera(ASICAMERA->ID)) != ASI_SUCCESS)		//关闭相机
		{
			IDLog("Unable to turn off the camera,error code is %d,please try again\n",errCode);
			return false;
		}
		IDLog("Disconnect from camera\n");
		return true;
    }
    
	std::string ASICCD::ReturnDeviceName()
	{
		return ASICAMERA->Name[ASICAMERA->ID];
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
		ASICAMERA->isColorCamera = ASICameraInfo.IsColorCam;
		/*判断是否为制冷相机*/
		ASICAMERA->isCoolCamera = ASICameraInfo.IsCoolerCam;
		/*判断是否为导星相机*/
		ASICAMERA->isGuidingCamera = ASICameraInfo.ST4Port;
		/*获取相机格式*/
		ASICAMERA->ImageType = ASICameraInfo.SupportedVideoFormat[7];
		/*获取相机最大画幅*/
		ASICAMERA->Image_Width = ASICAMERA->ImageMaxWidth = ASICameraInfo.MaxWidth;
		ASICAMERA->Image_Height = ASICAMERA->ImageMaxHeight = ASICameraInfo.MaxHeight;
		IDLog(_("Camera information obtained successfully.\n"));
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
		if(ASICAMERA->isCoolCamera == true)
		{
			if(!SetTemperature(CamTemp))
				return false;
			if(CoolerOFF == true)
				ActiveCool(false);
			else
				ActiveCool(true);
		}
		else
		{
			IDLog_Error("This is not a cooling camera. The cooling mode cannot be turned on. Please choose another camera\n");
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
			IDLog_Error("The temperature setting is unreasonable, please reset it.\n");
			return false;
		}
		/*检查是否可以制冷*/
		if(ActiveCool(true) == false)
		{
			IDLog_Error("Unable to start camera cooling, please check the power connection.\n");
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
		if((errCode = ASISetControlValue(ASICAMERA->ID,ASI_TEMPERATURE,TargetTemp,ASI_FALSE)) != ASI_SUCCESS)
		{
			IDLog_Error("Unable to set camera temperature,error code is %d.\n",errCode);
			return false;
		}
		ASICAMERA->Temperature = temperature;
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
		if(ASICAMERA->isCoolCamera == true)
		{
			if((errCode = ASISetControlValue(ASICAMERA->ID,ASI_COOLER_ON,enable ? ASI_TRUE : ASI_FALSE,ASI_FALSE)) != ASI_SUCCESS)
			{
				IDLog_Error("Unable to turn on refrigeration,error code is %d,please check the power supply.\n",errCode);
				return false;
			}
			ASICAMERA->isCameraCoolingOn = true;
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
		const long blink_duration = exp * 1000000;
		ASICAMERA->Bin = bin;
		ASICAMERA->Exposure = exp;
		IDLog(_("Blinking %ld time(us) before exposure\n"), blink_duration);
		if((errCode = ASISetControlValue(ASICAMERA->ID, ASI_EXPOSURE, blink_duration, ASI_FALSE)) != ASI_SUCCESS)
		{
			IDLog_Error(_("Failed to set blink exposure to %ldus, error %d\n"), blink_duration, errCode);
			return false;
		}
		else
		{
			if(!SetCameraConfig(bin,Gain,Offset))
			{
				IDLog_Error(_("Failed to set camera configure\n"));
				return false;
			}
			else
			{
				if((errCode = ASIStartExposure(ASICAMERA->ID, ASI_FALSE)) != ASI_SUCCESS)
				{
					IDLog_Error(_("Failed to start blink exposure, error %d,try it again\n"), errCode);
					AbortExposure();
					return false;
				}
				else
				{
					ASICAMERA->InExposure = true;
					do
					{
						usleep(10000);
						errCode = ASIGetExpStatus(ASICAMERA->ID, &expStatus);
					}
					while (errCode == ASI_SUCCESS && expStatus == ASI_EXP_WORKING);
					if (errCode != ASI_SUCCESS)
					{
						IDLog("Blink exposure failed, error %d, status %d\n", errCode, expStatus);
						AbortExposure();
						return false;
					}
					ASICAMERA->InExposure = false;
                }
            }
        }
        if(IsSave == true)
        {
			ASICAMERA->LastImageName = FitsName;
			IDLog("Finished exposure and save image locally\n");
			if(!SaveImage(FitsName))
			{
				IDLog_Error(_("Could not save image correctly,please check the config\n"));
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
		if((errCode = ASIStopExposure(ASICAMERA->ID)) != ASI_SUCCESS)
		{
			IDLog_Error("Unable to stop camera exposure,error id is %d,please try again.\n",errCode);
			return false;
		}
		ASICAMERA->InExposure = false;
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
		if((errCode = ASISetControlValue(ASICAMERA->ID, ASI_GAIN, ASICAMERA->Gain = Gain, ASI_FALSE)) != ASI_SUCCESS)
		{
			IDLog_Error(_("Unable to set camera gain,error code is %d\n"),errCode);
			return false;
		}
		if((errCode = ASISetControlValue(ASICAMERA->ID, ASI_BRIGHTNESS, ASICAMERA->Offset = Offset, ASI_FALSE)) != ASI_SUCCESS)
		{
			IDLog_Error(_("Unable to set camera offset,error code is %d\n"),errCode);
			return false;
		}
		ASICAMERA->Bin = Bin;
		ASICAMERA->Image_Height = ASICAMERA->ImageMaxHeight/Bin;
		ASICAMERA->Image_Width = ASICAMERA->ImageMaxWidth/Bin;
		if((errCode = ASISetROIFormat(ASICAMERA->ID, ASICAMERA->Image_Width , ASICAMERA->Image_Height , Bin, (ASI_IMG_TYPE)ASICAMERA->ImageType)) != ASI_SUCCESS)
		{
			IDLog_Error(_("Unable to set camera offset,error code is %d\n"),errCode);
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
		if(ASICAMERA->InExposure == false && ASICAMERA->InVideo == false)
		{
			long imgSize = ASICAMERA->Image_Width*ASICAMERA->Image_Height*(1 + (ASICAMERA->ImageType==ASI_IMG_RAW16));		//设置图像大小
			unsigned char * imgBuf = new unsigned char[imgSize];		//图像缓冲区大小
			/*曝光后获取图像信息*/
			if ((errCode = ASIGetDataAfterExp(ASICAMERA->ID, imgBuf, imgSize)) != ASI_SUCCESS)
			{
				/*获取图像失败*/
				IDLog_Error(_("ASIGetDataAfterExp error (%d)\n"),errCode);
				return false;
			}
			IDLog(_("Download from camera completely.\n"));
			/*将图像写入本地文件*/
			#ifdef HAS_FITSIO
				FitsIO::SaveFitsImage(imgBuf,FitsName.c_str(),ASICAMERA->ImageType,ASICAMERA->isColorCamera,ASICAMERA->Image_Height,ASICAMERA->Image_Width,ASICAMERA->Name[ASICAMERA->ID],ASICAMERA->Exposure,ASICAMERA->Bin,ASICAMERA->Offset,ASICAMERA->Gain,ASICAMERA->Temperature);
			#endif
			#ifdef HAS_OPENCV
				IMGINFO->img_data = "data:image/jpg;base64," + ImageTools::ConvertUCto64(imgBuf,ASICAMERA->isColorCamera,ASICAMERA->Image_Height,ASICAMERA->Image_Width);
			#endif
			if(imgBuf)
				delete[] imgBuf;		//删除图像缓存
		}
		return true;
	}

	/*
     * name: SaveCameraConfig()
     * describe: Save camera configuration
     * 描述：保存相机参数
     */
	bool ASICCD::SaveCameraConfig()
    {
        Json::Value Root;
        Root["Brand"] = Json::Value("ZWOASI");
        Root["Name"] = Json::Value(ASICAMERA->Name[ASICAMERA->ID]);
		/*相机设置*/
        Root["Config"]["BinMode"] = Json::Value(ASICAMERA->Bin);
        Root["Config"]["Exposure"] = Json::Value(ASICAMERA->Exposure);
		Root["Config"]["CameraFrameWidth"] = Json::Value(ASICAMERA->Image_Width);
		Root["Config"]["CameraFrameHeight"] = Json::Value(ASICAMERA->Image_Height);
		Root["Config"]["ImageType"] = Json::Value(ASICAMERA->ImageType);
		Root["Config"]["Temperature"] = Json::Value(ASICAMERA->Temperature);
		/*相机信息*/
		Root["Info"]["IsCoolCamera"] = Json::Value(ASICAMERA->isCoolCamera);
		Root["Info"]["IsColorCamera"] = Json::Value(ASICAMERA->isColorCamera);
		Root["Info"]["IsGuidingCamera"] = Json::Value(ASICAMERA->isGuidingCamera);
		Root["Info"]["MaxFrameWidth"] = Json::Value(ASICAMERA->ImageMaxWidth);
		Root["Info"]["MaxFrameHeight"] = Json::Value(ASICAMERA->ImageMaxHeight);
		Root["Info"]["LastImageName"] = Json::Value(ASICAMERA->LastImageName);
		/*输出至对应相机名称文件*/
		std::string temp = ASICAMERA->Name[ASICAMERA->ID];
		std::ofstream out(("config/camera/" + temp + ".json"),std::ios::trunc);
		out << Root.toStyledString();
		out.close();
		return true;
    }
}




