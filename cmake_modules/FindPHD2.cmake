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
 
#  Date:2021-7-1
 
#  Description: PHD2 Cmake Tool

################################################## 

option(HAS_PHD2 "Using PHD2" ON)
if(HAS_PHD2)
	find_path(PATH_PHD2 phd2.bin /usr/bin)
	find_path(PATH_PHD2 phd2.bin /usr/local/bin)
	if(PATH_PHD2)
		message("-- Found PHD2 in ${PATH_PHD2}")
		add_library(LIBPHD2 src/guider/air-phd2/air_phd2.cpp)
		target_link_libraries(airserver PUBLIC LIBPHD2)
	else()
		message("-- Could not found PHD2.Please build it before intall!")
		add_custom_command(
			TARGET LIBPHD2
			PRE_BUILD 
			COMMAND sudo apt install phd2 -y
			COMMENT "Downloaded and Building PHD2"
		)
	endif()
else()
	message("-- Not use PHD2")
endif()