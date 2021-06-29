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

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

option(USE_C++20 "Using c++ 20 style" ON)
if(USE_C++20)
	set(CMAKE_CXX_STANDARD 20)		#默认C++风格为20
	message("-- Using c++ 20 as building style")
else()
	set(CMAKE_CXX_STANDARD 11)		#默认C++风格为20
	message("-- Using c++ 11 as building style")
endif()

set(CMAKE_CXX_STANDARD_REQUIRED True)

option(USE_CLANG "Using Clang to compain" ON)
if(USE_CLANG)		#默认使用clang++-11,G++ 9.3.0亦可
	set(CMAKE_CXX_COMPILER /usr/bin/clang++-11)
	message("-- Using Clang++ to build")
else()
	set(CMAKE_CXX_COMPILER /usr/bin/g++)
	message("-- Using g++ to build")
endif()

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

include(FindWEBSOCKETPP)
#相机
include(FindASI)
include(FindQHY)
include(FindGPHOTO2)
#赤道仪
include(SetTelescope)
#依赖库
include(FindIMGTOOLS)
include(FindJSONCPP)
include(FindNOVA)

########################################AstroAir-Server官方API文件########################################

add_library(LIBCAMERA src/air_camera.cpp)		#相机
target_link_libraries(airserver PUBLIC LIBCAMERA)
add_library(LIBMOUNT src/air_mount.cpp)			#赤道仪
target_link_libraries(airserver PUBLIC LIBMOUNT)
add_library(LIBSCRIPT src/air_script.cpp)			#脚本&序列
target_link_libraries(airserver PUBLIC LIBSCRIPT)
add_library(LIBLOGGER src/logger.cpp)
target_link_libraries(airserver PUBLIC LIBLOGGER)
add_library(LIBFOCUS src/air_focus.cpp)			#对焦
target_link_libraries(airserver PUBLIC LIBFOCUS)
add_library(LIBFILTER src/air_filter.cpp)			#滤镜轮
target_link_libraries(airserver PUBLIC LIBFILTER)
add_library(LIBSOLVER src/air_solver.cpp)		#解析
target_link_libraries(airserver PUBLIC LIBSOLVER)
add_library(LIBGUIDER src/air_guider.cpp)		#导星
target_link_libraries(airserver PUBLIC LIBGUIDER)
add_library(LIBSEARCH src/air_search.cpp)		#搜索
target_link_libraries(airserver PUBLIC LIBSEARCH)
add_library(LIBCOM src/telescope/air_com.cpp)		#COM
target_link_libraries(airserver PUBLIC LIBCOM)
#add_library(LIBAUTOFOCUS src/air_autofocus.cpp)			#自动对焦
#target_link_libraries(airserver PUBLIC LIBAUTOFOCUS)
add_library(LIBAUTOUPDATE src/tools/AutoUpdate.cpp)				#自动检查更新
target_link_libraries(airserver PUBLIC LIBAUTOUPDATE)

set(LINK_DIR /usr/lib)
link_directories(${LINK_DIR})
set(LINK_DIR /usr/local/lib)
link_directories(${LINK_DIR})

#头文件
target_include_directories(airserver PUBLIC
                           "${PROJECT_BINARY_DIR}/src"
                           )

#安装到系统
install(TARGETS airserver DESTINATION bin)
