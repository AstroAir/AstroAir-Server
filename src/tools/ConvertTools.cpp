/*
 * ConvertTools.cpp
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

#include "ConvertTools.h"

#include <string.h>

namespace AstroAir
{
    void RAConvert(std::string Target_RA,int *h,int *m,int *s)
    {
        int p_h_ra = Target_RA.find("h");
        int p_m_ra = Target_RA.find("m");
        int p_s_ra = Target_RA.find("s");
        for(int i=0;i < p_h_ra;i++)
        {
            std::string temp;
            temp += Target_RA[i];
            h = reinterpret_cast<int*>(atoi(temp.c_str()));
        }
        for(int i=p_h_ra+1;i < p_m_ra;i++)
        {
            std::string temp;
            temp += Target_RA[i];
            m = reinterpret_cast<int*>(atoi(temp.c_str()));
        }
        for(int i=p_m_ra+1;i < p_s_ra;i++)
        {
            std::string temp;
            temp += Target_RA[i];
            s = reinterpret_cast<int*>(atoi(temp.c_str()));
        }
    }

    void DECConvert(std::string Target_DEC,int *h,int *m,int *s)
    {
        int p_h_dec = Target_DEC.find("h");
        int p_m_dec = Target_DEC.find("m");
        int p_s_dec = Target_DEC.find("s");
        for(int i=0;i < p_h_dec;i++)
        {
            std::string temp;
            temp += Target_DEC[i];
            h = reinterpret_cast<int*>(atoi(temp.c_str()));
        }
        for(int i=p_h_dec+1;i < p_m_dec;i++)
        {
            std::string temp;
            temp += Target_DEC[i];
            m = reinterpret_cast<int*>(atoi(temp.c_str()));
        }
        for(int i=p_m_dec+1;i < p_s_dec;i++)
        {
            std::string temp;
            temp += Target_DEC[i];
            s = reinterpret_cast<int*>(atoi(temp.c_str()));
        }
    }

    double DecodeString(const char * data, size_t size, double factor)
    {
        return DecodeString(data, size) / factor;
    }

    int DecodeString(const char *data, size_t size)
    {
        char str[32] = {0};
        strncpy(str, data, size);
        int iVal = atoi(str);
        return iVal;
    }
}