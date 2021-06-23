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
#include "libxls.h"

namespace AstroAir
{
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
        
        /*检查星体数据库是否存在*/
        if(access( "base/starbase.xls", F_OK ) == -1)
        {
            SearchTargetError(2);
            return;
        }
        // 工作簿
        xls::WorkBook base("base/starbase.xls");
        int line = 2;
        while(true)
        {
			xls::cellContent info = base.GetCell(0,line,1);
			if(info.type == xls::cellBlank) 
                break;
            std::string name = info.str;
            /*去除所有空格*/
            if( !name.empty() )
            {
                name.erase(0,name.find_first_not_of(" "));
                name.erase(name.find_last_not_of(" ") + 1);
                int index = 0;
                while( (index = name.find(' ',index)) != std::string::npos)
                    name.erase(index,1);
            }
            /*找到制定目标*/
            if(name == TargetName)
            {
                std::string ra,dec,oname,type,mag;
                ra = base.GetCell(0,line,5).str;
                dec = base.GetCell(0,line,6).str;
                oname = base.GetCell(0,line,2).str;
                type = base.GetCell(0,line,3).str;
                mag = base.GetCell(0,line,7).str;
                SearchTargetSuccess(ra,dec,name,oname,type,mag);
                return;
            }
            line++;
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
        json_message = Root.toStyledString();
		ws.send(json_message);
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
        json_message = Root.toStyledString();
		ws.send(json_message);
    }
}