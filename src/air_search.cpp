/*
 * search.cpp
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
 
Date:2021-6-23
 
Description:Object search engine

**************************************************/

#include "air_search.h"
#include "logger.h"
#include "wsserver.h"

namespace AstroAir
{
    Search SEARCH;

    /*
	 * name: SearchTarget(std::string TargetName)
     * @param TargerName:目标名称
	 * describe: Search for celestial bodies
	 * 描述：搜索天体
	 * calls: SearchTargetSuccess()
     * calls: SearchTargetError()
	 */
    void Search::SearchTarget(std::string TargetName)
    {
        if(TargetName[0] == 'M' || TargetName[0] == 'm')
        {
            /*检查星体数据库是否存在*/
            if(access( "StarBase/Messier.json", F_OK ) == -1)
            {
                SearchTargetError(2);
                return;
            }
            std::string line,jsonStr;
            std::ifstream in("StarBase/Messier.json", std::ios::binary);
            if (!in.is_open())
            {
                IDLog(_("Unable to open star file\n"));
                return ;
            }
            while (getline(in, line))
                jsonStr.append(line);
            in.close();
            std::unique_ptr<Json::CharReader>const json_read(reader.newCharReader());
            json_read->parse(jsonStr.c_str(), jsonStr.c_str() + jsonStr.length(), &root,&errs);
            if(!root[TargetName].empty())
            {   
                std::string ra,dec,oname,type,mag;
                ra = root[TargetName]["RAJ2000"].asString();
                dec = root[TargetName]["DECJ2000"].asString();
                oname = root[TargetName]["NGC"].asString();
                type = root[TargetName]["TYPE"].asString();
                mag = root[TargetName]["VMAG"].asString();
                SearchTargetSuccess(ra,dec,TargetName,oname,type,mag);
                return ;
            }
        }
        else
        {
            if(access( "StarBase/NGCIC.json", F_OK ) == -1)
            {
                SearchTargetError(2);
                return;
            }
            std::string line,jsonStr;
            std::ifstream in("StarBase/NGCIC.json", std::ios::binary);
            if (!in.is_open())
            {
                IDLog(_("Unable to open star file\n"));
                return ;
            }
            while (getline(in, line))
                jsonStr.append(line);
            in.close();
            std::unique_ptr<Json::CharReader>const json_read(reader.newCharReader());
            json_read->parse(jsonStr.c_str(), jsonStr.c_str() + jsonStr.length(), &root,&errs);
            if(!root[TargetName].empty())
            {   
                
                std::string ra,dec,oname,type,mag;
                if(root[TargetName]["NAMEZH"].empty())
                    oname = "None";
                else
                    oname = root[TargetName]["NGC"].asString();
                ra = root[TargetName]["RAJ2000"].asString();
                dec = root[TargetName]["DECJ2000"].asString();
                type = root[TargetName]["TYPE"].asString();
                mag = root[TargetName]["VMAG"].asString();
                SearchTargetSuccess(ra,dec,TargetName,oname,type,mag);
                return ;
            }
        }
        /*未找到制定目标*/
        SearchTargetError(0);
        return;
    }

    /*
	 * name: SearchTargetSuccess(std::string RA,std::string DEC,std::string Name,std::string OtherName,std::string Type)
     * @param RA:天体RA轴坐标
     * @param DEC:天体DEC轴坐标
     * @param Name:天体名称
     * @param OtherName:天体别名
     * @param Type:天体类型
	 * describe: Search for celestial bodies successfully
	 * 描述：搜索天体成功
	 * calls: send()
	 */
    void Search::SearchTargetSuccess(std::string RA,std::string DEC,std::string Name,std::string OtherName,std::string Type,std::string MAG)
    {
        Json::Value Root,info;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteSearchTarget");
        Root["ActionResultInt"] = Json::Value(4);
        Root["ParamRet"]["Result"] = Json::Value(1);
        Root["ParamRet"]["Name"] = Json::Value(Name);
        /*天体坐标信息*/
        //TargetRA = RA;
        //TargetDEC = DEC;
        Root["ParamRet"]["RAJ2000"] = Json::Value(RA);
        Root["ParamRet"]["DECJ2000"] = Json::Value(DEC);
        /*天体基础信息*/
        info["Key"] = Json::Value("别称");     //天体别名
        info["Value"] = Json::Value(OtherName);
        Root["ParamRet"]["Info"].append(info);
        info["Key"] = Json::Value("类型");      //天体类型
        info["Value"] = Json::Value(Type);
        Root["ParamRet"]["Info"].append(info);
        info["Key"] = Json::Value("星等");      //天体星等
        info["Value"] = Json::Value(MAG);
        Root["ParamRet"]["Info"].append(info);
		ws.send(Root.toStyledString());
    }

    /*
	 * name: SearchTargetError(int id)
     * @param id:错误信息ID
	 * describe: Search for celestial bodies error
	 * 描述：搜索天体失败
	 * calls: send()
	 */
    void Search::SearchTargetError(int id)
    {
        Json::Value Root;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteSearchTarget");
        if(id == 0)     //目标未找到
        {
            Root["ActionResultInt"] = Json::Value(4);
            Root["ParamRet"]["Result"] = Json::Value(0);
        }
        else        //星体数据库无法打开
        {
            Root["ActionResultInt"] = Json::Value(5);
            Root["Motivo"] = Json::Value("Could not open star base!");
        }
		ws.send(Root.toStyledString());
    }

    /*
	 * name: RoboClipGetTargetList(std::string FilterGroup,std::string FilterName,std::string FilterNote,int order)
     * @param FilterGroup:过滤组
     * @param FilterName:过滤名称
     * @param FilterNote:过滤提示
     * @param order:?
	 * describe: Celestial object manager
	 * 描述：天体目标管理器
	 * calls: RoboClipGetTargetListSuccess()
     * calls: RoboClipGetTargetListError(int errCode)
	 */
    void Search::RoboClipGetTargetList(std::string FilterGroup,std::string FilterName,std::string FilterNote,int order)
    {
        //此处需加入多文件支持
        if(access( "Roboclip/roboclip.json", F_OK ) == -1)
        {
            RoboClipGetTargetListError(5);
            return;
        }
        std::ifstream in("Roboclip/roboclip.json", std::ios::binary);
        if (!in.is_open())
        {
            IDLog(_("Unable to open roboclip file\n"));
            return;
        }
        std::string line, jsonStr;
        while (getline(in, line))
            jsonStr.append(line);
        /*判断文件是否为空*/
        in.seekg(0, std::ios_base::end);
        std::fstream::off_type Len = in.tellg();
        in.close();
        Json::Value Root;
        if (Len != 0)
        {
            std::unique_ptr<Json::CharReader> const json_read(reader.newCharReader());
            json_read->parse(jsonStr.c_str(), jsonStr.c_str() + jsonStr.length(), &root, &errs);
            for(int i = 0;i < root["target"].size(); i++)
            {
                Info.DEC[i] = root["target"][i]["DECJ2000"].asString();
                Info.RA[i] = root["target"][i]["RAJ2000"].asString();
                Info.FCOL[i] = root["target"][i]["FCOL"].asInt();
                Info.FROW[i] = root["target"][i]["FROW"].asInt();
                Info.Group[i] = root["target"][i]["Group"].asString();
                Info.GuidTarget[i] = root["target"][i]["GuidTarget"].asString();
                Info.IsMosaic[i] = root["target"][i]["IsMosaic"].asBool();
                Info.Note[i] = root["target"][i]["Note"].asString();
                Info.overlap[i] = root["target"][i]["overlap"].asInt();
                Info.PA[i] = root["target"][i]["PA"].asString();
                Info.TargetName[i] = root["target"][i]["TargetName"].asString();
                Info.TILES[i] = root["target"][i]["TILES"].asString();
                Info.angleAdj[i] = root["target"][i]["angleAdj"].asBool();
            }
            TargetCount = root["target"].size();
            if(TargetCount == 0)
            {
                IDLog(_("There is no target in the file\n"));
                RoboClipGetTargetListError(4);
            }
            else
            {
                IDLog(_("Found %d targets in the roboclip.json\n"),TargetCount);
                IDLog(_("Get target list and send to client\n"));
                RoboClipGetTargetListSuccess();
            }
                
        }
        else
        {
            IDLog(_("roboclip.json is an empty file\n"));
            RoboClipGetTargetListError(4);
            in.close();
            return ;
        }
        in.close();
        
    }

    void Search::RoboClipGetTargetListSuccess()
    {
        Json::Value Root,info;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteRoboClipGetTargetList");
        Root["ActionResultInt"] = Json::Value(4);
        int i = 0;
        while(i < TargetCount)
        {
            info["targetname"] = Json::Value(Info.TargetName[i]);
            info["raj2000"] = Json::Value(Info.RA[i]);
            info["decj2000"] = Json::Value(Info.DEC[i]);
            info["frow"] = Json::Value(Info.FROW[i]);
            info["fcol"] = Json::Value(Info.FCOL[i]);
            info["tiles"] = Json::Value(Info.TILES[i]);
            info["pa"] = Json::Value(Info.PA[i]);
            info["note"] = Json::Value(Info.Note[i]);
            info["guid"] = Json::Value(Info.GuidTarget[i]);
            info["gruppo"] = Json::Value(Info.Group[i]);
            Root["ParamRet"]["list"].append(info);
            i++;
        }
        ws.send(Root.toStyledString());
    }

    void Search::RoboClipGetTargetListError(int errCode)
    {
        Json::Value Root;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteRoboClipGetTargetList");
        Root["ActionResultInt"] = Json::Value(5);
        if(errCode == 5)
            Root["Motivo"] = Json::Value("Not Found File!");
        else
            Root["Motivo"] = Json::Value("Empty File!");
        ws.send(Root.toStyledString());
    }

    void Search::RemoteRoboClipAddTarget(std::string DECJ2000,std::string RAJ2000,int FCOL,int FROW,std::string Group,std::string GuidTarget,bool IsMosaic,std::string Note,std::string PA,std::string TILES,std::string TargetName,bool angleAdj,int overlap)
    {

    }
}