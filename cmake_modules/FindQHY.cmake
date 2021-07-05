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
 
#  Description: QHY Device Cmake Tool

################################################## 

option(HAS_QHY "Using QHY Library" ON)
if(HAS_QHY)
	find_path(PATH_QHY qhyccd.h /usr/include/libqhy)
	find_path(PATH_QHY qhyccd.h /usr/local/include/libqhy)
	find_library(PATH_QHY_LIB libqhyccd.so /usr/lib)
	find_library(PATH_QHY_LIB libqhyccd.so /usr/local/lib)
	if(PATH_QHY AND PATH_QHY_LIB)
		message("-- Found QHY header file in ${PATH_QHY} and library in ${PATH_QHY_LIB}")
		add_library(LIBQHY_CAMERA src/camera/air-qhy/qhy_ccd.cpp)		#QHY相机
		target_link_libraries(airserver PUBLIC LIBQHY_CAMERA)
		add_library(LIBQHY_FILTER src/filter/air-cfw/qhy_cfw.cpp)		#QHY滤镜轮
		target_link_libraries(airserver PUBLIC LIBQHY_FILTER)
		target_link_libraries(airserver PUBLIC libqhyccd.so)
	else()
		message("-- Could not found QHY library.Please build it before intall!")
		add_custom_command(
			TARGET airserver
			PRE_BUILD 
			COMMAND sudo apt install libqhy -y
			COMMENT "Downloaded and Building QHY Library"
		)
	endif()
else()
	message("-- Not built QHY camera library")
endif()