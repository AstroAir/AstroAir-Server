/*
 * air_phd2.cpp
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
 
Date:2021-6-26
 
Description:PD2 offical port

**************************************************/

#include "air_phd2.h"
#include "../../logger.h"
#include "../../wsserver.h"

/*以下三个函数均是用于switch支持string*/
constexpr hash_t hash_compile_time(char const *str, hash_t last_value = basis)
{
    return *str ? hash_compile_time(str + 1, (*str ^ last_value) * prime) : last_value;
}

hash_t hash_(char const *str)
{
    hash_t ret{basis};
    while (*str)
    {
        ret ^= *str;
        ret *= prime;
        str++;
    }
    return ret;
}

constexpr unsigned long long operator"" _hash(char const *p, size_t)
{
    return hash_compile_time(p);
}

namespace AstroAir
{
    PHD2::PHD2()
    {

    }

    PHD2::~PHD2()
    {
        if(isGuideConnected == true)
            Disconnect();
    }

    bool PHD2::Connect(std::string Device_Name)
    {
        if(!PHD2TCP->Connect("127.0.0.1",4400))
        {
            IDLog_Error(_("Coulnd not connect PHD2\n"));
            return false;
        }
        isGuideConnected = true;
        std::thread PHD2InfoThread(&PHD2::ReadFromPHD2,this);
        PHD2InfoThread.detach();
        return true;
    }

    bool PHD2::Disconnect()
    {
        PHD2TCP->Disconnect();
        isGuideConnected = false;
        return true;
    }

    std::string PHD2::ReturnDeviceName()
    {
        return "PHD2";
    }

    bool PHD2::StartGuiding()
    {
        Json::Value Root;
        Root["method"] = Json::Value("guide");
        Root["params"]["settle"]["pixels"] = Json::Value(1.5);
        Root["params"]["settle"]["time"] = Json::Value(8);
        Root["params"]["settle"]["timeout"] = Json::Value(40);
        Root["id"] = Json::Value(1);
        SendToPHD2(Root.toStyledString());
        return true;
    }

    bool PHD2::AbortGuiding()
    {
        Json::Value Root,info;
        Root["method"] = Json::Value("set_paused");
        info["boolen"] = Json::Value(true);
        info["guide"] = Json::Value("full");
        Root["params"].append(info["boolen"]);
        Root["params"].append(info["guide"]);
        SendToPHD2(Root.toStyledString());
        return true;
    }

    bool PHD2::Dither()
    {
        Json::Value Root;
        Root["method"] = Json::Value("dither");
        Root["params"]["amount"] = Json::Value(10);
        Root["params"]["raOnly"] = Json::Value(false);
        Root["params"]["settle"]["pixels"] = Json::Value(1.5);
        Root["params"]["settle"]["time"] = Json::Value(8);
        Root["params"]["settle"]["timeout"] = Json::Value(40);
        SendToPHD2(Root.toStyledString());
        return true;
    }

    /*
    void PHD2::UpdatePHD2Info()
    {
        Json::Value Root;
        std::string temp;
        Root["id"] = Json::Value(1);
        for(int i = 0;i < 5;i++)
        {
            Root["method"] = Json::Value(cmd[i]);
            SendToPHD2(Root.toStyledString());
            temp = ReadFromPHD2();
            ReadJson(temp);
        }
    }
    */
    void PHD2::ReadJson(std::string message)
    {
        std::unique_ptr<Json::CharReader>const json_read(reader.newCharReader());
        json_read->parse(message.c_str(), message.c_str() + message.length(), &root,&errs);
        if(!root["Event"].asString().empty())
        {
            std::string event = root["Event"].asString();
            switch (hash_(event.c_str()))
            {
                case "StartCalibration"_hash:
                {
                    IsCalibrating = true;
                    WebLog(_("Start Cakibration"),2);
                    IDLog(_("Start Calibration\n"));
                    break;
                }
                case "Calibrating"_hash:
                {
                    char temp[1024];
                    sprintf(temp,_("Calibrating:Direction %s , Step %d"),root["dir"].asString().c_str(),root["step"].asInt());
                    std::string tt = temp;
                    IDLog(_("Calibrating:Direction %s , Step %d\n"),root["dir"].asString().c_str(),root["step"].asInt());
                    WebLog(tt,2);
                    break;
                }
                case "CalibrationComplete"_hash:
                {
                    IsCalibrating = false;
                    WebLog(_("Calibration Complete"),2);
                    IDLog(_("Calibration Complete\n"));
                    break;
                }
                case "CalibrationFailed"_hash:
                {
                    IsCalibrating = false;
                    std::string temp = _("Calibration Failed,error: ") + root["Reason"].asString();
                    WebLog(temp,3);
                    IDLog_Error(temp.c_str());
                    break;
                }
                case "StartGuiding"_hash:
                {
                    IsGuiding = true;
                    WebLog(_("Start Guiding"),2);
                    IDLog(_("Start Guiding\n"));
                    break;
                }
                case "GuideStep"_hash:
                {
                    Guide_RA = root["RADistanceRaw"].asDouble();
                    Guide_DEC = root["DECDistanceRaw"].asDouble();
                    break;
                }
                case "GuidingStopped"_hash:
                {
                    IsGuiding = true;
                    WebLog(_("Stop Guiding!"),2);
                    IDLog(_("Stop Guiding\n"));
                    break;
                }
                case "Alert"_hash:
                {
                    if(root["Type"].asString() == "info")
                        IDLog(_("PHD2 info: %s\n"),root["Msg"].asString().c_str());
                    if(root["Type"].asString() == "warning")
                        IDLog_Warning(_("PHD2 Warning info: %s\n"),root["Msg"].asString().c_str());
                    break;
                }
                case "AppState"_hash:
                {
                    if(root["State"].asString() == "Stopped" || root["State"].asString() == "Paused")
                    {
                        IsGuiding = false;
                        IsCalibrating = false;
                    }    
                    if(root["State"].asString() == "Calibrating")
                        IsCalibrating = true;
                    if(root["State"].asString() == "Guiding")
                        IsGuiding = true;
                    break;
                }
                default:
                    break;
            }
        }
        
    }

    void PHD2::SendToPHD2(std::string message)
    {
        PHD2TCP->SendMessage(message.c_str());
    }

    void PHD2::ReadFromPHD2()
    {
        while(isGuideConnected)
        {
            std::string temp;
            temp = PHD2TCP->ReadMessage();
            ReadJson(temp);
        }
    }

    
}