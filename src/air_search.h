/*
 * search.h
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

#ifndef _AIR_SEARCH_H_
#define _AIR_SEARCH_H_

#include "wsserver.h"

namespace AstroAir
{
    class Search
    {
        public:
            /*搜索天体目标*/
            void SearchTarget(std::string TargetName);
            void SearchTargetSuccess(std::string RA,std::string DEC,std::string Name,std::string OtherName,std::string Type,std::string MAG,std::string CONZH);
            void SearchTargetError(int id);

            void RoboClipGetTargetList(std::string FilterGroup,std::string FilterName,std::string FilterNote,int order);
            void RoboClipGetTargetListSuccess();
            void RoboClipGetTargetListError(int errCode);

            void RemoteRoboClipAddTarget(std::string DECJ2000,std::string RAJ2000,int FCOL,int FROW,std::string Group,std::string GuidTarget,bool IsMosaic,std::string Note,std::string PA,std::string TILES,std::string TargetName,bool angleAdj,int overlap);
        private:
            Json::Value root;
			Json::String errs;
			Json::CharReaderBuilder reader;

            struct RoboclipInfo
            {
                std::string DEC[100];
                std::string RA[100];
                int FCOL[100];
                int FROW[100];
                std::string Group[100];
                std::string GuidTarget[100];
                bool IsMosaic[100];
                std::string Note[100];
                std::string PA[100];
                std::string TILES[100];
                std::string TargetName[100];
                bool angleAdj[100];
                int overlap[100];
            };
            RoboclipInfo Info;

            int TargetCount = 0;

            std::string TargetRA_temp,TargetDEC_temp;
            
    };
    extern Search SEARCH;
}

#endif