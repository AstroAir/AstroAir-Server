/*
 * ImgTools.cpp
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

#include "ImgTools.h"

#include <vector>
#include <iomanip>
#include <math.h>
#include <set>
#include <tuple>
#include <list>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

namespace AstroAir
{
    ImageInfo NEW1;
    ImageInfo *IMGINFO = &NEW1;
}

typedef std::tuple<int /*x*/,int /*y*/> PixelPosT;
typedef std::set<PixelPosT> PixelPosSetT;
typedef std::list<PixelPosT> PixelPosListT;

namespace AstroAir::ImageTools
{
    void clacStarInfo(cv::Mat iMat,int outDiameter);

    /*
     * name: ConvertUCto64(unsigned char *imgBuf,bool isColor,int ImageHeight,int ImageWidth)
     * @param imgBuf:图像缓冲区
	 * @param isColor:图像是否为彩色
	 * @param ImageHeight:图像高度
	 * @param ImageWidth:图像宽度
     * describe: Convert unsigned char format to Base64 format
     * 描述： 将Unsigned char格式转化为Base64格式
     * calls: imencode()
     * calls: base64Encode()
     */
    std::string ConvertUCto64(unsigned char *imgBuf,bool isColor,int ImageHeight,int ImageWidth)
    {
        std::vector<int> compression_params;		//图像质量
        std::vector<uchar> vecImg;
		compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);		//JPG图像质量
		compression_params.push_back(100);
		if(isColor == true)
        {
            cv::Mat img(ImageHeight,ImageWidth, CV_8UC3, imgBuf);		//3通道图像信息
            cv::imencode(".jpg", img, vecImg, compression_params);
            clacStarInfo(img,21);
        }
		else
        {
            cv::Mat img(ImageHeight,ImageWidth, CV_8UC1, imgBuf);		//单通道图像信息
            cv::imencode(".jpg", img, vecImg, compression_params);
            clacStarInfo(img,21);
        }
		return base64Encode(vecImg.data(), vecImg.size());
    }

    /*
     * name: clacHistogram(unsigned char *imgBuf,bool isColor,int ImageHeight,int ImageWidth)
     * @param imgBuf:图像缓冲区
	 * @param isColor:图像是否为彩色
	 * @param ImageHeight:图像高度
	 * @param ImageWidth:图像宽度
     * describe: Calculate histogram
     * 描述： 计算直方图
     * calls: calcHist()
     */
	void clacHistogram(unsigned char *imgBuf,bool isColor,int ImageHeight,int ImageWidth)
	{
		cv::MatND dstHist;  
		/*最大值&最小值*/
		double minValue = 0;
		double maxValue = 0;
		float hranges[] = { 0,255 }; //特征空间的取值范围
		if(isColor == true)		//如果是彩色相机
		{
			cv::Mat img(ImageHeight,ImageWidth, CV_8UC3, imgBuf);		//3通道图像信息
			int histSize[3] = {256,256,256};
			const float *ranges[3] = { hranges , hranges , hranges};
			int channels[3] = {0,1,2};
			cv::calcHist(&img, 1, channels, cv::Mat(), dstHist, 3, histSize, ranges);
		}
		else		//默认为黑白相机
		{
			//IDLog("Start calculating histogram of mono image\n");
			cv::Mat img(ImageHeight,ImageWidth, CV_8UC1, imgBuf);		//单通道图像信息
			const float *ranges[] = { hranges };
			int histSize = 256;  //存放每个维度的直方图的尺寸的数组
			int channels = 0;  //通道数
			cv::calcHist(&img, 1, &channels, cv::Mat(), dstHist, 1, &histSize, ranges);
		}
	}

    void clacStarInfo(cv::Mat iMat,int outDiameter)
    {
        /*计算HFD*/
        double out = outDiameter / 2;
        float sum = 0, sumDist = 0;
        int centerX = ceil(iMat.rows / 2.0);
        int centerY = ceil(iMat.cols / 2.0);

        double mean = cv::mean(iMat)[0];

        for(int x = 0 ;x < iMat.rows;x++)
        {
            for(int y = 0;y< iMat.cols;y++)
            {
                //if(iMat.at<float>(x,y) < mean)
                //    iMat.at<float>(x,y) = 0;
                //else
                //    iMat.at<float>(x,y) = iMat.at<float>(x,y) - mean;
                if((pow(x - centerX, 2.0) + pow(y - centerY, 2.0) <= pow(out, 2.0)))
                {
                    sum += iMat.at<float>(x,y);
                    sumDist += iMat.at<float>(x,y) * sqrt(pow((float)x - (float)centerX, 2.0f) + pow((float)y - (float)centerY, 2.0f));
                }
            }
        }
        IMGINFO->HFD = ((float)((int)(((sum ? 2.0 * sumDist / sum : sqrt(2.0) * out)+0.005)*100)))/100;

        /*计算星点数量*/
        std::vector<cv::Vec3f> circles;
        double dp = 2;
        double minDist = 10;  //两个圆心之间的最小距离
        double param1 = 100;  //Canny边缘检测的较大阈值
        double param2 = 100;  //累加器阈值
        int min_radius = 5;  //圆形半径的最小值
        int max_radius = 100; //圆形半径的最大值

        if(iMat.channels() == 3)        //彩色图像需转为灰度图像
        {
            cv::Mat gray;
            cvtColor(iMat, gray, cv::COLOR_BGR2GRAY);
            HoughCircles(gray, circles, cv::HOUGH_GRADIENT, dp, minDist, param1, param2, min_radius, max_radius);
        }
        else
        {
            HoughCircles(iMat, circles, cv::HOUGH_GRADIENT, dp, minDist, param1, param2, min_radius, max_radius);
        }
        IMGINFO->StarIndex = circles.size();
    }

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
		const char EncodeTable[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        //返回值
        std::string strEncode;
        unsigned char Tmp[4]={0};
        int LineLength=0;
        for(int i=0;i<(int)(DataByte / 3);i++)
        {
            Tmp[1] = *Data++;
            Tmp[2] = *Data++;
            Tmp[3] = *Data++;
            strEncode+= EncodeTable[Tmp[1] >> 2];
            strEncode+= EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
            strEncode+= EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
            strEncode+= EncodeTable[Tmp[3] & 0x3F];
            if(LineLength+=4,LineLength==76) {strEncode+="\r\n";LineLength=0;}
        }
        //对剩余数据进行编码
        int Mod=DataByte % 3;
        if(Mod==1)
        {
            Tmp[1] = *Data++;
            strEncode+= EncodeTable[(Tmp[1] & 0xFC) >> 2];
            strEncode+= EncodeTable[((Tmp[1] & 0x03) << 4)];
            strEncode+= "==";
        }
        else if(Mod==2)
        {
            Tmp[1] = *Data++;
            Tmp[2] = *Data++;
            strEncode+= EncodeTable[(Tmp[1] & 0xFC) >> 2];
            strEncode+= EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
            strEncode+= EncodeTable[((Tmp[2] & 0x0F) << 2)];
            strEncode+= "=";
        }
        return strEncode;
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
}