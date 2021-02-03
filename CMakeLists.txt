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

cmake_minimum_required(VERSION 3.10)
project(airserver VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PLATFORM x64)
set(MODE DEBUG)

configure_file(config.h.in config.h)

message("-- Building on the ${PLATFORM} platform")
message("-- Building mode is ${MODE}")

add_executable(airserver src/main.cpp)

#设置Websocketpp库
option(HAS_WEBSOCKET "Using Websocketpp Library" ON)
if(HAS_WEBSOCKET)
	find_path(PATH_WEBSOCKET server.hpp /usr/include/websocketpp)
	find_path(PATH_WEBSOCKET server.hpp /usr/local/include/websocketpp)
	if(PATH_WEBSOCKET)
		message("-- Found websocketpp library in ${PATH_WEBSOCKET}")
		add_library(LIBWEBSOCKET src/wsserver.cpp)
		target_link_libraries(airserver PUBLIC LIBWEBSOCKET)
	else()
		message("-- Could not found websocketpp library.Use local version!")
		target_include_directories(airserver PUBLIC "${PROJECT_SOURCE_DIR}/include/websocketpp")
		#add_custom_command(COMMAND sudo cp -r include/websocketpp /usr/include)
	endif()
else()
	message("Please check setting,websocketpp is one of the main library!")
endif()

#设置ASI相机库
option(HAS_ASI "Using ASI camera Library" ON)
if(HAS_ASI)
	find_path(PATH_ASI ASICamera2.h /usr/include/libasi)
	find_path(PATH_ASI ASICamera2.h /usr/local/include/libasi)
	if(PATH_ASI)
		message("-- Found ASI camera library in ${PATH_ASI}")
		add_library(LIBASI src/air-asi/asi_ccd.cpp)
		target_link_libraries(airserver PUBLIC LIBASI)
	else()
		message("-- Could not found ASI camera library.Try to build it!")
		add_custom_command(COMMAND sudo mkdir /usr/include/libasi
						COMMAND sudo cp *.h /usr/include/libasi)
	endif()
	target_link_libraries(airserver PUBLIC libopencv_core.so)
	target_link_libraries(airserver PUBLIC libopencv_imgcodecs.so)
else()
	message("-- Not built ASI camera library")
endif()

#设置日至系统库
option(HAS_LOGGER "Using Logger system Library" ON)
if(HAS_LOGGER)
	add_library(LIBLOGGER src/logger.cpp)
	target_link_libraries(airserver PUBLIC LIBLOGGER)
endif()

#设置日至系统库
option(HAS_NOVA "Using Nova Library" ON)
if(HAS_NOVA)
	find_library(PATH_NOVA libnova.so /usr/lib)
	find_library(PATH_NOVA libnova.so /usr/local/lib)
	if(PATH_NOVA)
		message("-- Found Nova library in ${PATH_NOVA}")
		add_library(LIBNOVA src/libastro.cpp)
		target_link_libraries(airserver PUBLIC LIBNOVA)
	endif()
endif()

option(HAS_OPENCV "Using opencv Library" ON)
if(HAS_OPENCV)
	add_library(LIBOPENCV src/opencv.cpp)
	target_link_libraries(airserver PUBLIC LIBOPENCV)
endif()

set(LINK_DIR_L /usr/local/lib)
link_directories(${LINK_DIR_L})
set(LINK_DIR /usr/lib)
link_directories(${LINK_DIR})

if(${PLATFORM} EQUAL "x64")
	message("Using the x64 platform ASI camera Library")
	set(LIBASI_x64 libasi/x64)
	link_directories(${LIBASI_x64})
elseif(${PLATFORM} EQUAL "armv7")
	message("Using the armv7 platform ASI camera Library")
	set(LIBASI_armv7 libasi/armv7)
	link_directories(${LIBASI_armv7})
elseif(${PLATFORM} EQUAL armv8)
	message("Using the armv8 platform ASI camera Library")
	set(LIBASI_armv8 libasi/armv8)
	link_directories(${LIBASI_armv8})
endif()

target_link_libraries(airserver PUBLIC libASICamera2.so)
option(HAS_JSONCPP "Using JsonCpp Library" ON)
if(HAS_JSONCPP)
	find_path(PATH_JSONCPP json.h /usr/include/json)
	find_path(PATH_JSONCPP json.h /usr/local/include/json)
	if(PATH_JSONCPP)
		message("-- Found JsonCpp library in ${PATH_JSONCPP}")
		target_link_libraries(airserver PUBLIC libjsoncpp.so)
	else()
		message("-- Could not found ASI camera library.Use local version!")
		target_include_directories(airserver PUBLIC "${PROJECT_SOURCE_DIR}/include/json")
		set(LOCAL_JSONCPP lib/json)
		link_directories(${LOCAL_JSONCPP})
		target_link_libraries(airserver PUBLIC libjsoncpp.so)
	endif()
else()
	message("Please check setting,jsoncpp is one of the main library!")
endif()

target_link_libraries(airserver PUBLIC libcfitsio.so)
target_link_libraries(airserver PUBLIC libpthread.so)
target_link_libraries(airserver PUBLIC libnova.so)
target_link_libraries(airserver PUBLIC libusb-1.0.so)
target_link_libraries(airserver PUBLIC libopencv_highgui.so)
target_link_libraries(airserver PUBLIC libopencv_imgproc.so)
target_link_libraries(airserver PUBLIC libopencv_imgcodecs.so)
target_link_libraries(airserver PUBLIC libopencv_core.so)

target_include_directories(airserver PUBLIC
                           "${PROJECT_BINARY_DIR}/src"
                           "${PROJECT_SOURCE_DIR}/src/air-asi"
                           "${PROJECT_SOURCE_DIR}/src/libasi"
                           )

#安装到系统
install(TARGETS airserver DESTINATION bin)
install(FILES "${PROJECT_BINARY_DIR}/src/wsserver.h" DESTINATION include)
install(FILES "${PROJECT_BINARY_DIR}/src/logger.h" DESTINATION include)
install(FILES "${PROJECT_BINARY_DIR}/src/libastro.h" DESTINATION include)
install(FILES "${PROJECT_BINARY_DIR}/src/air-asi/air_ccd.h" DESTINATION include)
