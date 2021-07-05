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
 
#  Description: Gphoto2 Cmake Tool

################################################## 

option(HAS_GPhoto2 "Using GPhoto2 camera Library" ON)
if(HAS_GPhoto2)
	find_path(PATH_GPhoto2 gphoto2.h /usr/include/gphoto2)
	find_path(PATH_GPhoto2 qhyccd.h /usr/local/include/gphoto2)
	find_library(PATH_GPhoto2_LIB libgphoto2.so /usr/lib)
	find_library(PATH_GPhoto2_LIB libgphoto2.so /usr/local/lib)
	if(PATH_GPhoto2 AND PATH_GPhoto2_LIB)
		message("-- Found GPhoto2 camera header file in ${PATH_GPhoto2} and library in ${PATH_GPhoto2_LIB}")
		add_library(LIBGPhoto2 src/camera/air-gphoto2/gphoto2_ccd.cpp)
		target_link_libraries(airserver PUBLIC LIBGPhoto2)
		target_link_libraries(airserver PUBLIC libgphoto2.so)		#GPhoto2相机
		target_link_libraries(airserver PUBLIC libgphoto2_port.so)
	else()
		message("-- Could not found GPhoto2 camera library.Please build it before intall!")
		add_custom_command(
			TARGET airserver
			PRE_BUILD 
			COMMAND sudo apt install libgphoto2-dev -y
			COMMENT "Downloaded and Building GPhoto2 Library"
		)
	endif()
else()
	message("-- Not built GPhoto2 camera library")
endif()