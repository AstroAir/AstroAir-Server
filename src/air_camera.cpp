/*
 * air_camera.cpp
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
 
Date:2021-7-06
 
Description:Camera Offical Port

**************************************************/

#include "air_camera.h"
#include "wsserver.h"
#include "logger.h"

namespace AstroAir
{
    AIRCAMERA *CCD = nullptr;
    CameraInfo NEW;
    CameraInfo *AIRCAMINFO = &NEW;
    
    /*
     * name: AIRCAMERA()
     * describe: Constructor for initializing camera parameters
     * 描述：构造函数，用于初始化相机参数
     */
    AIRCAMERA::AIRCAMERA()
    {
        AIRCAMINFO->InExposure = false;
        InSequenceRun = false;
    }

    /*
     * name: ~AIRCAMERA()
     * describe: Destructor
     * 描述：析构函数
     */
    AIRCAMERA::~AIRCAMERA()
    {
        if(AIRCAMINFO->InExposure|| InSequenceRun)
            CCD->AbortExposure();
        AIRCAMINFO->InExposure = false;
        InSequenceRun = false;
    }

    /*
     * name: Connect()
     * describe: Connect from camera
     * 描述：连接（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    bool AIRCAMERA::Connect(std::string Device_name)
    {
        return true;
    }

    /*
     * name: Disconnect()
     * describe: Disconnect from camera
     * 描述：断开连接（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    bool AIRCAMERA::Disconnect()
    {
        return true;
    }

    /*
     * name: ReturnDeviceName()
     * describe: Return device's name
     * 描述：返回设备名称（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    std::string AIRCAMERA::ReturnDeviceName()
    {
        return "None";
    }

    /*
     * name: StartExposureServer(int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset)
     * @param exp:相机曝光时间
     * @param bin:像素合并
     * @param IsSave:是否保存图像
     * @param FitsName:保存图像名称
     * @param Gain:相机增益
     * @param Offset:相机偏置
     * describe: Start exposure
     * 描述：开始曝光
     * calls: ImagineThread()
	 * calls: StartExposure(int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset)
     * calls: IDLog(const char *fmt, ...)
     * calls: IDLog_DEBUG(const char *fmt, ...)
	 * calls :StartExposureError(std::string message）
	 * note:This function should not be executed normally
     */
    bool AIRCAMERA::StartExposureServer(int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset)
    {
        if(exp <= 0)
        {
            IDLog(_("Exposure time is less than 0, please input a reasonable data\n"));
            WebLog(_("Exposure time is less than 0, please input a reasonable data"),3);
            StartExposureError();
            ShotRunningSend(0,4);
            return false;
        }
		if(AIRCAMINFO->isCameraConnected)
		{
            AIRCAMINFO->LastImageName = FitsName;
            AIRCAMINFO->Bin = bin;
            AIRCAMINFO->Exposure = exp;
            AIRCAMINFO->InExposure = true;
            std::thread CameraCountThread(&AIRCAMERA::ImagineThread,this);
            CameraCountThread.detach();
            WebLog(_("Start exposure!"),2);
			if(!CCD->StartExposure(exp, bin, IsSave, FitsName, Gain, Offset))
			{
				/*返回曝光错误的原因*/
				StartExposureError();
                ShotRunningSend(0,4);
				IDLog(_("Unable to start the exposure of the camera. Please check the connection of the camera. If you have any problems, please contact the developer\n"));
                WebLog(_("Unable to start the exposure of the camera"),3);
				AIRCAMINFO->InExposure = false;
                /*如果函数执行不成功返回false*/
				return false;
			}
            AIRCAMINFO->InExposure = false;
			/*将拍摄成功的消息返回至客户端*/
			StartExposureSuccess();
            WebLog("Successfully exposure",2);
            ShotRunningSend(100,2);
            newJPGReadySend();
		}
		else
		{
			IDLog_Error(_("There seems to be some unknown mistakes here.Maybe you need to check the camera connection\n"));
			return false;
        }
        return true;
    }


    /*
     * name: StartExposure(int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset)
     * @param exp:相机曝光时间
     * @param bin:像素合并
     * @param IsSave:是否保存图像
     * @param FitsName:保存图像名称
     * @param Gain:相机增益
     * @param Offset:相机偏置
     * describe: Start exposure
     * 描述：开始曝光（无任何实际用途，仅作为一个模板）
	 * note:This function should not be executed normally
     */
    bool AIRCAMERA::StartExposure(int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset)
    {
        return true;
    }

    bool AIRCAMERA::StartExposureSeq(int loop,int exp,int bin,bool IsSave,std::string FitsName,int Gain,int Offset)
    {
        return true;
    }
    
    /*
     * name: ImagineThread()
     * describe: Calculate the remaining exposure time
     * 描述：计算曝光剩余时间
     * calls: ShotRunningSend()
     * note: This thread is orphan. Please note
     */
    void AIRCAMERA::ImagineThread()
    {
        while(AIRCAMINFO->ExposureUsed < AIRCAMINFO->Exposure && AIRCAMINFO->InExposure)
        {
            sleep(1);
            AIRCAMINFO->ExposureUsed++;
            ShotRunningSend((AIRCAMINFO->ExposureUsed/AIRCAMINFO->Exposure)*100,1);
        }
        AIRCAMINFO->ExposureUsed = 0;
    }

    /*
     * name: AbortExposureServer()
     * describe: Abort exposure(Server)
     * 描述：停止曝光
     * calls: IDLog(const char *fmt, ...)
     * calls: IDLog_DEBUG(const char *fmt, ...)
     */
    bool AIRCAMERA::AbortExposureServer()
    {
		if(AIRCAMINFO->isCameraConnected)
		{
			if (!CCD->AbortExposure())
			{
				/*返回曝光错误的原因*/
				AbortExposureError();
                WebLog(_("Unable to stop the exposure of the camera"),3);
				IDLog_Error(_("Unable to stop the exposure of the camera. Please check the connection of the camera. If you have any problems, please contact the developer\n"));
				/*如果函数执行不成功返回false*/
				return false;
			}
			/*将拍摄成功的消息返回至客户端*/
			AbortExposureSuccess();
            WebLog(_("Abort exposure successfully"),2);
		}
		else
		{
            WebLog(_("Unable to stop the exposure of the camera"),3);
			IDLog_Error(_("Try to stop exposure,Should never get here.\n"));
			return false;
        }
        return true;
    }

    /*
     * name: AbortExposure()
     * describe: Abort exposure
     * 描述：停止曝光
	 * note:This function should not be executed normally
     */
    bool AIRCAMERA::AbortExposure()
    {
        return false;
    }
    
    /*
     * name: CoolingServer(bool SetPoint,bool CoolDown,bool ASync,bool Warmup,bool CoolerOFF,int CamTemp)
     * describe: Camera Cooling Settings (Server)
     * 描述：相机制冷设置 （服务器）
     * calls: IDLog(const char *fmt, ...)
     * calls: Cooling()
     */
    bool AIRCAMERA::CoolingServer(bool SetPoint,bool CoolDown,bool ASync,bool Warmup,bool CoolerOFF,int CamTemp)
    {
        if(CoolerOFF)
        {
            if(!CCD->Cooling(false,false,false,false,true,CamTemp))
            {
                IDLog_Error(_("Unable to turn off the camera cooling mode, please check the condition of the device\n"));
                WebLog(_("Unable to turn off the camera cooling mode"),3);
                return false;
            }
        }
        if(CoolerOFF)
        {
            if(!CCD->Cooling(true,false,false,false,false,CamTemp))
            {
                IDLog_Error(_("Unable to turn on the camera cooling mode, please check the condition of the device\n"));
                WebLog(_("Unable to turn on the camera cooling mode"),3);
                return false;
            }
        }
		if(CoolDown)
		{
			if(CCD->Cooling(false,true,false,false,false,CamTemp) != true)
			{
				IDLog_Error(_("The camera can't cool down normally, please check the condition of the equipment\n"));
				WebLog(_("The camera can't cool down normally"),3);
                return false;
			}
		}
		if(Warmup)
		{
			if(!CCD->Cooling(false,false,false,true,false,CamTemp))
			{
				IDLog_Error(_("The camera can't warm up normally, please check the condition of the equipment\n"));
				WebLog(_("The camera can't warm up normally"),3);
                return false;
			}
		}
        IDLog(_("Camera cooling set successfully\n"));
        return true;
    }

    /*
     * name: Cooling(bool SetPoint,bool CoolDown,bool ASync,bool Warmup,bool CoolerOFF,int CamTemp)
     * describe: Camera Cooling Settings
     * 描述：相机制冷设置,只是一个模板
     * note:This function should not be executed normally
     */
    bool AIRCAMERA::Cooling(bool SetPoint,bool CoolDown,bool ASync,bool Warmup,bool CoolerOFF,int CamTemp)
    {
        return false;
    }

    /*
     * name: StartExposureSuccess()
     * describe: Successfully exposure
     * 描述：成功连接设备
     * calls: IDLog(const char *fmt, ...)
     * calls: send()
     */
	void AIRCAMERA::StartExposureSuccess()
	{
        IDLog(_("Successfully exposure\n"));
        /*整合信息并发送至客户端*/
        Json::Value Root;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteCameraShot");
        Root["ActionResultInt"] = Json::Value(4);
        ws.send(Root.toStyledString());
	}
	
    /*
     * name: AbortExposureSuccess()
     * describe: Successfully stop exposure
     * 描述：成功连接设备
     * calls: IDLog(const char *fmt, ...)
     * calls: send()
     */
	void AIRCAMERA::AbortExposureSuccess()
	{
		IDLog(_("Successfully stop exposure\n"));
        /*整合信息并发送至客户端*/
        Json::Value Root;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteCameraShot");
        Root["ActionResultInt"] = Json::Value(6);
        ws.send(Root.toStyledString());
	}

	/*
	 * name: StartExposureError()
	 * describe: Error handling connection to device
	 * 描述：处理开始曝光时的错误
	 * calls: IDLog(const char *fmt, ...)
	 * calls: IDLog_DEBUG(const char *fmt, ...)
	 * calls: send()
	 */
    void AIRCAMERA::StartExposureError()
    {
		IDLog_Error(_("Unable to start exposure\n"));
		/*整合信息并发送至客户端*/
        Json::Value Root;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteCameraShot");
        Root["ActionResultInt"] = Json::Value(5);
		ws.send(Root.toStyledString());
    }
    
    /*
	 * name: AbortExposureError()
	 * describe: Unable to stop camera exposure
	 * 描述：无法停止相机曝光
	 * calls: IDLog(const char *fmt, ...)
	 * calls: IDLog_DEBUG(const char *fmt, ...)
	 * calls: send()
	 */
    void AIRCAMERA::AbortExposureError()
    {
		IDLog_Error(_("Unable to stop camera exposure\n"));
		/*整合信息并发送至客户端*/
		Json::Value Root;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteCameraShot");
        Root["ActionResultInt"] = Json::Value(5);
		ws.send(Root.toStyledString());
    }
    
    /*
	 * name: ShotRunningSend(int ElapsedPerc,int id)
     * @param ElapsedPerc:已完成进度
     * @param id:状态
	 * describe: Send exposure information
	 * 描述：发送曝光信息
	 * calls: send()
	 */
    void AIRCAMERA::ShotRunningSend(int ElapsedPerc,int id)
    {
        Json::Value Root;
        Root["Event"] = Json::Value("ShotRunning");
        Root["ElapsedPerc"] = Json::Value(ElapsedPerc);
        Root["Status"] = Json::Value(id);
        Root["File"] = Json::Value(AIRCAMINFO->LastImageName);
        Root["Expo"] = Json::Value(AIRCAMINFO->Exposure);
        Root["Elapsed"] = Json::Value(AIRCAMINFO->ExposureUsed);
		ws.send(Root.toStyledString());
    }

    /*
	 * name: newJPGReadySend()
	 * describe: Send the message that the picture is ready to the client
	 * 描述：将图片准备就绪的消息传给客户端
	 * calls: send()
     * calls: imread()
	 */
    void AIRCAMERA::newJPGReadySend()
    {
        auto start = std::chrono::high_resolution_clock::now();
        /*组合即将发送的json信息*/
        Json::Value Root;
        Root["Event"] = Json::Value("NewJPGReady");
        Root["UID"] = Json::Value("RemoteCameraShot");
        Root["ActionResultInt"] = Json::Value(5);
        Root["Base64Data"] = Json::Value(IMGINFO->img_data);
        Root["PixelDimX"] = Json::Value(AIRCAMINFO->Image_Width);
        Root["PixelDimY"] = Json::Value(AIRCAMINFO->Image_Height);
        Root["SequenceTarget"] = Json::Value(SequenceTarget);
        Root["Bin"] = Json::Value(AIRCAMINFO->Bin);
        Root["StarIndex"] = Json::Value(IMGINFO->StarIndex);
        Root["HFD"] = Json::Value(IMGINFO->HFD);
        Root["Expo"] = Json::Value(AIRCAMINFO->Exposure);
        Root["TimeInfo"] = Json::Value(timestampW());
        Root["File"] = Json::Value(AIRCAMINFO->LastImageName);
        Root["Filter"] = Json::Value("** BayerMatrix **");
        /*发送信息*/
		ws.send(Root.toStyledString());
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        IDLog(_("Progress image took %g seconds\n"), diff.count());
    }

    void AIRCAMERA::CameraGUI(bool* p_open)
    {
        
    }
}