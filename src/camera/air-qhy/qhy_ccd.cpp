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
#include "../../tools/ImgTools.h"

#include "../../logger.h"
#include "../../config.h"

#include <string.h>
#include <unistd.h>
#include <json/json.h>
#include <fstream>

namespace AstroAir
{
	/*
     * name: QHYCCD()
     * describe: Initialization, for camera constructor
     * 描述：构造函数，用于初始化相机参数
     */
	QHYCCD::QHYCCD(CameraInfo *NEW)
	{
		QHYCAMERA = NEW;
		QHYCAMERA->Exposure = 0;
		QHYCAMERA->ExposureUsed = 0;
		QHYCAMERA->Gain = 0;
		QHYCAMERA->Offset = 0;
		QHYCAMERA->Temperature = 0;
		QHYCAMERA->ID = 0;
		QHYCAMERA->Image_Height = 0;
		QHYCAMERA->Image_Width = 0;
		QHYCAMERA->ImageMaxHeight = 0;
		QHYCAMERA->ImageMaxWidth = 0;
		QHYCAMERA->InExposure = false;
		QHYCAMERA->isCameraConnected = false;
		QHYCAMERA->isCameraCoolingOn = false;
		QHYCAMERA->isColorCamera = false;
		QHYCAMERA->isCoolCamera = false;
		QHYCAMERA->isGuidingCamera = false;
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
		if (QHYCAMERA->isCameraConnected)
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
		/*初始化SDK*/
		if((retVal = InitQHYCCDResource()) != QHYCCD_SUCCESS)
		{
			IDLog_Error(_("Unable to initialize SDK settings,error code is %d please check system settings\n"),retVal);
			return false;
		}
		else
		{
			IDLog(_("Init SDK successfully!\n"));
			/*搜索QHY相机数量*/
			if((QHYCAMERA->Count = ScanQHYCCD()) <= 0)
			{
				IDLog_Error(_("QHY camera not found, please check the power supply or make sure the camera is connected.\n"));
				return false;
			}
			else
			{
				/*根据相机数量依次搜索*/
				for(int i = 0;i < QHYCAMERA->Count;i++)
				{
					/*获取相机ID*/
					if((GetQHYCCDId(i, iCamId)) != QHYCCD_SUCCESS)
					{
						IDLog_Error(_("Unable to get camera ID, please check connection\n"));
						return false;
					}
					strcpy(CamId,iCamId);
					/*判断当前相机是否为指定相机*/
					if(strtok(iCamId,"-") == Device_name)
					{
						IDLog(_("Find %s.\n"),iCamId);
						/*打开相机*/
						if((pCamHandle = OpenQHYCCD(CamId)) == NULL)
						{
							IDLog_Error(_("Unable to turn on the %s.\n"),iCamId);
							return false;
						}
						else
						{
							retVal = IsQHYCCDControlAvailable(pCamHandle, CAM_SINGLEFRAMEMODE);
							if (SetQHYCCDStreamMode(pCamHandle, 0) != QHYCCD_SUCCESS)
							{
								IDLog_Error(_("This camera doesn't support single frame shooting\n"));
								return false;
							}
							/*初始化相机*/
							if(InitQHYCCD(pCamHandle) != QHYCCD_SUCCESS)
							{
								IDLog_Error(_("Unable to initialize connection to camera.\n"));
								return false;
							}
							else
							{
								QHYCAMERA->isCameraConnected = true;
								QHYCAMERA->isCameraConnected = true;
								IDLog(_("Camera turned on successfully\n"));
								/*获取连接相机配置信息，并存入参数*/
								UpdateCameraConfig();
								return true;
							}
						}
					}
				}
				IDLog_Error(_("The specified camera was not found. Please check the camera connection"));
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
		if(QHYCAMERA->InVideo)
		{
			if(StopQHYCCDLive(pCamHandle) != QHYCCD_SUCCESS)		//停止视频拍摄
			{
				IDLog_Error(_("Unable to stop video capture, please try again.\n"));
				return false;
			}
			IDLog(_("Stop video capture.\n"));
		}
		if(QHYCAMERA->InExposure)
		{
			if(CancelQHYCCDExposingAndReadout(pCamHandle) != QHYCCD_SUCCESS)		//停止曝光
			{
				IDLog_Error(_("Unable to stop exposure, please try again.\n"));
				return false;
			}
			IDLog(_("Stop exposure.\n"));
		}
		/*在关闭相机之前保存设置*/
		//SaveConfig();
		/*关闭相机*/
		if(CloseQHYCCD(pCamHandle) != QHYCCD_SUCCESS)		//关闭相机
		{
			IDLog_Error(_("Unable to turn off the camera, please try again"));
			return false;
		}
		if ((retVal = ReleaseQHYCCDResource()) != QHYCCD_SUCCESS)
		{
			IDLog_Error(_("Cannot release SDK resources, error %d.\n"), retVal);
		}
		else
		{
			IDLog(_("SDK resources released.\n")); 
		}
		IDLog(_("Disconnect from camera\n"));
		return true;
    }
    
	std::string QHYCCD::ReturnDeviceName()
	{
		return iCamId;
	}

	/*
     * name: UpdateCameraConfig()
     * describe: Get the required parameters of the camera
     * 描述：获取相机所需参数
     * @return ture: meaningless
	 * calls: IsQHYCCDControlAvailable()
	 * calls: GetQHYCCDChipInfo()
	 * calls: IDLog()
     * note: These parameters are very important and related to the following program
     */
    bool QHYCCD::UpdateCameraConfig()
	{
		retVal = IsQHYCCDControlAvailable(pCamHandle, CAM_COLOR);
  		if (retVal == BAYER_GB || retVal == BAYER_GR || retVal == BAYER_BG || retVal == BAYER_RG)
		{
			QHYCAMERA->isColorCamera = true;
			channels = 3;
		}
		if((retVal = IsQHYCCDControlAvailable(pCamHandle, CONTROL_COOLER)) == QHYCCD_SUCCESS)
			QHYCAMERA->isCoolCamera = true;
		if((retVal = IsQHYCCDControlAvailable(pCamHandle, CONTROL_ST4PORT)) == QHYCCD_SUCCESS)
			QHYCAMERA->isGuidingCamera = true;
		if((retVal = GetQHYCCDChipInfo(pCamHandle, &chipWidth, &chipHeight, (uint32_t*)&QHYCAMERA->ImageMaxWidth, (uint32_t*)&QHYCAMERA->ImageMaxHeight, &pixelWidth, &pixelHeight, (uint32_t*)&QHYCAMERA->ImageType)) != QHYCCD_SUCCESS)
		{
			IDLog_Error(_("Unable to get camera parameters, please check the connection\n"));
			return false;
		}
		QHYCAMERA->Image_Width = QHYCAMERA->ImageMaxWidth;
		QHYCAMERA->Image_Height = QHYCAMERA->ImageMaxHeight;
		IDLog(_("Camera information obtained successfully.\n"));
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
     * calls: IsQHYCCDControlAvailable()
	 * calls: SetQHYCCDParam()
	 * calls: SetCameraConfig()
	 * calls: ExpQHYCCDSingleFrame()
     * calls: IDLog()
     * calls: AnortExposure()
     * calls: SaveImage()
     */
	bool QHYCCD::StartExposure(int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset)
	{
		double blink_duration = exp * 1000000;
		QHYCAMERA->Bin = bin;
		IDLog(_("Blinking %ld time(s) before exposure\n"), blink_duration);
		if((retVal = IsQHYCCDControlAvailable(pCamHandle,CONTROL_EXPOSURE)) != QHYCCD_SUCCESS || (retVal = SetQHYCCDParam(pCamHandle,CONTROL_EXPOSURE,blink_duration)) != QHYCCD_SUCCESS)
		{
			IDLog_Error(_("Failed to set blink exposure to %ldus, error %d\n"), blink_duration, retVal);
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
				QHYCAMERA->InExposure = true;
				if((retVal = ExpQHYCCDSingleFrame(pCamHandle)) != QHYCCD_ERROR)
				{
					usleep(10);
				}
				else
				{
					IDLog_Error(_("Blink exposure failed, error code is %d\n"), retVal);
					AbortExposure();
					return false;
                }
				QHYCAMERA->InExposure = false;
            }
        }
        if(IsSave == true)
        {
			IDLog(_("Finished exposure and save image locally\n"));
			if(SaveImage(FitsName) != true)
			{
				IDLog_Error(_("Could not save image correctly,please check the config\n"));
				return false;
			}
			else
			{
				IDLog(_("Saved Fits and JPG images %s successfully in locally\n"),FitsName.c_str());
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
     * calls: CancelQHYCCDExposingAndReadout()
     * calls: IDLog()
     */
	bool QHYCCD::AbortExposure()
	{
		IDLog(_("Aborting camera exposure...\n"));
		if((retVal = CancelQHYCCDExposingAndReadout(pCamHandle)) != QHYCCD_SUCCESS)
		{
			IDLog_Error(_("Unable to stop camera exposure,error id is %d,please try again.\n"),retVal);
			return false;
		}
		QHYCAMERA->InExposure = false;
		return true;
	}
	
	/*
     * name: SetCameraConfig(double Bin,double Gain,double Offset)
     * describe: set camera cinfig
     * 描述：设置相机参数
     * calls: IDLog()
     * calls: IsQHYCCDControlAvailable()
     * calls: SetQHYCCDStreamMode()
	 * calls: SetQHYCCDParam()
	 * calls: SetQHYCCDResolution()
     */
	bool QHYCCD::SetCameraConfig(double Bin,double Gain,double Offset)
	{
		retVal = IsQHYCCDControlAvailable(pCamHandle, CONTROL_USBTRAFFIC);
  		if ((retVal = SetQHYCCDParam(pCamHandle, CONTROL_USBTRAFFIC, 50)) != QHYCCD_SUCCESS)
  		{
			IDLog_Error(_("Unable to set camera USBTRAFFIC failure, error code is  %d\n"), retVal);
			return false;
		}
		/*设置相机增益*/
		retVal = IsQHYCCDControlAvailable(pCamHandle, CONTROL_GAIN);
		if ((retVal = SetQHYCCDParam(pCamHandle, CONTROL_GAIN, Gain)) != QHYCCD_SUCCESS)
		{
			IDLog_Error(_("Unable to set camera GAIN failure, error code is  %d\n"), retVal);
			return false;
		}
		/*设置相机偏置*/
		retVal = IsQHYCCDControlAvailable(pCamHandle, CONTROL_OFFSET);
		if((retVal = SetQHYCCDParam(pCamHandle, CONTROL_OFFSET, Offset)) != QHYCCD_SUCCESS)
		{
			IDLog_Error(_("Unable to set camera OFFSET failure, error code is  %d\n"), retVal);
			return false;
		}
		/*设置像素合并模式*/
		QHYCAMERA->Image_Height =  QHYCAMERA->Image_Height / Bin;
		QHYCAMERA->Image_Width = QHYCAMERA->Image_Width /Bin;
		if((retVal = SetQHYCCDBinMode(pCamHandle,Bin,Bin)) != QHYCCD_SUCCESS)
		{
			IDLog_Error(_("Unable to set camera BIN MODE failure, error code is  %d\n"), retVal);
			return false;
		}
		else
		{
			if((retVal = SetQHYCCDResolution(pCamHandle, 0, 0, QHYCAMERA->Image_Width, QHYCAMERA->Image_Height)) != QHYCCD_SUCCESS)
			{
				IDLog_Error(_("Unable to set camera frame size failure, error code is  %d\n"), retVal);
				return false;
			}
			QHYCAMERA->Bin = Bin;
		}
		/*设置相机USB速度
		retVal = IsQHYCCDControlAvailable(pCamHandle, CONTROL_SPEED);
		if((retVal = SetQHYCCDParam(pCamHandle, CONTROL_SPEED,1)) != QHYCCD_SUCCESS)
		{
			IDLog("Unable to set camera speed failure, error code is  %d\n", retVal);
			return false;
		}
		*/
		retVal = SetQHYCCDParam(pCamHandle, CONTROL_DDR, 1.0);
		retVal = IsQHYCCDControlAvailable(pCamHandle, CONTROL_USBTRAFFIC);
		if((retVal = SetQHYCCDParam(pCamHandle, CONTROL_USBTRAFFIC,50)) != QHYCCD_SUCCESS)
		{
			IDLog_Error(_("Unable to set camera speed failure, error code is  %d\n"), retVal);
			return false;
		}
		/*设置相机图像深度*/
		if((retVal = IsQHYCCDControlAvailable(pCamHandle, CAM_16BITS)) == QHYCCD_SUCCESS)
		{
			if ((retVal = SetQHYCCDParam(pCamHandle, CONTROL_TRANSFERBIT, 16)) != QHYCCD_SUCCESS)
			{
				IDLog_Error(_("Unable to set camera 16 bits mode failure, error code is  %d\n"), retVal);
				return false;
			}
		}
		else
		{
			if ((retVal = SetQHYCCDParam(pCamHandle, CONTROL_TRANSFERBIT,8)) != QHYCCD_SUCCESS)
			{
				IDLog_Error(_("Unable to set camera 8 bits mode failure, error code is  %d\n"), retVal);
				return false;
			}
		}
		return true;
	}

	/*
     * name: SaveImage(std::string FitsName)
     * describe: Save images
     * 描述：存储图像
     * calls: GetQHYCCDMemLength()
	 * calls: GetQHYCCDSingleFrame()
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
    bool QHYCCD::SaveImage(std::string FitsName)
    {
		if(QHYCAMERA->InExposure == false && QHYCAMERA->InVideo == false)
		{
			uint32_t imgSize = GetQHYCCDMemLength(pCamHandle);		//设置图像大小
			unsigned char * imgBuf = new unsigned char [imgSize];		//图像缓冲区大小
			/*曝光后获取图像信息*/
			if ((retVal = GetQHYCCDSingleFrame(pCamHandle, (uint32_t*)&QHYCAMERA->Image_Width, (uint32_t*)&QHYCAMERA->Image_Height, (uint32_t*)&QHYCAMERA->ImageType, &channels, imgBuf)) != QHYCCD_SUCCESS)
			{
				/*获取图像失败*/
				IDLog_Error(_("GetQHYCCDSingleFrame error (%d)\n"),retVal);
				return false;
			}
			IDLog(_("Download complete.\n"));
			/*将图像写入本地文件*/
			#ifdef HAS_FITSIO
				FitsIO::SaveFitsImage(imgBuf,FitsName.c_str(),QHYCAMERA->ImageType,QHYCAMERA->isColorCamera,QHYCAMERA->Image_Height,QHYCAMERA->Image_Width,QHYCAMERA->Name[QHYCAMERA->ID],QHYCAMERA->Exposure,QHYCAMERA->Bin,QHYCAMERA->Offset,QHYCAMERA->Gain,QHYCAMERA->Temperature);
			#endif
			#ifdef HAS_OPENCV
				IMGINFO->img_data = "data:image/jpg;base64," + ImageTools::ConvertUCto64(imgBuf,QHYCAMERA->isColorCamera,QHYCAMERA->Image_Height,QHYCAMERA->Image_Width);
			#endif
			if(imgBuf)
				delete[] imgBuf;		//删除图像缓存
		}
		return true;
	}

	bool QHYCCD::Cooling(bool SetPoint,bool CoolDown,bool ASync,bool Warmup,bool CoolerOFF,int CamTemp)
	{
		if(QHYCAMERA->isCoolCamera == true)
		{

		}
		else
		{
			IDLog_Error(_("This is not a cooling camera. The cooling mode cannot be turned on. Please choose another camera\n"));
			return false;
		}
		return true;
	}

	/*
     * name: SaveCameraConfig()
     * describe: Save camera configuration
     * 描述：保存相机参数
     */
	bool QHYCCD::SaveCameraConfig()
    {
        Json::Value Root;
        Root["Brand"] = Json::Value("QHYCCD");
        Root["Name"] = Json::Value(QHYCAMERA->Name[QHYCAMERA->ID]);
		/*相机设置*/
        Root["Config"]["BinMode"] = Json::Value(QHYCAMERA->Bin);
        Root["Config"]["Exposure"] = Json::Value(QHYCAMERA->Exposure);
		Root["Config"]["CameraFrameWidth"] = Json::Value(QHYCAMERA->Image_Width);
		Root["Config"]["CameraFrameHeight"] = Json::Value(QHYCAMERA->Image_Height);
		Root["Config"]["ImageType"] = Json::Value(QHYCAMERA->ImageType);
		Root["Config"]["Temperature"] = Json::Value(QHYCAMERA->Temperature);
		/*相机信息*/
		Root["Info"]["IsCoolCamera"] = Json::Value(QHYCAMERA->isCoolCamera);
		Root["Info"]["IsColorCamera"] = Json::Value(QHYCAMERA->isColorCamera);
		Root["Info"]["IsGuidingCamera"] = Json::Value(QHYCAMERA->isGuidingCamera);
		Root["Info"]["MaxFrameWidth"] = Json::Value(QHYCAMERA->ImageMaxWidth);
		Root["Info"]["MaxFrameHeight"] = Json::Value(QHYCAMERA->ImageMaxHeight);
		Root["Info"]["LastImageName"] = Json::Value(QHYCAMERA->LastImageName);
		/*输出至对应相机名称文件*/
		std::string temp = QHYCAMERA->Name[QHYCAMERA->ID];
		std::ofstream out("config/camera/" + temp + ".json",std::ios::trunc);
		out << Root.toStyledString();
		out.close();
		return true;
    }
}
