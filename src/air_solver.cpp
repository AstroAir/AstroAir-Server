/*
 * air_solver.cpp
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
 
Date:2021-6-24
 
Description:Solve engine

**************************************************/

#include "air_solver.h"
#include "logger.h"
#include "wsserver.h"
#include "air_camera.h"

namespace AstroAir
{
    AIRSOLVER *SOLVER;
    std::atomic_bool isSolverConnected;

    /*
     * name: AIRSOLVER()()
     * describe: Constructor for initializing solver parameters
     * 描述：构造函数，用于初始化解析器参数
     */
    AIRSOLVER::AIRSOLVER()
    {
        if(access( "/usr/bin/solve-field", F_OK ) == -1 && access( "/usr/local/bin/solve-field", F_OK ) == -1)
            isSolverConnected = false;
        else
            isSolverConnected = true;
        IsSolving = false;
    }

    /*
     * name: ~AIRSOLVER()
     * describe: Destructor
     * 描述：析构函数
     */
    AIRSOLVER::~AIRSOLVER()
    {
        IsSolving = false;
    }

    /*
	 * name: SolveActualPosition(bool IsBlind,bool IsSync)
     * @param isBlind:是否为盲解析
     * @param IsSync：是否同步
	 * describe: Start the parser
	 * 描述：启动解析器
     * calls: IDLog()
     * calls: SolveActualPositionSuccess()
	 * calls: SolveActualPositionError()
	 */
    void AIRSOLVER::SolveActualPosition(bool IsBlind,bool IsSync)
    {
        if(!isSolverConnected)
        {
            IDLog_Error(_("Not found any avilible solver\n"));
            WebLog(_("Couldn't found solver"),2);
            SolveActualPositionError();
            return ;
        }
        IsSolving = true;
        char cmd[2048] = {0},line[256]={0},parity_str[8]={0};
        int UsedTime = 0;
        float ra = -1000, dec = -1000, angle = -1000, pixscale = -1000, parity = 0;
        snprintf(cmd,2048,"solve-field %s --guess-scale --downsample 2 --ra %s --dec %s --radius 5 ",CameraImageName.c_str(),TargetRA.c_str(),TargetDEC.c_str());
        IDLog("Run:%s",cmd);
        FILE *handle = popen(cmd, "r");
        if (handle == nullptr)
        {
            IDLog_Error(_("Could not solve this image,the error code is %s\n"),strerror(errno));
            return;
        }
        while (fgets(line, sizeof(line), handle) != nullptr && UsedTime <= MaxUsedTime && IsSolving == true)
        {
            UsedTime++;
            IDLog("%s", line);
            sscanf(line, "Field rotation angle: up is %f", &angle);
            sscanf(line, "Field center: (RA,Dec) = (%f,%f)", &ra, &dec);
            sscanf(line, "Field parity: %s", parity_str);
            sscanf(line, "%*[^p]pixel scale %f", &pixscale);
            if (strcmp(parity_str, "pos") == 0)
                parity = 1;
            else if (strcmp(parity_str, "neg") == 0)
                parity = -1;
            if (ra != -1000 && dec != -1000 && angle != -1000 && pixscale != -1000)
            {
                // Astrometry.net angle, E of N
                MountAngle = angle;
                // Astrometry.net J2000 RA in degrees
                TargetRA = ra;
                // Astrometry.net J2000 DEC in degrees
                TargetDEC = dec;
                fclose(handle);
                IDLog(_("Solver complete."));
                SolveActualPositionSuccess();
                IsSolving = false;
                return;
            }
        }
        fclose(handle);
        IsSolving = false;
        SolveActualPositionError();
    }

    /*
     * name: SolveActualPositionSuccess()
     * describe: Return parsing information to client
     * 描述：向客户端返回解析信息
     * calls: send()
     */
    void AIRSOLVER::SolveActualPositionSuccess()
    {
        Json::Value Root;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("sendRemoteSolveNoSync");
        Root["ActionResultInt"] = Json::Value(4);
        Root["ParamRet"]["RA"] = Json::Value(TargetRA);
        Root["ParamRet"]["DEC"] = Json::Value(TargetDEC);
        Root["ParamRet"]["PA"] = Json::Value(MountAngle);
        Root["ParamRet"]["IsSolved"] = Json::Value("Completed");
        ws.send(Root.toStyledString());
    }

    /*
     * name:SolveActualPositionError()
     * describe: Return parsing information to client
     * 描述：向客户端返回解析信息
     * calls: send()
     */
    void AIRSOLVER::SolveActualPositionError()
    {
        Json::Value Root;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("sendRemoteSolveNoSync");
        Root["ActionResultInt"] = Json::Value(5);
        Root["Motivo"] = Json::Value("Could not solve image!");
        ws.send(Root.toStyledString());
    }

    /*
	 * name: SolveActualPositionOnline(bool IsBlind,bool IsSync)
     * @param isBlind:是否为盲解析
     * @param IsSync：是否同步
	 * describe: Start the parser
	 * 描述：启动解析器
	 */
    void AIRSOLVER::SolveActualPositionOnline(bool IsBlind,bool IsSync)
    {
        //这一个功能还有待考量，如果需要注册账号则不与添加！！！
        
    }
}