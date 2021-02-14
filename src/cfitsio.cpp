/*
 * cfitsio.cpp
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
 
Date:2021-2-14
 
Description:CFitIO Library
 
**************************************************/

#include "cfitsio.h"
#include "logger.h"

#include <cstring>

namespace AstroAir::FITSIO
{
    void SaveFitsImage(unsigned char *imgBuf,std::string ImageName,bool isColor,int Image_type,int ImageHeight,int ImageWidth,char *CameraName,const char* CameraBrand)
    {
		char datatype[40];		//相机品牌
        char keywords[40];		//相机品牌
		char value[20];		//相机名称
		char description[40];		//相机描述
        strcpy(datatype, "TSTRING");
        strcpy(keywords, "Camera");
		strcpy(value,CameraName);
		strcpy(description,CameraBrand);

        fitsfile *fptr;		//cFitsIO定义
        int FitsStatus;		//cFitsio状态
		long naxes[2] = {ImageWidth,ImageHeight};
        long naxis = 2;
        long nelements;
		long fpixel = 1;
        long imgSize = ImageWidth*ImageHeight*(1 + (Image_type==1));

		fits_create_file(&fptr, ImageName.c_str(), &FitsStatus);		//创建Fits文件
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
    }
}
