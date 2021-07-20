/*
 * air_gui.cpp
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

#include "air_gui.h"
#include "logger.h"
#include "wsserver.h"

#include <thread>

static bool show_app_help = false;
static bool show_app_quit = false;
static bool show_app_log = false;
static bool show_app_ssl = false;
static bool show_app_devices = false;
static bool show_app_gui = false;

static char port[64] = "";
static char timeout[64] = "";
static char maxclient[64] = "";
static char maxthread[64] = "";
static char newpath[64] = "";
int WebServer = 5950;

#ifdef OpenGL2
    #define ImGui_ImplOpenGL_Init() ImGui_ImplOpenGL3_Init()
    #define ImGui_ImplOpenGL_NewFrame() ImGui_ImplOpenGL3_NewFrame()
    #define ImGui_ImplOpenGL_RenderDrawData(draw_data) ImGui_ImplOpenGL3_RenderDrawData(draw_data)
    #define ImGui_ImplOpenGL_Shutdown() ImGui_ImplOpenGL3_Shutdown()
#else
    #define ImGui_ImplOpenGL_Init() ImGui_ImplOpenGL2_Init()
    #define ImGui_ImplOpenGL_NewFrame() ImGui_ImplOpenGL2_NewFrame()
    #define ImGui_ImplOpenGL_RenderDrawData(draw_data) ImGui_ImplOpenGL2_RenderDrawData(draw_data)
    #define ImGui_ImplOpenGL_Shutdown() ImGui_ImplOpenGL2_Shutdown()
#endif

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

namespace AstroAir
{
    AIRGUI::AIRGUI()
    {
        
    }

    AIRGUI::~AIRGUI()
    {

    }

    bool AIRGUI::InitMainWindow()
    {
        glfwSetErrorCallback(glfw_error_callback);
		if (!glfwInit())
			return false;

		#ifdef OpenGL2
			#if defined(IMGUI_IMPL_OPENGL_ES2)
				// GL ES 2.0 + GLSL 100
				const char* glsl_version = "#version 100";
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
				glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
			#elif defined(__APPLE__)
				// GL 3.2 + GLSL 150
				const char* glsl_version = "#version 150";
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
				glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
			#else
				// GL 3.0 + GLSL 130
				const char* glsl_version = "#version 130";
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
			#endif
		#endif

        GLFWwindow* window = glfwCreateWindow(1280, 720, _("AstroAir-Client"), NULL, NULL);
        if (window == NULL)
			return 1;
		glfwMakeContextCurrent(window);
		glfwSwapInterval(1); // Enable vsync

        IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL_Init();
		io.Fonts->AddFontFromFileTTF("misc/fonts/Roboto-Medium.ttf", 16.0f);

        //主窗口循环
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			ImGui_ImplOpenGL_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			//const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
			//ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
			//ImGui::SetNextWindowSize(ImVec2(350, 250), ImGuiCond_FirstUseEver);

			if(!ImGui::Begin(_("Main Page")))                          // Create a window called "Hello, world!" and append into it.
			{
				ImGui::End();
				break;
			}
			//主菜单
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("Menu"))
				{
					ImGui::MenuItem(_("Qiut"), NULL, &show_app_quit);
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu(_("Setting")))
				{
					ImGui::MenuItem(_("Devices"), NULL, &show_app_devices);
					ImGui::MenuItem(_("GUI"), NULL, &show_app_gui);
					ImGui::MenuItem(_("Help"), NULL, &show_app_help);
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}

			if(show_app_quit)		//关程序
			{
				ImGui::End();
				break;
			}
			if(show_app_help)		//显示帮助信息
			{
				ShowAboutWindow(&show_app_help);
			}
			if(show_app_devices)
			{
				ShowDevicesWindow(&show_app_devices);
			}
			if(show_app_gui)
			{
				ShowGUIEidtorWindow(&show_app_gui);
			}

			//页面管理
			if (ImGui::TreeNode(_("PageManager")))
        	{
				ImGui::TreePop();
            	ImGui::Separator();
			}

			//网页服务器
			if (ImGui::TreeNode(_("WebServer")))
        	{
				//启动服务器
				ImGui::InputText(_("Port"), port, 64, ImGuiInputTextFlags_CharsDecimal);
				ImGui::SameLine();
				ImGui::Checkbox("SSL", &show_app_ssl);
				if (ImGui::Button(_("Start")))
				{
					std::thread MainThread(&AIRGUI::WebServer,this, atoi(port), show_app_ssl);
					MainThread.detach();
				}
				ImGui::Separator();
				if (ImGui::TreeNode(_("ServerSetting")))
				{
					ImGui::InputText(_("MaxThread"), maxthread, 64, ImGuiInputTextFlags_CharsDecimal);
					ImGui::SameLine();
					ImGui::InputText(_("MaxClient"), maxclient, 64, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputText(_("Timeout"), timeout, 64, ImGuiInputTextFlags_CharsDecimal);
					if (ImGui::Button(_("Save Setting")))
					{
						AppSaveSetting(atoi(maxthread),atoi(maxclient),atoi(timeout));
					}
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}

			ImGui::Checkbox(_("Log"), &show_app_log);

			if(show_app_log)
			{
				ShowAppLog(&show_app_log);
			}

			ImGui::End();

			ImGui::Render();
			int display_w, display_h;
			glfwGetFramebufferSize(window, &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);
			//glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(window);
		}

		ImGui_ImplOpenGL_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(window);
		glfwTerminate();
		return true;
    }

	void AIRGUI::ShowDevicesWindow(bool* p_open)
	{
		ImGui::SetNextWindowSize(ImVec2(500, 400));
		if (!ImGui::Begin(_("Devices Setting"), p_open, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::End();
            return;
        }

		static bool app_camera_asi = false;
		static bool app_camera_qhy = false;
		static bool app_camera_gphoto2 = false;
		static bool app_camera_indi_camera = false;
		if(ImGui::TreeNode(_("Camera")))
		{
			ImGui::SameLine();
			HelpMarker(_("Please choose both of camera's brand and name."));
			static int camera_brand_idx = -1;
			
			if (ImGui::Combo("Brand##Selector", &camera_brand_idx, "ZWOASI\0QHYCCD\0Gphoto2\0INDI\0NONE\0"))
			{
				switch (camera_brand_idx)
				{
					case 0: {
						app_camera_asi = true;
						app_camera_qhy = false;
						app_camera_gphoto2 = false;
						app_camera_indi_camera = false;
						break;
					} 
					case 1: {
						app_camera_asi = false;
						app_camera_qhy = true;
						app_camera_gphoto2 = false;
						app_camera_indi_camera = false;
						break;
					}
					case 2: {
						app_camera_asi = false;
						app_camera_qhy = false;
						app_camera_gphoto2 = true;
						app_camera_indi_camera = false;
						break;
					}
					case 3: {
						app_camera_asi = false;
						app_camera_qhy = false;
						app_camera_gphoto2 = false;
						app_camera_indi_camera = true;
						break;
					}
				}
			}

			//Max:这里需要补充相机列表
			if(app_camera_asi)
			{				
				const char* items[] = { "ZWO ASI120MM Mini", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
				static int item_current_idx = 0; // Here we store our selection data as an index.
				const char* combo_preview_value = items[item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
				static ImGuiComboFlags flags = 0;
				if (ImGui::BeginCombo("Name", combo_preview_value, flags))
				{
					for (int n = 0; n < IM_ARRAYSIZE(items); n++)
					{
						const bool is_selected = (item_current_idx == n);
						if (ImGui::Selectable(items[n], is_selected))
						{
							item_current_idx = n;
							device.Camera[0] = "ZWOASI";
							device.Camera[1] = items[item_current_idx];
						}
						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}
			if(app_camera_qhy)
			{				
				const char* items[] = { "ZWO ASI120MM Mini", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
				static int item_current_idx = 0; // Here we store our selection data as an index.
				const char* combo_preview_value = items[item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
				static ImGuiComboFlags flags = 0;
				if (ImGui::BeginCombo("Name", combo_preview_value, flags))
				{
					for (int n = 0; n < IM_ARRAYSIZE(items); n++)
					{
						const bool is_selected = (item_current_idx == n);
						if (ImGui::Selectable(items[n], is_selected))
						{
							item_current_idx = n;
							device.Camera[0] = "QHYCCD";
							device.Camera[1] = items[item_current_idx];
						}
						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}
			if(app_camera_gphoto2)
			{				
				const char* items[] = { "ZWO ASI120MM Mini", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
				static int item_current_idx = 0; // Here we store our selection data as an index.
				const char* combo_preview_value = items[item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
				static ImGuiComboFlags flags = 0;
				if (ImGui::BeginCombo("Name", combo_preview_value, flags))
				{
					for (int n = 0; n < IM_ARRAYSIZE(items); n++)
					{
						const bool is_selected = (item_current_idx == n);
						if (ImGui::Selectable(items[n], is_selected))
						{
							item_current_idx = n;
							device.Camera[0] = "Gphoto2";
							device.Camera[1] = items[item_current_idx];
						}
						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}
			if(app_camera_indi_camera)
			{				
				const char* items[] = { "ZWO ASI120MM Mini", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
				static int item_current_idx = 0; // Here we store our selection data as an index.
				const char* combo_preview_value = items[item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
				static ImGuiComboFlags flags = 0;
				if (ImGui::BeginCombo("Name", combo_preview_value, flags))
				{
					for (int n = 0; n < IM_ARRAYSIZE(items); n++)
					{
						const bool is_selected = (item_current_idx == n);
						if (ImGui::Selectable(items[n], is_selected))
						{
							item_current_idx = n;
							device.Camera[0] = "INDI";
							device.Camera[1] = items[item_current_idx];
						}
						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}
			ImGui::TreePop();
			ImGui::Separator();
		}

		static bool app_scope_ioptron = false;
		static bool app_scope_skywatcher = false;
		static bool app_scope_indi_scope = false;
		if(ImGui::TreeNode(_("Telescope")))
		{
			ImGui::SameLine();
			HelpMarker(_("Please choose both of telescope's brand and name."));
			static int scope_brand_idx = -1;

			if (ImGui::Combo("Brand##Selector", &scope_brand_idx, "iOptron\0SkyWatcher\0INDI\0NONE\0"))
			{
				switch (scope_brand_idx)
				{
					case 0: {
						app_scope_ioptron = true;
						app_scope_skywatcher = false;
						app_scope_indi_scope = false;
						break;
					}
					case 1: {
						app_scope_ioptron = false;
						app_scope_skywatcher = true;
						app_scope_indi_scope = false;
						break;
					}
					case 2: {
						app_scope_ioptron = false;
						app_scope_skywatcher = false;
						app_scope_indi_scope = true;
						break;
					}
				}
			}

			if(app_scope_ioptron)
			{				
				const char* items[] = { "Cube II EQ", "Smart EQ Pro+", "CEM25", "CEM25-EC", "iEQ30 Pro", "CEM40", "CEM40-EC", "GEM45", "GEM45-EC", "iEQ45 Pro EQ", "iEQ45 Pro AA", "CEM60", "CEM60-EC", "Cube II AA","AZ Mount Pro"};
				static int item_current_idx = 0; // Here we store our selection data as an index.
				const char* combo_preview_value = items[item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
				static ImGuiComboFlags flags = 0;
				if (ImGui::BeginCombo("Name", combo_preview_value, flags))
				{
					for (int n = 0; n < IM_ARRAYSIZE(items); n++)
					{
						const bool is_selected = (item_current_idx == n);
						if (ImGui::Selectable(items[n], is_selected))
						{
							item_current_idx = n;
							device.Scope[0] = "iOptron";
							device.Scope[1] = items[item_current_idx];
						}
						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}
			if(app_scope_skywatcher)
			{
				//这里需要补充赤道仪列表
				const char* items[] = { "EQ3", "EQ5","HEQ5","EQ6","AZEQ6","AZ-GTi","114GT","DOB"};
				static int item_current_idx = 0; // Here we store our selection data as an index.
				const char* combo_preview_value = items[item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
				static ImGuiComboFlags flags = 0;
				if (ImGui::BeginCombo("Name", combo_preview_value, flags))
				{
					for (int n = 0; n < IM_ARRAYSIZE(items); n++)
					{
						const bool is_selected = (item_current_idx == n);
						if (ImGui::Selectable(items[n], is_selected))
						{
							item_current_idx = n;
							device.Scope[0] = "SkyWatcher";
							device.Scope[1] = items[item_current_idx];
						}
						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			}
			ImGui::TreePop();
			ImGui::Separator();
		}

		if(ImGui::TreeNode(_("Focuser")))
		{
			ImGui::SameLine();
			HelpMarker(_("Just need to choose focuser's name."));
			const char* items[] = { "EAF","Grus","Moonlight","NONE"};
			static int item_current_idx = 0;						   // Here we store our selection data as an index.
			const char *combo_preview_value = items[item_current_idx]; // Pass in the preview value visible before opening the combo (it could be anything)
			static ImGuiComboFlags flags = 0;
			if (ImGui::BeginCombo("Name", combo_preview_value, flags))
			{
				for (int n = 0; n < IM_ARRAYSIZE(items); n++)
				{
					const bool is_selected = (item_current_idx == n);
					if (ImGui::Selectable(items[n], is_selected))
					{
						item_current_idx = n;
						device.Focuser = items[item_current_idx];
					}
					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::TreePop();
			ImGui::Separator();
		}

		if(ImGui::TreeNode(_("Filter")))
		{
			ImGui::SameLine();
			HelpMarker(_("Just need to choose filter's name."));
			const char* items[] = { "EFW","CFW","NONE"};
			static int item_current_idx = 0;						   // Here we store our selection data as an index.
			const char *combo_preview_value = items[item_current_idx]; // Pass in the preview value visible before opening the combo (it could be anything)
			static ImGuiComboFlags flags = 0;
			if (ImGui::BeginCombo("Name", combo_preview_value, flags))
			{
				for (int n = 0; n < IM_ARRAYSIZE(items); n++)
				{
					const bool is_selected = (item_current_idx == n);
					if (ImGui::Selectable(items[n], is_selected))
					{
						item_current_idx = n;
						device.Filter = items[item_current_idx];
					}
					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::TreePop();
			ImGui::Separator();
		}

		ImGui::InputText(_("Path"), newpath, 64, ImGuiInputTextFlags_CharsDecimal);
		ImGui::SameLine();
		HelpMarker(_("Max size is 64 bit."));
		ImGui::SameLine();
		if (ImGui::Button(_("Save")))
		{
			AppSaveNewDeviceConfigure(newpath);
		}
		ImGui::End();
	}

    void AIRGUI::ShowAboutWindow(bool* p_open)
    {
        if (!ImGui::Begin(_("About AstroAir"), p_open, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::End();
            return;
        }
        ImGui::Text("AstroAir %s", "1.0.0");
        ImGui::Separator();
        ImGui::Text(_("By Max Qian and members in QQ group."));
        ImGui::Text(_("Dear ImGui is licensed under the GPL3 License, see LICENSE for more information."));
        ImGui::End();
    }

	void AIRGUI::ShowGUIEidtorWindow(bool* p_open)
	{
		ImGui::SetNextWindowSize(ImVec2(500, 400));
        ImGui::Begin(_("GUI"), p_open);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);
		ShowStyleSelector("Colors##Selector");
        ImGui::End();
	}

	bool AIRGUI::ShowStyleSelector(const char* label)
	{
		static int style_idx = -1;
		if (ImGui::Combo(label, &style_idx, "Dark\0Light\0Classic\0"))
		{
			switch (style_idx)
			{
				case 0: ImGui::StyleColorsDark(); break;
				case 1: ImGui::StyleColorsLight(); break;
				case 2: ImGui::StyleColorsClassic(); break;
			}
			IDLog(_("Style changed successfully\n"));
			return true;
		}
		IDLog_Error(_("Could not set new style\n"));
		return false;
	}

    void AIRGUI::ShowAppLog(bool* p_open)
    {
        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin(_("Log"), p_open);
        ImGui::End();
        //Log.Draw("Log", p_open);
    }

    void AIRGUI::AppSaveSetting(int MaxThread,int MaxClient,int Timeout)
    {
        if(MaxThread <= 0 || MaxThread > 10)
        {
			IDLog_Error(_("The setting is unreasonable,please try again\n"));
			WebLog(_("The setting is unreasonable,please try again"),3);
        }
		else
		{
			SS->MaxThreadNumber = MaxThread;
			SS->MaxClientNumber = MaxClient;
			SS->MaxUsedTime = Timeout;
			std::ofstream out("config/config.json",std::ios::trunc);
			Json::Value NewProfile;
			NewProfile["ServerConfig"]["Timeout"] = Json::Value(SS->MaxUsedTime);
        	NewProfile["ServerConfig"]["MaxClientNum"] = Json::Value(SS->MaxClientNumber);
        	NewProfile["ServerConfig"]["MaxThreadNum"] = Json::Value(SS->MaxThreadNumber);
			out << NewProfile.toStyledString();
			out.close();
			IDLog(_("Save new configure properly\n"));
		}
    }

	void AIRGUI::AppSaveNewDeviceConfigure(std::string path)
	{
		std::ofstream out(path.c_str(),std::ios::trunc);
		Json::Value Root;
		if(!device.Camera[1].empty() && device.Camera[1] != "NONE")
		{
			Root["camera"]["Brand"] = Json::Value(device.Camera[0]);
			Root["camera"]["Name"] = Json::Value(device.Camera[1]);
			IDLog(_("Camera Brand is %s and name is %s\n"),device.Camera[0].c_str(),device.Camera[1].c_str());
		}
		if(!device.Scope[1].empty() && device.Scope[1] != "NONE")
		{
			Root["mount"]["Brand"] = Json::Value(device.Scope[0]);
			Root["mount"]["Name"] = Json::Value(device.Scope[1]);
			IDLog(_("Telescope brand is %s and name is %s\n"),device.Scope[0].c_str(),device.Scope[1].c_str());
		}
		if(!device.Focuser.empty())
		{
			Root["focus"]["Name"] = Json::Value(device.Focuser);
			IDLog(_("Focuser name is %s\n"),device.Focuser.c_str());
		}
		if(!device.Filter.empty())
		{
			Root["filter"]["Name"] = Json::Value(device.Filter);
			IDLog(_("Filter name is %s\n"),device.Filter.c_str());
		}
		out << Root.toStyledString();
		out.close();
		if(access(path.c_str(),F_OK) == -1)
		{
			IDLog_Error(_("Could not save new profile\n"));
		}
		else
		{
			IDLog(_("Save new profile in %s\n"),path.c_str());
		}
	}

    void AIRGUI::WebServer(int port,bool IsSSL)
    {
        AstroAir::ws.run(port);
    }
}