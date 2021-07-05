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
 
#  Description: JsonCPP Cmake Tool

################################################## 

option(HAS_JSONCPP "Using JsonCpp Library" ON)
if(HAS_JSONCPP)
	find_path(PATH_JSONCPP json.h /usr/include)
	find_path(PATH_JSONCPP json.h /usr/local/include)
	find_library(PATH_JSONCPP_LIB libjsoncpp.so /usr/lib)
	find_library(PATH_JSONCPP_LIB libjsoncpp.so /usr/local/lib)
	if(PATH_JSONCPP AND PATH_JSONCPP_LIB)
		message("-- Found JsonCpp header file in ${PATH_JSONCPP} and library in ${PATH_JSONCPP_LIB}")
		target_link_libraries(airserver PUBLIC libjsoncpp.so)
	else()
		message("-- Could not found JsonCpp library.Try to build it!")
		add_custom_command(
			TARGET airserver
			PRE_BUILD
			COMMAND sudo apt install libjsoncpp-dev -y
			COMMENT "Downloaded and Building JSONCPP Library"
		)
	endif()
else()
	message("Please check setting,jsoncpp is one of the main library!")
endif()