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
 
#  Description: Websocketpp Cmake Tool

################################################## 

option(HAS_WEBSOCKET "Using Websocketpp Library" ON)
if(HAS_WEBSOCKET)
	find_path(PATH_WEBSOCKET server.hpp /usr/include/websocketpp)
	find_path(PATH_WEBSOCKET server.hpp /usr/local/include/websocketpp)
	if(PATH_WEBSOCKET)
		message("-- Found Websocketpp library in ${PATH_WEBSOCKET}")
		add_library(LIBWEBSOCKET src/wsserver.cpp)
		target_link_libraries(airserver PUBLIC LIBWEBSOCKET)
		target_link_libraries(airserver PUBLIC libpthread.so)
	else()
		message("-- Could not found websocketpp library.Try to build it!")
		add_custom_command(
			TARGET airserver
			PRE_BUILD
			COMMAND sudo apt install libasio-dev libwebsocketpp-dev -y
			COMMENT "Downloaded and Building WebSocketpp Library"
		)
	endif()
else()
	message("Please check setting,websocketpp is one of the main library!")
endif()