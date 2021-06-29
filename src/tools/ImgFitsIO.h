/*
 * ImgFitsIO.h
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
 
Description:FitsIO Port
 
**************************************************/

#ifndef _IMG_FITSIO_H_
#define _IMG_FITSIO_H_

#include <fitsio.h>

namespace AstroAir::FitsIO
{
    bool SaveFitsImage(unsigned char *imgBuf,const char * ImageName,int Image_Type,bool isColor,int ImageHeight,int ImageWidth,const char* CameraName,int Expo,int Bin,int Offset,int Gain,double Temp);
    void AddImageKeywords(fitsfile * fptr,const char* ImageName,int ImageHeight,int ImageWidth,const char* CameraName,int Expo,int Bin,int Offset,int Gain,double Temp);
    void FitsImageError(fitsfile * fptr);
}

#endif