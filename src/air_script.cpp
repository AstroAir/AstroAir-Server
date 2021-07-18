/*
 * air_script.cpp
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
 
Date:2021-6-24
 
Description:Script port

**************************************************/

#include "air_script.h"
#include "logger.h"
#include "wsserver.h"

#include "air_camera.h"
#include "air_focus.h"
#include "air_solver.h"
#include "air_mount.h"
#include "air_filter.h"

#include <thread>
#include <stdlib.h>

#include <yaml-cpp/yaml.h>

#define ScriptsVersion 1.0

namespace AstroAir
{
    Json::Value root;
	Json::String errs;
	Json::CharReaderBuilder reader;
    AIRSCRIPT *SCRIPT;

    AIRSCRIPT::AIRSCRIPT()
    {
        InSequenceRun = false;          //序列拍摄状态
    }

    AIRSCRIPT::~AIRSCRIPT()
    {

    }

    /*
     * name:GetListAvalaibleSequence()
     * describe: Search all available shot sequences in the folder
     * 描述：搜索文件夹中所有可用的拍摄序列
     * calls: send()
     */
    void AIRSCRIPT::GetListAvalaibleSequence()
    {
        /*寻找当前文件夹下的所有文件*/
        struct dirent *ptr;      
		DIR *dir;  
		std::string PATH = "./Seq";        //搜索目录，正式版本应该可以选择目录位置
		dir=opendir(PATH.c_str());      //打开目录
        std::vector<std::string> files;
        /*搜索所有符合条件的文件*/
		while((ptr = readdir(dir)) != NULL)  
		{  
			if(ptr->d_name[0] == '.' || strcmp(ptr->d_name,"..") == 0)  
				continue;
            /*判断文件后缀是否为.air*/
            int size = strlen(ptr->d_name);
            if(strcmp( ( ptr->d_name + (size - 4) ) , ".air") != 0)
                continue;
            files.push_back(ptr->d_name);
		}  
		closedir(dir);      //关闭目录
        /*判断是否找到配置文件*/
        Json::Value Root,profile;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteGetListAvalaibleSequence");
        if(files.begin() == files.end())
        {
            IDLog_Error(_("Cound not found any avalaible sequence files,please check it\n"));
            Root["ActionResultInt"] = Json::Value(5);
            Root["Motivo"] = Json::Value(_("Cound not found any avalaible sequence files"));
        }
        else
        {
            Root["ActionResultInt"] = Json::Value(4);
            for (int i = 0; i < files.size(); i++)  
            {
                Root["ParamRet"]["list"].append(files[i]);
                IDLog(_("Found avalaible sequence file named %s\n"),files[i].c_str());
            }
        }
        /*整合信息并发送至客户端*/
        ws.send(Root.toStyledString());
    }

    /*
	 * name: RunSequence(std::string SequenceFile)
     * @param SequenceFile:序列文件
	 * describe: Start shooting sequence
	 * 描述：启动拍摄序列
     * calls: IDLog()
     * calls: IDLog_DEBUG()
     * calls: WebLog()
     * calls: RunSequenceError()
	 * calls: Goto()
     * calls: MoveTo()
     * calls: FilterMoveTo()
	 */
    void AIRSCRIPT::RunSequence(std::string SequenceFile)
    {
        std::string a = "Seq/"+SequenceFile;
        if(access(a.c_str(), F_OK ) == -1)
        {
            RunSequenceError(_("Could not found file"));
            return;
        }
        std::string line,jsonStr;
        std::ifstream in(a.c_str(), std::ios::binary);
        Json::Value Root;
        /*打开文件*/
        if (!in.is_open())
        {
            IDLog_Error(_("Unable to open sequence file\n"));
            IDLog_DEBUG(_("Unable to open sequence file\n"));
            return;
        }
        /*将文件转化为string格式*/
        while (getline(in, line))
        {
            jsonStr.append(line);
        }
        /*关闭文件*/
        in.close();
        /*将读取出的json数组转化为string*/
        std::unique_ptr<Json::CharReader>const json_read(reader.newCharReader());
        json_read->parse(jsonStr.c_str(), jsonStr.c_str() + jsonStr.length(), &Root,&errs);
        SequenceTarget = Root["TargetName"].asString();
        /*赤道仪运动到指定位置*/
        if(!Root["Mount"]["MountName"].asString().empty() && Root["Mount"]["MountName"] == MOUNT->ReturnDeviceName())
        {
            if(AIRMOUNTINFO->isMountConnected)
            {
                TargetRA = Root["Mount"]["TargetRA"].asString();
                TargetDEC = Root["Mount"]["TargetDEC"].asString();
                if(MOUNT->Goto(Root["Mount"]["TargetRA"].asString(),Root["Mount"]["TargetDEC"].asString()) == true)
                {
                    IDLog("The equator successfully moved to the designated position\n");
                    WebLog("赤道仪运动到指定位置 RA:"+TargetRA+" DEC:"+TargetDEC,2);
                }
                else        //赤道仪无法运动到指定位置
                {
                    IDLog_Error("The equator could not moved to the designated position\n");
                    WebLog("赤道仪无法运动到指定位置 RA:"+TargetRA+" DEC:"+TargetDEC,3);
                    RunSequenceError("赤道仪无法运动到指定位置");
                    return;
                }
            }
        }
        if(!Root["Filter"]["FilterName"].asString().empty() && Root["Filter"]["FilterName"] == FILTER->ReturnDeviceName())
        {
            if(isFilterConnected)
            {
                if(FILTER->FilterMoveTo(Root["Filter"]["TargetPosition"].asInt()))
                {
                    IDLog("%s moves to the specified focusing position\n",Root["Filter"]["FilterName"].asString().c_str());
                    WebLog(Root["Filter"]["FilterName"].asString() +" 成功运动到对焦位置",2);
                }
                else        //滤镜轮无法运动到指定位置
                {
                    IDLog_Error("The equator could not moved to the designated position\n");
                    WebLog(Root["Filter"]["FilterName"].asString() +" 无法运动到指定位置 " + Root["Filter"]["TargetPosition"].asString(),3);
                    RunSequenceError("滤镜轮无法运动到指定位置");
                    return;
                }
            }
            else
            {
                IDLog_Error(_("Filter has not connected\n"));
                WebLog(_("Filter not connnected,please reconnect!"),3);
                return;
            }
        }
        if(!Root["Focus"]["FocusName"].asString().empty() && Root["Focus"]["FocusName"] == FOCUS->ReturnDeviceName())
        {
            if(isFocusConnected)
            {
                if(FOCUS->MoveTo(Root["Focus"]["TargetPosition"].asInt()))
                {
                    IDLog("%s moves to the specified focusing position\n",Root["Focus"]["FocusName"].asString().c_str());
                    WebLog(Root["Focus"]["FocusName"].asString()+" 成功运动到对焦位置",2);
                }
                else        //电动调焦座无法运动到指定位置
                {
                    IDLog_Error("The equator could not moved to the designated position\n");
                    WebLog(Root["Focus"]["FocusName"].asString()+" 无法运动到指定位置: " + Root["Focus"]["TargetPosition"].asString(),3);
                    RunSequenceError("电动调焦座无法运动到指定位置");
                    return;
                }
            }
            {
                IDLog_Error(_("Focus has not connected\n"));
                WebLog(_("Focus not connnected,please reconnect!"),3);
                return;
            }
        }
        if(!Root["Camera"]["CameraName"].asString().empty() && Root["Camera"]["CameraName"] == CCD->ReturnDeviceName())
        {
            if(AIRCAMINFO->isCameraConnected)
            {
                InSequenceRun = true;
                SequenceImageName = "Image_"+SequenceTarget+"_"+timestamp();
                std::thread RunSequenceCamera(&AIRCAMERA::StartExposureSeq,CCD,Root["Camera"]["Loop"].asInt(),Root["Camera"]["Expo"].asInt(),Root["Camera"]["Bin"].asInt(),Root["Camera"]["SaveImage"].asBool(),SequenceImageName,Root["Camera"]["Gain"].asInt(),Root["Camera"]["Offset"].asInt());
                RunSequenceCamera.detach();
                WebLog("Start sequence capture",2);
                SS->thread_num--;
            }
            else
            {
                IDLog_Error(_("Camera not connected,how do you exposure?\n"));
                WebLog(_("Camera not connected,how do you exposure?"),3);
                return;
            }
        }
        else
        {
            RunSequenceError(_("There is no camera been selected"));
            WebLog(_("未指定相机，无法进行计划拍摄"),3);
            SS->thread_num--;
            return;
        }
    }

    void AIRSCRIPT::RunSequenceError(std::string error)
    {
        Json::Value Root;
        Root["error"]["message"] = Json::Value(error);
        Root["id"] = Json::Value(100);
        ws.send(Root.toStyledString());
    }

//----------------------------------------脚本----------------------------------------

    /*
     * name:GetListAvalaibleDragScript()
     * describe: Search all available shot drag script in the folder
     * 描述：搜索文件夹中所有可用的拍摄脚本
     * calls: send()
     */
    void AIRSCRIPT::GetListAvalaibleDragScript()
    {
        /*寻找当前文件夹下的所有文件*/
        struct dirent *ptr;      
		DIR *dir;  
		std::string PATH = "./DS";        //搜索目录，正式版本应该可以选择目录位置
		dir=opendir(PATH.c_str());      //打开目录
        std::vector<std::string> files;
        /*搜索所有符合条件的文件*/
		while((ptr = readdir(dir)) != NULL)  
		{  
			if(ptr->d_name[0] == '.' || strcmp(ptr->d_name,"..") == 0)  
				continue;
            /*判断文件后缀是否为.air*/
            int size = strlen(ptr->d_name);
            if(strcmp( ( ptr->d_name + (size - 4) ) , ".yml") != 0)
                continue;
            files.push_back(ptr->d_name);
		}  
		closedir(dir);      //关闭目录
        /*判断是否找到配置文件*/
        Json::Value Root,profile;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteGetListAvalaibleDragScript");
        if(files.begin() == files.end())
        {
            IDLog_Error(_("Cound not found any avalaible drag script,please check it\n"));
            Root["ActionResultInt"] = Json::Value(5);
            Root["Motivo"] = Json::Value(_("Cound not found any avalaible drag script files"));
        }
        else
        {
            Root["ActionResultInt"] = Json::Value(4);
            for (int i = 0; i < files.size(); i++)  
            {
                Root["ParamRet"]["list"].append(files[i]);
                IDLog(_("Found avalaible drag script file named %s\n"),files[i].c_str());
            }
        }
        /*整合信息并发送至客户端*/
        ws.send(Root.toStyledString());
    }

    constexpr std::uint32_t hash_str_to_uint32(const char* data)
    {
        std::uint32_t h(0);
        for (int i = 0; data && ('\0' != data[i]); i++)
            h = (h << 6) ^ (h >> 26) ^ data[i];
        return h;
    }

    /*
	 * name: RemoteDragScript(std::string DragScript)
     * @param DragScript:脚本文件
	 * describe: Start DragScript
	 * 描述：启动脚本（默认格式为标准的shell）
     * calls: IDLog()
     * calls: WebLog()
     * calls: RunSequenceError()
	 * calls: Goto()
     * calls: MoveTo()
     * calls: FilterMoveTo()
	 */
    void AIRSCRIPT::RemoteDragScript(std::string DragScript)
    {
        if(access(("DS/" + DragScript).c_str(), F_OK ) == -1)
        {
            RunSequenceError(_("Could not found file"));
            return;
        }
        YAML::Node script = YAML::LoadFile("DS/" + DragScript);
        if(script["Version"].as<double>() == ScriptsVersion)
        {
            Scripts.sudo_r = script["Sudo"].as<bool>();
            Scripts.shell_r = script["Shell"].as<bool>();
            /*提前打开所需要的软件，使用非阻塞线程*/
            for(YAML::const_iterator it= script["jobs"]["apps"].begin(); it != script["jobs"]["apps"].end();++it)
            {
                switch (hash_str_to_uint32(it->first.as<std::string>().c_str()))
                {
                    case hash_str_to_uint32("PHD2"):
                        system("sudo phd2.bin &");
                        break;
                    case hash_str_to_uint32("Kstars"):
                        system("sudo kstars &");
                        break;
                    case hash_str_to_uint32("INDIWebManager"):
                        system("sudo indi-web -v &");
                        break;
                    default:
                        IDLog(_("No app needs to load\n"));
                        break;
                }
            }
            for(YAML::const_iterator it= script["jobs"]["steps"].begin(); it != script["jobs"]["steps"].end();++it)
            {
                /*执行脚本*/
                switch (hash_str_to_uint32(it->first.as<std::string>().c_str()))
                {
                    /*相机拍摄*/
                    case hash_str_to_uint32("shot"):{
                        std::string s = it->second.as<std::string>();
                        Scripts.Enable = true;
                        DS_Shot(script[s]["Type"].as<std::string>(),script[s]["Loop"].as<int>(),script["Exposure"].as<int>(),script["Bin"].as<int>(),script["Gain"].as<int>(),script["Offset"].as<int>());
                        break;
                    }
                    /*赤道仪Goto*/
                    case hash_str_to_uint32("goto"):{
                        std::string s = it->second.as<std::string>();
                        Scripts.Enable = true;
                        DS_Goto(script[s]["RA"].as<std::string>(),script[s]["DEC"].as<std::string>());
                        break;
                    }
                    /*电动调焦座对焦*/
                    case hash_str_to_uint32("focuser"):{
                        std::string s = it->second.as<std::string>();
                        Scripts.Enable = true;
                        DS_Move(script[s]["Position"].as<int>());
                        break;
                    }
                    /*滤镜轮转到指定位置*/
                    case hash_str_to_uint32("filter"):{
                        std::string s = it->second.as<std::string>();
                        Scripts.Enable = true;
                        DS_FilterMoveTo(script[s]["Position"].as<int>());
                        break;
                    }
                    /*解析器解析*/
                    case hash_str_to_uint32("solver"):{
                        std::string s = it->second.as<std::string>();
                        Scripts.Enable = true;
                        DS_Solve(script[s]["Downsample"].as<int>());
                        break;
                    }
                    /*开始导星*/
                    case hash_str_to_uint32("guide"):{
                        std::string s = it->second.as<std::string>();
                        Scripts.Enable = true;
                        DS_Guide();
                        break;
                    }
                    /*运行命令（阻塞）*/
                    case hash_str_to_uint32("command"):{
                        if(Scripts.shell_r)
                        {
                            std::string s = it->second.as<std::string>();
                            if(Scripts.sudo_r)
                                s = "sudo " + s;
                            IDLog(_("Run command: %s"),s.c_str());
                            WebLog(_(s.c_str()),2);
                            system(s.c_str());
                        }
                        else
                        {
                            IDLog_Error(_("Unable to execute system command: not allowed\n"));
                            WebLog(_("Unable to execute system command: not allowed"),3);
                        }
                        break;
                    }
                    /*等待*/
                    case hash_str_to_uint32("sleep"):{
                        sleep(it->second.as<int>());
                        break;
                    }
                    /*重启系统*/
                    case hash_str_to_uint32("reboot"):{
                        WebLog(_("System is rebooting ..."),2);
                        system("sudo reboot");
                        break;
                    }
                    /*关机*/
                    case hash_str_to_uint32("shutdown"):{
                        WebLog(_("The system is shutting down ..."),2);
                        system("sudo shutdown");
                        break;
                    }
                    /*无任务被发现*/
                    default:{
                        IDLog_Error(_("No tasks were found\n"));
                        WebLog(_("No tasks were found"),3);
                        break;
                    }
                }
            }
        }
        else
        {
            IDLog_Error(_("Scripts version mismatch\n"));
            WebLog(_("Scripts version mismatch"),3);
        }
    }

    void AIRSCRIPT::DS_Shot(std::string type,int loop,int exp,int bin,int Gain,int Offset)
    {
        for(int i = 0 ;i<loop;i++)
        {
            if(Scripts.Enable)
                CCD->StartExposure(exp,bin,true,Scripts.SequenceImageName,Gain,Offset);
        }
    }

    void AIRSCRIPT::DS_Goto(std::string RA,std::string DEC)
    {
        if(Scripts.Enable)
            MOUNT->Goto(RA,DEC);
    }

    void AIRSCRIPT::DS_Move(int TargetPosition)
    {
        if(Scripts.Enable)
            FOCUS->MoveTo(TargetPosition);
    }

    void AIRSCRIPT::DS_FilterMoveTo(int TargetPosition)
    {
        if(Scripts.Enable)
            FILTER->FilterMoveTo(TargetPosition);
    }

    void AIRSCRIPT::DS_Solve(int downsample)
    {
        if(Scripts.Enable)
            SOLVER->SolveActualPosition(true,true,downsample);
    }

    void AIRSCRIPT::DS_Guide()
    {
        
    }
}