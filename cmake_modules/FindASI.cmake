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
 
#  Description: ASI Device Cmake Tool

################################################## 

option(HAS_ASI_CAMERA "Using ASI Camera Library" ON)
if(HAS_ASI_CAMERA)
	find_path(PATH_ASI_CAMERA ASICamera2.h /usr/include/libasi)
	find_path(PATH_ASI_CAMERA ASICamera2.h /usr/local/include/libasi)
	find_library(PATH_ASI_CAMERA_LIB libASICamera2.so /usr/lib)
	find_library(PATH_ASI_CAMERA_LIB libASICamera2.so /usr/local/lib)
	if(PATH_ASI_CAMERA AND PATH_ASI_CAMERA_LIB)
		message("-- Found ASI camera header file in ${PATH_ASI} and library in ${PATH_ASI_LIB}")
		add_library(LIBASI_CAMERA src/camera/air-asi/asi_ccd.cpp)
		target_link_libraries(airserver PUBLIC LIBASI_CAMERA)
		target_link_libraries(airserver PUBLIC libASICamera2.so)		#ASI相机
		target_link_libraries(airserver PUBLIC libusb-1.0.so)
	else()
		message("-- Could not found ASI camera library.Please build it before intall!")
		add_custom_command(
			TARGET airserver
			PRE_BUILD 
			COMMAND sudo apt install libasi -y
			COMMENT "Downloaded and Building ASI Camera Library"
		)
	endif()
else()
	message("-- Not built ASI camera library")
endif()
option(HAS_ASI_FOCUSER "Using ASI Focuser Library" ON)
if(HAS_ASI_FOCUSER)
	find_path(PATH_ASI_FOCUSER EAF_focuser.h /usr/include/libasi)
	find_path(PATH_ASI_FOCUSER EAF_focuser.h /usr/local/include/libasi)
	find_library(PATH_ASI_FOCUSER_LIB libEAFFocuser.so /usr/lib)
	find_library(PATH_ASI_FOCUSER_LIB libEAFFocuser.so /usr/local/lib)
	if(PATH_ASI_FOCUSER AND PATH_ASI_FOCUSER_LIB)
		message("-- Found ASI focuser header file in ${PATH_ASI_FOCUSER} and library in ${PATH_ASI_FOCUSER_LIB}")
		add_library(LIBASI_FOCUSER src/focus/air-eaf/air_eaf.cpp)
		target_link_libraries(airserver PUBLIC LIBASI_FOCUSER)
		target_link_libraries(airserver PUBLIC libEAFFocuser.so)		#ASI电动调焦座
	else()
		message("-- Could not found ASI focuser library.Please build it before intall!")
		add_custom_command(
			TARGET airserver
			PRE_BUILD 
			COMMAND sudo apt install libasi -y
			COMMENT "Downloaded and Building ASI Focuser Library"
		)
	endif()
else()
	message("-- Not built ASI focuser library")
endif()
option(HAS_ASI_FILTER "Using ASI Filter Library" ON)
if(HAS_ASI_FILTER)
	find_path(PATH_ASI_FILTER EFW_filter.h /usr/include/libasi)
	find_path(PATH_ASI_FILTER EFW_filter.h /usr/local/include/libasi)
	find_library(PATH_ASI_FILTER_LIB libEFWFilter.so /usr/lib)
	find_library(PATH_ASI_FILTER_LIB libEFWFilter.so /usr/local/lib)
	if(PATH_ASI_FILTER AND PATH_ASI_FILTER_LIB)
		message("-- Found ASI filter header file in ${PATH_ASI_FILTER} and library in ${PATH_ASI_FILTER_LIB}")
		add_library(LIBASI_FILTER src/filter/air-efw/air_efw.cpp)
		target_link_libraries(airserver PUBLIC LIBASI_FILTER)
		target_link_libraries(airserver PUBLIC libEFWFilter.so)		#ASI滤镜轮
	else()
		message("-- Could not found ASI filter library.Please build it before intall!")
		add_custom_command(
			TARGET airserver
			PRE_BUILD 
			COMMAND sudo apt install libasi -y
			COMMENT "Downloaded and Building ASI Filter Library"
		)
	endif()
else()
	message("-- Not built ASI filter library")
endif()