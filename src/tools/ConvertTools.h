/*
 * ConvertTools.h
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
 
Date:2021-7-19
 
Description:Some small tools

**************************************************/

#ifndef _CONVERT_TOOLS_H_
#define _CONVERT_TOOLS_H_

#include <string>

namespace AstroAir
{
    void RAConvert(std::string Target_RA,int *h,int *m,int *s);
    void DECConvert(std::string Target_DEC,int *h,int *m,int *s);
    
    double DecodeString(const char * data, size_t size, double factor);
    int DecodeString(const char *data, size_t size);
}

#endif