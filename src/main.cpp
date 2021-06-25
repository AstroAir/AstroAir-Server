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

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <thread>

#include "wsserver.h"
#include "check.h"
#include "logger.h"

using namespace AstroAir;

#define AIRPORT 5950
int port = AIRPORT;

/*
 * name: usage()
 * describe: Output help information
 * 描述：输出帮助信息
 * note: If you execute this function, the program will exit automatically
 */
void usage(char *me)
{
    fprintf(stderr, "Usage: %s [options]\n", me);
    fprintf(stderr, "Purpose: Start or stop the server\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, " -v       : start server\n");
	fprintf(stderr, " -s       : stop server\n");
    fprintf(stderr, " -p p     : alternate IP port, default %d\n", AIRPORT);
	fprintf(stderr, " -c       : write a configure file for server\n");
    exit(2);
}

/*
 * name: PrintLogo()
 * describe: Output Logo
 * 描述：显示Logo
 */
void PrintLogo()
{
	std::cout << "    _        _               _    _          ____" << std::endl;
	std::cout << "   / \\   ___| |_ _ __ ___   / \\  (_)_ __    / ___|  ___ _ ____   _____ _ __" << std::endl;
	std::cout << "  / _ \\ / __| __| '__/ _ \\ / _ \\ | | '__|___\\___ \\ / _ \\ '__\\ \\ / / _ \\ '__|" << std::endl;
    std::cout << " / ___ \\__ \\ |_| | | (_) / ___ \\| | | |_____|__) |  __/ |   \\ V /  __/ |" << std::endl;
    std::cout << "/_/   \\_\\___/\\__|_|  \\___/_/   \\_\\_|_|      |____/ \\___|_|    \\_/ \\___|_|" << std::endl;
}

/*
 * name: start_server()
 * describe: Start the server according to different ports
 * 描述：依据不同端口启动服务器
 */
void start_server()
{
    ws.run(port);
}

/*
 * name: stop_server()
 * describe: Stop the server
 * 描述：停止服务器
 */
void stop_server()
{
    ws.stop();
}

/*
 * name: configure()
 * describe: Write config file
 * 描述：编写配置文件
 */
void configure()
{
	std::string name,name_camera,name_mount,name_focus,name_filter,name_guide,name_guide_camera;
	std::string brand_camera,brand_mount,brand_focus,brand_filter;
	std::cout << "Please input the name of the configure file:";
	std::cin >> name;
	std::cout << "Please input the brand of the camera:";
	std::cin >> brand_camera;
	std::cout << "Please input the name of camera:";
	std::cin >> name_camera;
	std::cout << "Please input the brand of the mount:";
	std::cin >> brand_mount;
	std::cout << "Please input the name of mount:";
	std::cin >> name_mount;
	std::cout << "Please input the brand of the focus:";
	std::cin >> brand_focus;
	std::cout << "Please input the name of focus:";
	std::cin >> name_focus;
	std::cout << "Please input the brand of the filter:";
	std::cin >> brand_filter;
	std::cout << "Please input the name of filter:";
	std::cin >> name_filter;
	std::cout << "Please input the name of guide software:";
	std::cin >> name_guide;
	std::cout << "Please input the name of guide camera name:";
	std::cin >> name_guide_camera;
	Json::Value Root;
	Root["camera"]["brand"] = Json::Value(brand_camera);
	Root["camera"]["name"] = Json::Value(name_camera);
	Root["mount"]["brand"] = Json::Value(brand_mount);
	Root["mount"]["name"] = Json::Value(name_mount);
	Root["focus"]["brand"] = Json::Value(brand_focus);
	Root["focus"]["name"] = Json::Value(name_focus);
	Root["filter"]["brand"] = Json::Value(brand_filter);
	Root["filter"]["name"] = Json::Value(name_filter);
	Root["guide"]["brand"] = Json::Value(name_guide);
	Root["guide"]["name"] = Json::Value(name_guide_camera);
	std::string file = Root.toStyledString();
	std::string ok;
	if(access( name.c_str(), F_OK ) != -1)
	{
		std::cout << "Do you want cover the old file with the new one?[Y/n]";
		std::cin >> ok;
		if(ok == "Y")
		{
			std::string command = "sudo rm ";
			command += name;
			system(command.c_str());
		}
		else
		{
			std::cout << "Please input a new file name:";
			std::cin >> name;
		}
	}
	std::ofstream out;
	out.open(name,std::ios::out);
	out << file;
	out.close();
	std::cout << "Write new config file named " << name << "successfully" << std::endl;
	std::cout << "Do you want to start server right now?[Y/n]";
	std::cin >> ok;
	if(ok == "Y")
	{
		std::thread t1(start_server);
		t1.join();
	}
	else
	{
		exit(1);
	}
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
	setlocale(LC_ALL,"");
  	bindtextdomain(PACKAGE, "locale");
  	textdomain(PACKAGE);
	/*输出Logo*/
	PrintLogo();
	//AutoCheckVersion();
	char *optarg;
    int optind, opterr, optopt;
    int verbose = 0;
    int opt = -1;
    while ((opt = getopt(argc, argv, "vp:sc")) != -1) 
    {    
		switch (opt) 
		{    
			case 'v':{
				std::thread t1(start_server);
				t1.join();
				sleep(1);
				break;
			}
			case 'p':
				port = atoi(optarg);
				break;
			case 's':
				stop_server();
				break;
			case 'c':
				configure();
				break;
			default:
				usage(argv[0]);
		}
    }
    return 0;
}

