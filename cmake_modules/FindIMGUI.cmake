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
 
#  Date:2021-7-4
 
#  Description: Dear Imgui Library

################################################## 

option(IMGUI "Dear Imgui Library" ON)

if(IMGUI)
    find_path(IMGUI_GL glfw3.h /usr/include/GLFW)
    find_path(IMGUI_GL glfw3.h /usr/local/include/GLFW)
    find_library(IMGUI_GL_LIB libglfw.so /usr/lib)
    find_library(IMGUI_GL_LIB libglfw.so /usr/local/include)
    if(IMGUI_GL AND IMGUI_GL_LIB)
        message("-- Found GL header file in ${IMGUI_GL} and library in ${IMGUI_GL_LIB}")
    else()
        message("-- Could not found GL library.Please build it before intall!")
        add_custom_command(
			TARGET airserver
			PRE_BUILD 
			COMMAND sudo apt install libglfw3-dev libgl-dev -y
			COMMENT "Downloaded and Building OpenGL Library"
		)
    endif()
    add_library(IMGUI src/gui/imgui.cpp src/gui/imgui_draw.cpp src/gui/imgui_tables.cpp src/gui/imgui_widgets.cpp src/gui/imgui_impl_glfw.cpp src/gui/imgui_impl_opengl2.cpp)
    target_link_libraries(airserver PUBLIC IMGUI)
    target_link_libraries(airserver PUBLIC libglfw.so)
    target_link_libraries(airserver PUBLIC libGL.so)
endif()