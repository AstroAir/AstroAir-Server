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

using namespace AstroAir;

#define AIRPORT 5950

char *me = new char [100];
int port = AIRPORT;

/*
 * name: usage()
 * describe: Output help information
 * 描述：输出帮助信息
 * note: If you execute this function, the program will exit automatically
 */
void usage()
{
    fprintf(stderr, "Usage: %s [options]\n", me);
    fprintf(stderr, "Purpose: Start or stop the server\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, " -v       : show key events, no traffic\n");
    fprintf(stderr, " -p p     : alternate IP port, default %d\n", AIRPORT);
    exit(2);
}

/*
 * name: start_server()
 * describe: Start the server according to different ports
 * 描述：依据不同端口启动服务器
 */
void start_server()
{
    WSSERVER ws;
    ws.run(port);
}

/*
 * name: stop_server()
 * describe: Stop the server
 * 描述：停止服务器
 */
void stop_server()
{
    WSSERVER ws;
    ws.stop();
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
	/*输出Logo*/
	std::cout << "    _        _               _    _          ____" << std::endl;
	std::cout << "   / \\   ___| |_ _ __ ___   / \\  (_)_ __    / ___|      _                 _" << std::endl;
	std::cout << "  / _ \\ / __| __| '__/ _ \\ / _ \\ | | '__|___\\___ \\ / _ \\ '__\\ \\ / / _ \\ '__|" << std::endl;
    std::cout << " / ___ \\__ \\ |_| | | (_) / ___ \\| | | |_____|__) |  __/ |   \\ V /  __/ |" << std::endl;
    std::cout << "/_/   \\_\\___/\\__|_|  \\___/_/   \\_\\_|_|      |____/ \\___|_|    \\_/ \\___|_|" << std::endl;
    me = argv[0];
    extern char *optarg;
    extern int optind, opterr, optopt;
    int verbose = 0;
    int opt = -1;
    while ((opt = getopt(argc, argv, "vp:s")) != -1) 
    {    
	switch (opt) {    
	    case 'v':
		verbose = 1;
		break;
	    case 'p':
		port = atoi(optarg);
		break;
	    case 's':
		verbose = 2;
		break;
	    default:
		usage();
	}
    }
    switch(verbose)
    {
	case 1:{
	    std::thread t1(start_server);
	    t1.join();
	    break;
	}
	case 2:
	    stop_server();
	    break;
	default:
	    usage();
    }
    return 0;
}

