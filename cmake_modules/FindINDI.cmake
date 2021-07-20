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
 
#  Date:2021-7-18
 
#  Description: INDI Device Cmake Tool

################################################## 

option(HAS_INDI "Using INDI Library" ON)
if(HAS_INDI)
	find_path(PATH_INDI basedevice.h /usr/include/libindi)
	find_path(PATH_INDI basedevice.h /usr/local/include/libindi)
	find_library(PATH_INDI_LIB libindidriver.so /usr/lib)
	find_library(PATH_INDI_LIB libindidriver.so /usr/local/lib)
	if(PATH_INDI AND PATH_INDI_LIB)
		message("-- Found INDI header file in ${PATH_INDI} and library in ${PATH_INDI_LIB}")
	else()
		message("-- Could not found INDI library.Please build it before intall!")
		add_custom_command(
			TARGET airserver
			PRE_BUILD 
			COMMAND sudo apt install libindi-dev -y
			COMMENT "Downloaded and Building INDI Library"
		)
	endif()
	add_library(INDI src/indi/indiccd.cpp src/indi/inditelescope.cpp)
	target_link_libraries(airserver PUBLIC INDI)
	target_link_libraries(airserver PUBLIC libindidriver.so)		#ASI相机
	target_link_libraries(airserver PUBLIC libindiclient.a)
else()
	message("-- Not built INDI library")
endif()