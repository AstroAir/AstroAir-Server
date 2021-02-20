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

#include <fitsio.h>

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
		/*初始化SDK*/
		if((retVal = InitQHYCCDResource()) != QHYCCD_SUCCESS)
		{
			IDLog("Unable to initialize SDK settings,error code is %d please check system settings\n",retVal);
			return false;
		}
		else
		{
			IDLog("Init SDK successfully!\n");
			/*搜索QHY相机数量*/
			if((CamNumber = ScanQHYCCD()) <= 0)
			{
				IDLog("QHY camera not found, please check the power supply or make sure the camera is connected.\n");
				return false;
			}
			else
			{
				/*根据相机数量依次搜索*/
				for(int i = 0;i < CamNumber;i++)
				{
					/*获取相机ID*/
					if((GetQHYCCDId(i, iCamId)) != QHYCCD_SUCCESS)
					{
						IDLog("Unable to get camera ID, please check connection\n");
						return false;
					}
					strcpy(CamId,iCamId);
					char *p = strtok(iCamId,"-");
					/*判断当前相机是否为指定相机*/
					if(p == Device_name)
					{
						IDLog("Find %s.\n",iCamId);
						/*打开相机*/
						if((pCamHandle = OpenQHYCCD(CamId)) == NULL)
						{
							IDLog("Unable to turn on the %s.\n",iCamId);
							return false;
						}
						else
						{
							retVal = IsQHYCCDControlAvailable(pCamHandle, CAM_SINGLEFRAMEMODE);
							if (SetQHYCCDStreamMode(pCamHandle, 0) != QHYCCD_SUCCESS)
							{
								IDLog("This camera doesn't support single frame shooting\n");
								return false;
							}
							/*初始化相机*/
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
			isColorCamera = true;
			channels = 3;
		}
		if((retVal = IsQHYCCDControlAvailable(pCamHandle, CONTROL_COOLER)) == QHYCCD_SUCCESS)
			isCoolCamera = true;
		if((retVal = IsQHYCCDControlAvailable(pCamHandle, CONTROL_ST4PORT)) == QHYCCD_SUCCESS)
			isGuideCamera = true;
		if((retVal = GetQHYCCDChipInfo(pCamHandle, &chipWidth, &chipHeight, &iMaxWidth, &iMaxHeight, &pixelWidth, &pixelHeight, &Image_type)) != QHYCCD_SUCCESS)
		{
			IDLog("Unable to get camera parameters, please check the connection\n");
			return false;
		}
		CamWidth = iMaxWidth;
		CamHeight = iMaxHeight;
		IDLog("Camera information obtained successfully.\n");
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
		std::unique_lock<std::mutex> guard(condMutex);
		double blink_duration = exp * 1000000;
		CamBin = bin;
		IDLog("Blinking %ld time(s) before exposure\n", blink_duration);
		if((retVal = IsQHYCCDControlAvailable(pCamHandle,CONTROL_EXPOSURE)) != QHYCCD_SUCCESS || (retVal = SetQHYCCDParam(pCamHandle,CONTROL_EXPOSURE,blink_duration)) != QHYCCD_SUCCESS)
		{
			IDLog("Failed to set blink exposure to %ldus, error %d\n", blink_duration, retVal);
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
				InExposure = true;
				if((retVal = ExpQHYCCDSingleFrame(pCamHandle)) != QHYCCD_ERROR)
				{
					usleep(10);
				}
				else
				{
					IDLog("Blink exposure failed, error code is %d\n", retVal);
					AbortExposure();
					return false;
                }
				InExposure = false;
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
     * calls: CancelQHYCCDExposingAndReadout()
     * calls: IDLog()
     */
	bool QHYCCD::AbortExposure()
	{
		IDLog("Aborting camera exposure...\n");
		if((retVal = CancelQHYCCDExposingAndReadout(pCamHandle)) != QHYCCD_SUCCESS)
		{
			IDLog("Unable to stop camera exposure,error id is %d,please try again.\n",retVal);
			return false;
		}
		InExposure = false;
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
			IDLog("Unable to set camera USBTRAFFIC failure, error code is  %d\n", retVal);
			return false;
		}
		/*设置相机增益*/
		retVal = IsQHYCCDControlAvailable(pCamHandle, CONTROL_GAIN);
		if ((retVal = SetQHYCCDParam(pCamHandle, CONTROL_GAIN, Gain)) != QHYCCD_SUCCESS)
		{
			IDLog("Unable to set camera GAIN failure, error code is  %d\n", retVal);
			return false;
		}
		/*设置相机偏置*/
		retVal = IsQHYCCDControlAvailable(pCamHandle, CONTROL_OFFSET);
		if((retVal = SetQHYCCDParam(pCamHandle, CONTROL_OFFSET, Offset)) != QHYCCD_SUCCESS)
		{
			IDLog("Unable to set camera OFFSET failure, error code is  %d\n", retVal);
			return false;
		}
		/*设置像素合并模式*/
		if((retVal = SetQHYCCDBinMode(pCamHandle,Bin,Bin)) != QHYCCD_SUCCESS)
		{
			IDLog("Unable to set camera BIN MODE failure, error code is  %d\n", retVal);
			return false;
		}
		else
		{
			if((retVal = SetQHYCCDResolution(pCamHandle, 0, 0, CamWidth/Bin, CamHeight/Bin)) != QHYCCD_SUCCESS)
			{
				IDLog("Unable to set camera frame size failure, error code is  %d\n", retVal);
				return false;
			}
			CamBin = Bin;
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
			IDLog("Unable to set camera speed failure, error code is  %d\n", retVal);
			return false;
		}
		/*设置相机图像深度*/
		if((retVal = IsQHYCCDControlAvailable(pCamHandle, CAM_16BITS)) == QHYCCD_SUCCESS)
		{
			if ((retVal = SetQHYCCDParam(pCamHandle, CONTROL_TRANSFERBIT, 16)) != QHYCCD_SUCCESS)
			{
				IDLog("Unable to set camera 16 bits mode failure, error code is  %d\n", retVal);
				return false;
			}
		}
		else
		{
			if ((retVal = SetQHYCCDParam(pCamHandle, CONTROL_TRANSFERBIT,8)) != QHYCCD_SUCCESS)
			{
				IDLog("Unable to set camera 8 bits mode failure, error code is  %d\n", retVal);
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
		if(InExposure == false && InVideo == false)
		{	
			//std::unique_lock<std::mutex> guard(ccdBufferLock);
			uint32_t imgSize = GetQHYCCDMemLength(pCamHandle);		//设置图像大小
			//long imgSize = CamWidth*CamHeight*(1 + (Image_type==16));
			unsigned char * imgBuf = new unsigned char [imgSize];		//图像缓冲区大小
			long naxis = 2;
			CamWidth /= CamBin;
			CamHeight /= CamBin;
			/*曝光后获取图像信息*/
			if ((retVal = GetQHYCCDSingleFrame(pCamHandle, &CamWidth, &CamHeight, &Image_type, &channels, imgBuf)) != QHYCCD_SUCCESS)
			{
				/*获取图像失败*/
				IDLog("GetQHYCCDSingleFrame error (%d)\n",retVal);
				return false;
			}
			//guard.unlock();
			IDLog("Download complete.\n");
			/*将图像写入本地文件*/
			#ifdef HAS_FITSIO
				char datatype[40];		//相机品牌
				char keywords[40];		//相机品牌
				char value[20];		//相机名称
				char description[40];		//相机描述
				strcpy(datatype, "TSTRING");
				strcpy(keywords, "Camera");
				strcpy(value,iCamId);
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
