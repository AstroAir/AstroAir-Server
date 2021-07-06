#  
#  Copyright (C) 2020-2021 Max Qian
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#  

################################################## 
 
#  Copyright: 2020-2021 Max Qian. All rights reserved
 
#  Author:Max Qian

#  E-mail:astro_air@126.com
 
#  Date:2021-6-29
 
#  Description: Image Progress Tools Cmake Tool

################################################## 

option(HAS_FITSIO "Using CFitsIO Library" ON)
if(HAS_FITSIO)
	find_path(PATH_FITSIO fitsio.h /usr/include)
	find_path(PATH_FITSIO fitsio.h /usr/local/include)
	find_library(PATH_FITSIO_LIB libcfitsio.so /usr/lib)
	find_library(PATH_FITSIO_LIB libcfitsio.so /usr/local/lib)
	if(PATH_FITSIO AND PATH_FITSIO_LIB)
		message("-- Found FITSIO header file in ${PATH_FITSIO} and library in ${PATH_FITSIO_LIB}")
	else()
		message("-- Could not found CFitsIO library.Try to build it!")
		add_custom_command(
			TARGET airserver
			PRE_BUILD 
			COMMAND sudo apt install libcfitsio-dev libccfits-dev -y
			COMMENT "Downloaded and Building CFitsIO Library"
		)
	endif()
	add_library(LIBFITSIO src/tools/ImgFitsIO.cpp)
	target_link_libraries(airserver PUBLIC LIBFITSIO)
	target_link_libraries(airserver PUBLIC libgsl.so)
	target_link_libraries(airserver PUBLIC libcfitsio.so)		#CFitsIO
else()
	message("-- Not built CFitsIO library")
endif()

#设置OPENCV图像处理库
option(HAS_OPENCV "Using opencv Library" ON)
option(HAS_BASE64 "Using base64 Library" ON)
if(HAS_OPENCV AND HAS_BASE64)
	find_path(PATH_IMGTOOLS_OPENCV opencv.hpp /usr/include/opencv2)
	find_path(PATH_IMGTOOLS_OPENCV opencv.hpp /usr/local/include/opencv2)
	find_library(PATH_IMGTOOLS_OPENCV_LIB libopencv_core.so /usr/lib/)
	find_library(PATH_IMGTOOLS_OPENCV_LIB libopencv_core.so /usr/local/lib)
	if(PATH_IMGTOOLS_OPENCV AND PATH_IMGTOOLS_OPENCV_LIB)
		message("-- Found OPENCV header file in ${PATH_IMGTOOLS_OPENCV} and library in ${PATH_IMGTOOLS_OPENCV_LIB}")
	else()
		message("-- Could not found OPENCV library.Try to build it!")
		add_custom_command(
			TARGET airserver
			PRE_BUILD 
			COMMAND sudo apt install libopencv-dev -y
			COMMENT "Downloaded and Building OPENCV Library"
		)
	endif()
	add_library(LIBOPENCV src/tools/ImgTools.cpp)
	target_link_libraries(airserver PUBLIC LIBOPENCV)
	target_link_libraries(airserver PUBLIC libopencv_imgcodecs.so)
	target_link_libraries(airserver PUBLIC libopencv_core.so)
	target_link_libraries(airserver PUBLIC libopencv_imgproc.so)
else()
	message("-- Not built OPENCV library")
endif()