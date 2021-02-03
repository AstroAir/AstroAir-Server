/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <string.h>

namespace AstroAir::OPENCV
{
	void SaveImage(unsigned char *imgBuf,std::string ImageName,bool isColor,int ImageHeight,int ImageWidth)
	{
		std::vector<int> compression_params;		//图像质量
		compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);		//JPG图像质量
		compression_params.push_back(100);
		const char* JPGName = strtok(const_cast<char *>(ImageName.c_str()),".");
		strcat(const_cast<char *>(JPGName), ".jpg");
		if(isColor == true)
		{
			cv::Mat img(ImageHeight,ImageWidth, CV_8UC3, imgBuf);		//3通道图像信息
			imwrite(JPGName,img, compression_params);	
		}
		else
		{
			cv::Mat img(ImageHeight,ImageWidth, CV_8UC1, imgBuf);		//单通道图像信息
			imwrite(JPGName,img, compression_params);		//写入文件
		}
	}
}
