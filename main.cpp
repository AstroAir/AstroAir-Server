/*
 * main.cpp
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

/************************************************* 
 
Copyright: 2020 Max Qian. All rights reserved
 
Author:Max Qian

E-mail:astro_air@126.com
 
Date:2020-12-20
 
Description:Main program of astroair server
 
**************************************************/

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <thread>

#include "wsserver.h"

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
    AstroAir::WSSERVER ws;
    ws.run(port);
}

/*
 * name: stop_server()
 * describe: Stop the server
 * 描述：停止服务器
 */
void stop_server()
{
    AstroAir::WSSERVER ws;
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

