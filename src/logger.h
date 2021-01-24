/*
 * logger.h
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
 
Copyright: 2020-2021 Max Qian. All rights reserved
 
Author:Max Qian

E-mail:astro_air@126.com
 
Date:2021-1-4
 
Description:Log system of astroair server
 
**************************************************/

#pragma once

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <cstdarg>
#include <time.h>
#include <sys/time.h>

#ifdef WINDOWS
#include <windows.h>
#endif

typedef std::uint64_t hash_t;  
constexpr hash_t prime = 0x100000001B3ull;  
constexpr hash_t basis = 0xCBF29CE484222325ull;  

typedef struct {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
} TIME;

namespace AstroAir
{
	/*日志输出*/
	/*在终端输出*/
	void IDLog(const char *fmt, ...);
	/*输出服务器Debug日志*/
	void IDLog_DEBUG(const char *fmt, ...);
	/*输出客户端命令日志*/
	void IDLog_CMDL(std::string message);
	/*功能性函数*/
	const char *timestamp();		//获取时间戳
	/*获取CPU核心个数*/
	int GetCPUCores();
	/*设置本地系统时间*/
	bool setSystemTime(TIME *_time);
	/*获取本地系统时间*/
	TIME* getSystemLocalTime();
}

#endif
