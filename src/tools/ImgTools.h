/*
 * ImgTools.h
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
 
Date:2021-6-28
 
Description:Image Progress Tools
 
**************************************************/

#ifndef _IMG_TOOLS_H_
#define _IMG_TOOLS_H_

#include <string>

namespace AstroAir::ImageTools
{
    /*格式转化*/
    std::string ConvertUCto64(unsigned char *imgBuf,bool isColor,int ImageHeight,int ImageWidth);       /*转为Base64格式*/
    std::string base64Encode(const unsigned char* Data, int DataByte);      /*Base64编码*/
    std::string base64Decode(const char* Data, int DataByte);               /*Base64解码*/
    
    /*图像信息处理*/
    void clacHistogram(unsigned char *imgBuf,bool isColor,int ImageHeight,int ImageWidth);      /*计算直方图*/
}

#endif