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
#include "air_gui.h"
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

	if(IsGUI)
	{
		AstroAir::AIRGUI GUI;
		GUI.InitMainWindow();
	}
	else
	{
		AstroAir::ws.run(WebPortal);
	}
    return 0;
}

