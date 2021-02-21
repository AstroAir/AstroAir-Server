# CMakeLists.txt
#
# Copyright (C) 2020-2021 Max Qian
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


cmake_minimum_required(VERSION 3.10)
project(airserver VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)		#默认C++风格为20
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_COMPILER /usr/bin/clang-11)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-11)		#默认使用clang++-11,G++ 9.3.0亦可

#输出配置文件用于后续编译
configure_file(config.h.in ${PROJECT_SOURCE_DIR}/src/config.h)

add_executable(airserver src/main.cpp)

set(PLATFORM x64)
set(MODE DEBUG)
set(LOG "install.txt")
message("-- Building on the ${PLATFORM} platform")
message("-- Building mode is ${MODE}")
message("-- Export installation files to ${LOG}")
option(DEBUG_MODE "Using debug mode" ON)

#设置Websocketpp库
option(HAS_WEBSOCKET "Using Websocketpp Library" ON)
if(HAS_WEBSOCKET)
	find_path(PATH_WEBSOCKET server.hpp /usr/include/websocketpp)
	find_path(PATH_WEBSOCKET server.hpp /usr/local/include/websocketpp)
	if(PATH_WEBSOCKET)
		message("-- Found websocketpp library in ${PATH_WEBSOCKET}")
		add_library(LIBWEBSOCKET src/wsserver.cpp)
		target_link_libraries(airserver PUBLIC LIBWEBSOCKET)
		target_link_libraries(airserver PUBLIC libpthread.so)
		target_link_libraries(airserver PUBLIC libboost_system.so)
		target_link_libraries(airserver PUBLIC libboost_thread.so)
		#SSL支持
		target_link_libraries(airserver PUBLIC libssl.so)
		target_link_libraries(airserver PUBLIC libcrypto.so)
	else()
		message("-- Could not found websocketpp library.Try to build it!")
		add_custom_command(
			TARGET LIBWEBSOCKET
			PRE_BUILD
			COMMAND sudo apt install libasio-dev libboost_system-dev libwebsocketpp-dev -y
			COMMENT "Downloaded and Building WebSocketpp Library"
		)
	endif()
else()
	message("Please check setting,websocketpp is one of the main library!")
endif()

#设置ASI相机库
option(HAS_ASI "Using ASI camera Library" ON)
if(HAS_ASI)
	find_path(PATH_ASI ASICamera2.h /usr/include/libasi)
	find_path(PATH_ASI ASICamera2.h /usr/local/include/libasi)
	find_library(PATH_ASI_LIB libASICamera2.so /usr/lib)
	find_library(PATH_ASI_LIB libASICamera2.so /usr/local/lib)
	if(PATH_ASI AND PATH_ASI_LIB)
		message("-- Found ASI camera header file in ${PATH_ASI} and library in ${PATH_ASI_LIB}")
		add_library(LIBASI src/air-asi/asi_ccd.cpp)
		target_link_libraries(airserver PUBLIC LIBASI)
		link_directories("${PROJECT_SOURCE_DIR}/src/libasi/${PLATFORM}/")
		target_link_libraries(airserver PUBLIC libASICamera2.so)		#ASI相机
		target_link_libraries(airserver PUBLIC libusb-1.0.so)
	else()
		message("-- Could not found ASI camera library.Please build it before intall!")
		add_custom_command(
			TARGET LIBASI
			PRE_BUILD 
			COMMAND sudo apt install libasi -y
			COMMENT "Downloaded and Building ASI Library"
		)
	endif()
else()
	message("-- Not built ASI camera library")
endif()

#设置QHY相机库
option(HAS_QHY "Using QHY camera Library" ON)
if(HAS_QHY)
	find_path(PATH_QHY qhyccd.h /usr/include/libqhy)
	find_path(PATH_QHY qhyccd.h /usr/local/include/libqhy)
	find_library(PATH_QHY_LIB libqhyccd.so /usr/lib)
	find_library(PATH_QHY_LIB libqhyccd.so /usr/local/lib)
	if(PATH_QHY AND PATH_QHY_LIB)
		message("-- Found QHY camera header file in ${PATH_QHY} and library in ${PATH_QHY_LIB}")
		add_library(LIBQHY src/air-qhy/qhy_ccd.cpp)
		target_link_libraries(airserver PUBLIC LIBQHY)
		link_directories("${PROJECT_SOURCE_DIR}/src/libqhy/${PLATFORM}/")
		target_link_libraries(airserver PUBLIC libqhyccd.so)		#QHY相机
	else()
		message("-- Could not found QHY camera library.Please build it before intall!")
		add_custom_command(
			TARGET LIBQHY
			PRE_BUILD 
			COMMAND sudo apt install libqhy -y
			COMMENT "Downloaded and Building QHY Library"
		)
	endif()
else()
	message("-- Not built QHY camera library")
endif()

#设置INDI库
option(HAS_INDI "Using INDI Library" ON)
if(HAS_INDI)
	find_path(PATH_INDI baseclient.h /usr/include/libindi)
	find_path(PATH_INDI baseclient.h /usr/local/include/libindi)
	find_library(PATH_INDI_LIB libindiclient.a /usr/lib)
	find_library(PATH_INDI_LIB libindiclient.a /usr/local/lib)
	if(PATH_INDI AND PATH_INDI_LIB)
		message("-- Found INDI header file in ${PATH_INDI} and library in ${PATH_INDI_LIB}")
		add_library(LIBINDI src/air-indi/indi_device.cpp)
		target_link_libraries(airserver PUBLIC LIBINDI)
		target_link_libraries(airserver PUBLIC libindiclient.a)
		target_link_libraries(airserver PUBLIC libz.so)
	else()
		message("-- Could not found INDI library.Try to build it!")
		add_custom_command(
			TARGET LIBINDI
			PRE_BUILD 
			COMMAND sudo apt install libindi-dev -y
			COMMENT "Downloaded and Building INDI Library"
		)
	endif()
else()
	message("-- Not built INDI library")
endif()

#设置GPhoto2相机库
option(HAS_GPhoto2 "Using GPhoto2 camera Library" ON)
if(HAS_GPhoto2)
	find_path(PATH_GPhoto2 gphoto2.h /usr/include/gphoto2)
	find_path(PATH_GPhoto2 qhyccd.h /usr/local/include/gphoto2)
	find_library(PATH_GPhoto2_LIB libgphoto2.so /usr/lib)
	find_library(PATH_GPhoto2_LIB libgphoto2.so /usr/local/lib)
	if(PATH_GPhoto2 AND PATH_GPhoto2_LIB)
		message("-- Found GPhoto2 camera header file in ${PATH_GPhoto2} and library in ${PATH_GPhoto2_LIB}")
		add_library(LIBGPhoto2 src/air-gphoto2/gphoto2_ccd.cpp)
		target_link_libraries(airserver PUBLIC LIBGPhoto2)
		target_link_libraries(airserver PUBLIC libgphoto2.so)		#GPhoto2相机
		target_link_libraries(airserver PUBLIC libgphoto2_port.so)
	else()
		message("-- Could not found GPhoto2 camera library.Please build it before intall!")
		add_custom_command(
			TARGET LIBGPhoto2
			PRE_BUILD 
			COMMAND sudo apt install libgphoto2-dev -y
			COMMENT "Downloaded and Building GPhoto2 Library"
		)
	endif()
else()
	message("-- Not built GPhoto2 camera library")
endif()

#设置日志系统库
option(HAS_LOGGER "Using Logger system Library" ON)
if(HAS_LOGGER)
	add_library(LIBLOGGER src/logger.cpp)
	target_link_libraries(airserver PUBLIC LIBLOGGER)
endif()

#设置Nova库
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

#设置CFitsIO库
option(HAS_FITSIO "Using CFitsIO Library" ON)
if(HAS_FITSIO)
	find_path(PATH_FITSIO fitsio.h /usr/include)
	find_path(PATH_FITSIO fitsio.h /usr/local/include)
	find_library(PATH_FITSIO_LIB libcfitsio.so /usr/lib)
	find_library(PATH_FITSIO_LIB libcfitsio.so /usr/local/lib)
	if(PATH_FITSIO AND PATH_FITSIO_LIB)
		message("-- Found FITSIO header file in ${PATH_FITSIO} and library in ${PATH_FITSIO_LIB}")
		target_link_libraries(airserver PUBLIC libcfitsio.so)		#CFitsIO
	else()
		message("-- Could not found CFitsIO library.Try to build it!")
		add_custom_command(
			TARGET LIBFITSIO
			PRE_BUILD 
			COMMAND sudo apt install libcfitsio-dev -y
			COMMENT "Downloaded and Building CFitsIO Library"
		)
	endif()
else()
	message("-- Not built CFitsIO library")
endif()

#设置OPENCV图像处理库
option(HAS_OPENCV "Using opencv Library" ON)
if(HAS_OPENCV)
	find_path(PATH_OPENCV opencv.hpp /usr/include/opencv2)
	find_path(PATH_OPENCV opencv.hpp /usr/local/include/opencv2)
	find_library(PATH_OPENCV_LIB libopencv_core.so /usr/lib)
	find_library(PATH_OPENCV_LIB libopencv_core.so /usr/local/lib)
	if(PATH_OPENCV AND PATH_OPENCV_LIB)
		message("-- Found OPENCV header file in ${PATH_OPENCV} and library in ${PATH_OPENCV_LIB}")
		add_library(LIBOPENCV src/opencv.cpp)
		target_link_libraries(airserver PUBLIC LIBOPENCV)
		target_link_libraries(airserver PUBLIC libopencv_imgproc.so)
		target_link_libraries(airserver PUBLIC libopencv_imgcodecs.so)
		target_link_libraries(airserver PUBLIC libopencv_core.so)
	else()
		message("-- Could not found OPENCV library.Try to build it!")
		add_custom_command(
			TARGET LIBOPENCV
			PRE_BUILD 
			COMMAND sudo apt install libopencv-dev -y
			COMMENT "Downloaded and Building OPENCV Library"
		)
	endif()
else()
	message("-- Not built OPENCV library")
endif()

#设置Base64编码库
option(HAS_BASE64 "Using base64 Library" ON)
if(HAS_BASE64)
	add_library(LIBBASE64 src/base64.cpp)
	target_link_libraries(airserver PUBLIC LIBBASE64)
endif()

#设置JsonCpp库
option(HAS_JSONCPP "Using JsonCpp Library" ON)
if(HAS_JSONCPP)
	find_path(PATH_JSONCPP json.h /usr/include/json)
	find_path(PATH_JSONCPP json.h /usr/local/include/json)
	find_library(PATH_JSONCPP_LIB libjsoncpp.so /usr/lib)
	find_library(PATH_JSONCPP_LIB libopencv_core.so /usr/local/lib)
	if(PATH_JSONCPP AND PATH_JSONCPP_LIB)
		message("-- Found JsonCpp header file in ${PATH_JSONCPP} and library in ${PATH_JSONCPP_LIB}")
		target_link_libraries(airserver PUBLIC libjsoncpp.so)
	else()
		message("-- Could not found JsonCpp library.Try to build it!")
		add_custom_command(
			TARGET LIBOPENCV
			PRE_BUILD
			COMMAND sudo apt install libjsoncpp-dev -y
			COMMENT "Downloaded and Building JSONCPP Library"
		)
	endif()
else()
	message("Please check setting,jsoncpp is one of the main library!")
endif()

set(LINK_DIR /usr/lib)
link_directories(${LINK_DIR})

#头文件
target_include_directories(airserver PUBLIC
                           "${PROJECT_BINARY_DIR}/src"
                           "${PROJECT_SOURCE_DIR}/src/air-asi"
                           "${PROJECT_SOURCE_DIR}/src/libasi"
						   "${PROJECT_SOURCE_DIR}/src/air-qhy"
                           "${PROJECT_SOURCE_DIR}/src/libqhy"
                           )

#安装到系统
install(TARGETS airserver DESTINATION bin)
