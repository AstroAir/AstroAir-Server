/*
 * opencv.cpp
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
 
Description:OPENCV Library
 
**************************************************/

#include <vector>
#include <string.h>
#include <fstream>

#include "logger.h"
#include "opencv.h"
#include "base64.h"

namespace AstroAir::OPENCV
{
	/*
     * name: SaveImage(unsigned char *imgBuf,std::string ImageName,bool isColor,int ImageHeight,int ImageWidth)
     * @param imgBuf:图像缓冲区
	 * @param ImageName:保存图像名称
	 * @param isColor:图像是否为彩色
	 * @param ImageHeight:图像高度
	 * @param ImageWidth:图像宽度
     * describe: Save JPG Image
     * 描述： 保存JPG图像
     * calls: imwrite()
     * calls: IDLog()
     * note: The default quality of JPG image is 100
     */
	std::string SaveImage(unsigned char *imgBuf,std::string ImageName,bool isColor,int ImageHeight,int ImageWidth)
	{
		std::vector<int> compression_params;		//图像质量
		std::string imgdate;
		compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);		//JPG图像质量
		compression_params.push_back(100);
		const char* JPGName = strtok(const_cast<char *>(ImageName.c_str()),".");
		strcat(const_cast<char *>(JPGName), ".jpg");
		if(isColor == true)
		{
			cv::Mat img(ImageHeight,ImageWidth, CV_8UC3, imgBuf);		//3通道图像信息
			imwrite(JPGName,img, compression_params);
			imgdate = Mat2Base64(img,"jpg");
		}
		else
		{
			cv::Mat img(ImageHeight,ImageWidth, CV_8UC1, imgBuf);		//单通道图像信息
			imwrite(JPGName,img, compression_params);		//写入文件
			imgdate = Mat2Base64(img,"jpg");
		}
		IDLog("JPG image saved successfully\n");
		return imgdate;
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
     * calls: IDLog()
     * note: The result of histogram calculation will be output to a file
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
			IDLog("Start calculating color image histogram\n");
			cv::Mat img(ImageHeight,ImageWidth, CV_8UC3, imgBuf);		//3通道图像信息
			int histSize[3] = {256,256,256};
			const float *ranges[3] = { hranges , hranges , hranges};
			int channels[3] = {0,1,2};
			cv::calcHist(&img, 1, channels, cv::Mat(), dstHist, 3, histSize, ranges);
			IDLog("Finish calculating color image histogram\n");
		}
		else		//默认为黑白相机
		{
			IDLog("Start calculating histogram of mono image\n");
			cv::Mat img(ImageHeight,ImageWidth, CV_8UC1, imgBuf);		//单通道图像信息
			const float *ranges[] = { hranges };
			int histSize = 256;  //存放每个维度的直方图的尺寸的数组
			int channels = 0;  //通道数
			cv::calcHist(&img, 1, &channels, cv::Mat(), dstHist, 1, &histSize, ranges);
			IDLog("Finish calculating histogram of mono image\n");
		}
		std::ofstream outfile;
		outfile.open("histogram.txt");
		if(outfile.is_open())
		{
			outfile << dstHist;
			outfile.close();
		}
	}
}
