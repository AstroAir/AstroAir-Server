/*
 * air_gui.h
 * 
 * Copyright (C) 2020-2021 Max Qian
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
 
/************************************************* 
 
Copyright: 2020-2021 Max Qian. All rights reserved
 
Author:Max Qian

E-mail:astro_air@126.com
 
Date:2021-7-19
 
Description:Gui for AstroAir-Server

Using:Imgui <https://github.com/ocornut/imgui>

**************************************************/

#ifndef _AIR_GUI_H_
#define _AIR_GUI_H_

#include "config.h"

#include "gui/imgui.h"
#include "gui/imgui_impl_glfw.h"

#ifdef OpenGL2
    #include "gui/imgui_impl_opengl3.h"
#else
    #include "gui/imgui_impl_opengl2.h"
#endif

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string>

namespace AstroAir
{
    class AIRGUI
    {
        public:
            explicit AIRGUI();
            ~AIRGUI();
            virtual bool InitMainWindow();
            virtual void OpenCameraGUI() {}
            virtual void OpenScopeGUI() {}
            virtual void OpenFocuserGUI() {}
            virtual void OpenFilterGUI() {}
            virtual void OpenGuiderGUI() {}
        protected:
            
            void ShowDevicesWindow(bool* p_open);
            void ShowAboutWindow(bool* p_open);
            void ShowGUIEidtorWindow(bool* p_open);
            bool ShowStyleSelector(const char* label);
            void ShowAppLog(bool* p_open);
            void AppSaveSetting(int MaxThread,int MaxClient,int Timeout);
            void AppSaveNewDeviceConfigure(std::string path);
            void WebServer(int port,bool IsSSL);
        private:
            struct DeviceInfo
            {
                std::string Camera[2];
                std::string Scope[2];
                std::string Focuser;
                std::string Filter;
                std::string Guider;
            }device;
            
    };
}

#endif