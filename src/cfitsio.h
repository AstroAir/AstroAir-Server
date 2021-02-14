/*
 * cfitsio.h
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

#include <fitsio.h>
#include <string>

namespace AstroAir::FITSIO
{
    void SaveFitsImage(unsigned char *imgBuf,std::string ImageName,bool isColor,int Image_type,int ImageHeight,int ImageWidth,char *CameraName,const char* CameraBrand);
}