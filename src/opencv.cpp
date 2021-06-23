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
		compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);		//JPG图像质量
		compression_params.push_back(100);
		//const char* JPGName = strtok(const_cast<char *>(ImageName.c_str()),".");
		//strcat(const_cast<char *>(JPGName), ".jpg");
		if(isColor == true)
		{
			cv::Mat img(ImageHeight,ImageWidth, CV_8UC3, imgBuf);		//3通道图像信息
			
			return Mat2Base64(img,"jpg");
			//imwrite(JPGName,img, compression_params);
			//IDLog("JPG image saved successfully\n");
			//return Mat2Base64(img,"jpg");
		}
		else
		{
			cv::Mat img(ImageHeight,ImageWidth, CV_8UC1, imgBuf);		//单通道图像信息

			return Mat2Base64(img,"jpg");
			//imwrite(JPGName,img, compression_params);		//写入文件
			//IDLog("JPG image saved successfully\n");
			//return Mat2Base64(img,"jpg");
		}
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
			//IDLog("Start calculating color image histogram\n");
			cv::Mat img(ImageHeight,ImageWidth, CV_8UC3, imgBuf);		//3通道图像信息
			int histSize[3] = {256,256,256};
			const float *ranges[3] = { hranges , hranges , hranges};
			int channels[3] = {0,1,2};
			cv::calcHist(&img, 1, channels, cv::Mat(), dstHist, 3, histSize, ranges);
			IDLog("Finish calculating color image histogram\n");
		}
		else		//默认为黑白相机
		{
			//IDLog("Start calculating histogram of mono image\n");
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

	/*
     * name: drop_noise(cv::Mat &img)
     * @param img:图像缓冲区
     * describe: noise reduction
     * 描述： 降噪
     */
	void drop_noise(cv::Mat &img)
	{
		for (int i = 0; i < img.rows; i++)
		{
			for (int j = 0; j < img.cols; j++)
			{
				if (img.at<unsigned char>(i, j) >= 69)//图像中有些盐噪声，通过设置阈值滤除掉
				{
					img.at<unsigned char>(i, j) = 0;
				}
			}
		}
	}

	cv::Mat matrixWiseMulti(cv::Mat &m1, cv::Mat &m2)
	{
		cv::Mat dst = m1.mul(m2);
		return dst;
	}

	cv::Mat ACE(cv::Mat &src)
	{
		int rows = src.rows;
		int cols = src.cols;
		int C = 3,n = 3;
		float MaxCG = 7.5;
		cv::Mat meanLocal; //图像局部均值  
		cv::Mat varLocal;  //图像局部方差  
		cv::Mat meanGlobal;//全局均值
		cv::Mat varGlobal; //全局标准差  

		blur(src.clone(), meanLocal, cv::Size(n, n));
		cv::Mat highFreq = src - meanLocal;//高频成分 
		varLocal = matrixWiseMulti(highFreq, highFreq);
		blur(varLocal, varLocal, cv::Size(n, n));
		//换算成局部标准差  
		varLocal.convertTo(varLocal, CV_32F);
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				varLocal.at<float>(i, j) = (float)sqrt(varLocal.at<float>(i, j));
			}
		}
		cv::meanStdDev(src, meanGlobal, varGlobal);
		cv::Mat gainArr = 0.5 * meanGlobal / varLocal;//增益系数矩阵  
		//对增益矩阵进行截止  
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++){
				if (gainArr.at<float>(i, j) > MaxCG)
				{
					gainArr.at<float>(i, j) = MaxCG;
				}
			}
		}
		gainArr.convertTo(gainArr, CV_8U);
		gainArr = matrixWiseMulti(gainArr, highFreq);
		cv::Mat dst2 = meanLocal + C*highFreq;
		return dst2;
	}
/*
	static std::string base64Encode(const unsigned char *Data, int DataByte)
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

	//imgType 包括png bmp jpg jpeg等opencv能够进行编码解码的文件
	static std::string Mat2Base64(const cv::Mat &img, std::string imgType)
	{
		//Mat转base64
		std::string img_data;
		std::vector<uchar> vecImg;
		std::vector<int> vecCompression_params;
		vecCompression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
		vecCompression_params.push_back(100);
		imgType = "." + imgType;
		cv::imencode(imgType, img, vecImg, vecCompression_params);
		img_data = "data:image/jpg;base64,"+base64Encode(vecImg.data(), vecImg.size());
		return img_data;
	}
	*/
}
