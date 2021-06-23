/*
 * base64.cpp
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
 
Description:Base64 Library
 
**************************************************/

#include "base64.h"

namespace AstroAir
{
    /*
     * name: base64Encode(const unsigned char* Data, int DataByte) 
     * @param Data:图像缓冲区
	 * @param DataByte:编码参数
     * describe: Base64 encoding
     * 描述： Base64编码
     * @return strEncode:已编码的图像
     */
	std::string base64Encode(const unsigned char *Data, int DataByte)
	{
		//编码表
		const char EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		//返回值
		std::string strEncode;
		unsigned char Tmp[4] = {0};
		int LineLength = 0;
		for (int i = 0; i < (int)(DataByte / 3); i++)
		{
			Tmp[1] = *Data++;
			Tmp[2] = *Data++;
			Tmp[3] = *Data++;
			strEncode += EncodeTable[Tmp[1] >> 2];
			strEncode += EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
			strEncode += EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
			strEncode += EncodeTable[Tmp[3] & 0x3F];
			if (LineLength += 4, LineLength == 76)
			{
				strEncode += "\r\n";
				LineLength = 0;
			}
		}
		//对剩余数据进行编码
		int Mod = DataByte % 3;
		if (Mod == 1)
		{
			Tmp[1] = *Data++;
			strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
			strEncode += EncodeTable[((Tmp[1] & 0x03) << 4)];
			strEncode += "==";
		}
		else if (Mod == 2)
		{
			Tmp[1] = *Data++;
			Tmp[2] = *Data++;
			strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
			strEncode += EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
			strEncode += EncodeTable[((Tmp[2] & 0x0F) << 2)];
			strEncode += "=";
		}
		return strEncode;
	}

	/*
     * name: Mat2Base64(const cv::Mat &img, std::string imgType) 
     * @param img:图像缓冲区
	 * @param imgType:图像格式，包括png bmp jpg jpeg等opencv能够进行编码解码的文件
     * describe: Convert mat format to Base64 format
     * 描述： 将Mat格式转化为Base64格式
     * @return img_data:已编码的图像
     */
	std::string Mat2Base64(const cv::Mat &img, std::string imgType) 
	{
		//Mat转base64
		std::string img_data;
		std::vector<uchar> vecImg;
		std::vector<int> compression_params;
		compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
		compression_params.push_back(100);
		imgType = "." + imgType;
		cv::imencode(imgType, img, vecImg, compression_params);
		img_data = base64Encode(vecImg.data(), vecImg.size());
		return img_data;
	}

    /*
     * name: base64Decode(const char* Data, int DataByte)
     * @param Data:图像缓冲区
	 * @param DataByte:编码参数
     * describe: Base64 decoding
     * 描述： Base64解码
     * @return strDecode:已解码的图像
     */
    std::string base64Decode(const char* Data, int DataByte)
    {
        //解码表
        const char DecodeTable[] =
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            62, // '+'
            0, 0, 0,
            63, // '/'
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
            0, 0, 0, 0, 0, 0, 0,
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
            13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
            0, 0, 0, 0, 0, 0,
            26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
            39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
        };
        std::string strDecode;
        int nValue;
        int i = 0;
        while (i < DataByte)
        {
            if (*Data != '\r' && *Data != '\n')
            {
                nValue = DecodeTable[*Data++] << 18;
                nValue += DecodeTable[*Data++] << 12;
                strDecode += (nValue & 0x00FF0000) >> 16;
                if (*Data != '=')
                {
                    nValue += DecodeTable[*Data++] << 6;
                    strDecode += (nValue & 0x0000FF00) >> 8;
                    if (*Data != '=')
                    {
                        nValue += DecodeTable[*Data++];
                        strDecode += nValue & 0x000000FF;
                    }
                }
                i += 4;
            }
            else
            {
                Data++;
                i++;
            }
        }
        return strDecode;
    }

    /*
     * name: Base2Mat(std::string &base64_data) 
     * @param base64_data:Base64编码
     * describe: Convert Base64 format to Mat format
     * 描述： 将Base64格式转化为Mat格式
     * @return img_data:已编码的图像
     */
    cv::Mat Base2Mat(std::string &base64_data) 
    {
        cv::Mat img;
        std::string s_mat;
        s_mat = base64Decode(base64_data.data(), base64_data.size());
        std::vector<char> base64_img(s_mat.begin(), s_mat.end());
        img = cv::imdecode(base64_img, cv::IMREAD_COLOR);
        return img;
    }
}