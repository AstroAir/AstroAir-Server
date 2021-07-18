/*
 * main.cpp
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
 
Date:2021-2-15
 
Description:Main program of astroair server
 
**************************************************/

#include <thread>
#include <stdlib.h>

#include "logger.h"
#include "wsserver.h"

#include "air_search.h"
#include "air_camera.h"
#include "air_mount.h"
#include "air_solver.h"
#include "air_script.h"
#include "air_focus.h"
#include "air_filter.h"
#include "air_guider.h"

#include "gui/imgui.h"
#include "gui/imgui_impl_glfw.h"
#include "gui/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

/*
 * name: usage()
 * describe: Output help information
 * 描述：输出帮助信息
 * note: If you execute this function, the program will exit automatically
 */
void Usage(char *me)
{
    fprintf(stderr, _("Usage: %s [options]\n"), me);
    fprintf(stderr, _("Purpose: Server commands\n"));
	fprintf(stderr, _("AIILib Version:1.0.0 bate\n"));
	fprintf(stderr, _("Code: 1.0.0_unstable\n"));
    fprintf(stderr, _("Options:\n"));
	fprintf(stderr, _(" -g       : open gui\n"));
    fprintf(stderr, _(" -p p     : alternate IP port, default 5950\n"));
    exit(2);
}

void ShowAboutWindow(bool* p_open)
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

void ShowAppLog(bool* p_open)
{
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin(_("Log"), p_open);
	ImGui::End();
	//Log.Draw("Log", p_open);
}

void AppSaveSetting(int MaxThread,int MaxClient,int Timeout)
{
	if(MaxThread <= 0 || MaxThread > 10)
	{

	}
	AstroAir::SS->MaxThreadNumber = MaxThread;
	AstroAir::SS->MaxClientNumber = MaxClient;
	AstroAir::SS->MaxUsedTime = Timeout;
}

void CameraGUI(bool* p_open)
{
	IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");
	const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
	ImGuiWindowFlags window_flags = 0;
	if (!ImGui::Begin(AstroAir::AIRCAMINFO->Brand.c_str(), p_open, window_flags))
	{
		ImGui::End();
		return;
	}
	ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu(_("Menu")))
		{
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void WebServer(int port,bool IsSSL)
{
	AstroAir::ws.run(port);
}

void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

/*
 * name: main(int argc, char *argv[])
 * @prama argc:命令行参数数量
 * @prama *argv[]:命令行参数
 * describe: Principal function
 * @return:There is no practical significance
 * 描述：主函数
 */
int main(int argc, char *argv[])
{
	/*多语言支持*/
	setlocale(LC_ALL,"");
  	bindtextdomain(PACKAGE, "locale");
  	textdomain(PACKAGE);

	int WebPortal = 5950;

	char *optarg;
    int optind, opterr, optopt;
    int verbose = 0;
    int opt = -1;
    while ((opt = getopt(argc, argv, "p:g")) != -1) 
    {    
		switch (opt) 
		{    
			case 'p':
				WebPortal = atoi(optarg);
				break;
			case 'g':
				IsGUI = true;
				break;
			default:
				Usage(argv[0]);
				break;
		}
    }

	if(IsGUI)		//目前有未知段错误，无法进行测试
	{
		static bool show_app_help = false;
		static bool show_app_quit = false;
		static bool show_app_log = false;
		static bool show_app_ssl = false;
		static bool app_save_configure = false;

		static char port[64] = "";
		static char timeout[64] = "";
		static char maxclient[64] = "";
		static char maxthread[64] = "";

		glfwSetErrorCallback(glfw_error_callback);
		if (!glfwInit())
			return 1;

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
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		#endif

		GLFWwindow* window = glfwCreateWindow(1280, 720, _("AstroAir-Client"), NULL, NULL);
		if (window == NULL)
			return 1;
		glfwMakeContextCurrent(window);
		glfwSwapInterval(1); // Enable vsync

		//界面设置
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init();
		io.Fonts->AddFontFromFileTTF("misc/fonts/Roboto-Medium.ttf", 16.0f);

		//主窗口循环
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			ImGui_ImplOpenGL3_NewFrame();
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
			//启动服务器
			ImGui::InputText(_("Port"), port, 64, ImGuiInputTextFlags_CharsDecimal);
			ImGui::SameLine();
			ImGui::Checkbox("SSL", &show_app_ssl);
			if (ImGui::Button(_("Start")))
			{
				std::thread MainThread(WebServer, atoi(port), show_app_ssl);
				MainThread.detach();
			}

			if (ImGui::TreeNode(_("ServerSetting")))
        	{
				ImGui::InputText(_("MaxThread"), maxthread, 64, ImGuiInputTextFlags_CharsDecimal);
				ImGui::SameLine();
				ImGui::InputText(_("MaxClient"), maxclient, 64, ImGuiInputTextFlags_CharsDecimal);
				ImGui::InputText(_("Timeout"), timeout, 64, ImGuiInputTextFlags_CharsDecimal);
				ImGui::Checkbox(_("Save"), &app_save_configure);
				if (ImGui::Button(_("Save Setting")))
				{
					AppSaveSetting(atoi(maxthread),atoi(maxclient),atoi(timeout));
				}
				ImGui::TreePop();
            	ImGui::Separator();
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
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(window);
		}

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(window);
		glfwTerminate();
	}
	else
	{
		AstroAir::ws.run(WebPortal);
	}
    return 0;
}

