/*
 * air_solver.h
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

#include <string>
#include <atomic>

namespace AstroAir
{
    /*
        这是服务器的解析模块
        分为离线和在线解析
        且均会有参数返回客户端
    */
    class AIRSOLVER
    {
        public:
            explicit AIRSOLVER();
            ~AIRSOLVER();
            /*解析：离线*/
            void SolveActualPosition(bool IsBlind,bool IsSync);     //开始解析
            void SolveActualPositionSuccess();  //解析成功
            void SolveActualPositionError();    //解析失败
            /*解析：在线*/
            void SolveActualPositionOnline(bool IsBlind,bool IsSync);
        private:
            std::string message;
            std::atomic_bool IsSolving;
    };
    extern AIRSOLVER *SOLVER;
    extern std::atomic_bool isSolverConnected;
}