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
 
#  Description: Nova Cmake Tool

################################################## 

option(HAS_NOVA "Using Nova Library" ON)
if(HAS_NOVA)
	find_path(PATH_NOVA nutation.h /usr/include/libnova)
	find_path(PATH_NOVA nutation.h /usr/local/include/libnova)
	find_library(PATH_NOVA_LIB libnova.so /usr/lib)
	find_library(PATH_NOVA_LIB libnova.so /usr/local/lib)
	if(PATH_NOVA AND PATH_NOVA_LIB)
		message("-- Found Nova header file in ${PATH_NOVA} and library in ${PATH_NOVA_LIB}")
		add_library(LIBNOVA src/libastro.cpp)
		target_link_libraries(airserver PUBLIC LIBNOVA)
		add_library(LIBANOVA src/telescope/air_nova.cpp)
		target_link_libraries(airserver PUBLIC LIBANOVA)
		target_link_libraries(airserver PUBLIC libnova.so)
	else()
		message("-- Could not found Nova library.Try to build it!")
		add_custom_command(
			TARGET LIBNOVA
			PRE_BUILD 
			COMMAND sudo apt install libnova-dev -y
			COMMENT "Downloaded and Building Nova Library"
		)
	endif()
else()
	message("-- Not built Nova library")
endif()