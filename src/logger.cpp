/*
 * logger.cpp <Hangzhou@astroair.cn>
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

#include <fstream>
#include <thread>
#include <cstring>
#include <string.h>

#include "logger.h"

namespace AstroAir
{
	/*
	 * name: IDLog(const char *fmt, ...)
	 * @param fmt：任意形式的字符输入
	 * describe: Output program usage log
	 * 描述：输出程序使用日志
	 * calls:timestamp()
	 */
	void IDLog(const char *fmt, ...)		//日志输出
	{
		va_list ap;
		fprintf (stderr, "%s ", timestamp());
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	}
		
	/*
	 * name: IDLog_DEBUG(const char *fmt, ...)
	 * @param fmt：任意形式的字符输入
	 * describe: Output server debug log (mainly websocket log)
	 * 描述：输出服务器Debug日志（主要为WebSocket日志）
	 */
	void IDLog_DEBUG(const char *fmt, ...)
	{
		std::ofstream outlog;
		outlog.open("debug.txt", std::ios::app);
		if(outlog.is_open())
		{
			outlog << fmt;
			outlog.close();
		}
		else
		{
			IDLog("Unable to open debug log\n");
		}
	}
	
	/*
	 * name: IDLog_CMDL(std::string message)
	 * @param message：客户端发送信息
	 * describe: Output the command from the client
	 * 描述：输出客户端传来的命令
	 */
	void IDLog_CMDL(std::string message)
	{
		std::ofstream outlog;
		outlog.open("comdline.txt",std::ios::app);
		if(outlog.is_open())
		{
			outlog << message;
			outlog.close();
		}
		else
		{
			IDLog("Unable to open cmdline log\n");
		}
	}
	
	/*
	 * name: timestamp()
	 * @return ts:系统当前时间
	 * describe: Get server time
	 * 描述：获取服务器时间
	 * note: The time is in the UTF-8 time zone
	 */
	const char *timestamp()			//获取当前时间
	{
		static char ts[32];
		struct tm *tp;
		time_t t;

		time(&t);
		tp = localtime(&t);
		strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tp);
		return (ts);
	}	
	
	/*
	 * name: timestampW()
	 * @return ts:系统当前时间
	 * describe: Get server time
	 * 描述：获取服务器时间
	 * note: The time is in the UTF-8 time zone
	 */
	const char *timestampW()			//获取当前时间
	{
		static char ts[32];
		struct tm *tp;
		time_t t;

		time(&t);
		tp = localtime(&t);
		strftime(ts, sizeof(ts), "%H:%M:%S", tp);
		return (ts);
	}	

	/*
	 * name: GetCPUCores()
	 * @return n:CPU核心个数
	 * describe: Get cpu cores number
	 * 描述：获取CPU核心个数
	 */
	int GetCPUCores()
	{
		auto n = std::thread::hardware_concurrency();
		return n;
	}
	
	/*
	 * name: setSystemTime(TIME *_time)
	 * @return ts:需要设置的时间
	 * describe: Set system local time
	 * 描述：设置系统本地时间
	 * note: The time is in the UTF-8 time zone
	 */
	bool setSystemTime(TIME *_time)
	{
		struct tm *p = new struct tm();
		struct timeval tv;
		struct timezone tz;
		gettimeofday (&tv , &tz);//获取时区保存tz中
		p->tm_year = _time->year - 1900;
		p->tm_mon = _time->month - 1;
		p->tm_mday = _time->day;
		p->tm_hour = _time->hour;
		p->tm_min = _time->minute;
		p->tm_sec = _time->second;
		time_t utc_t = mktime(p);
		delete(p);
		tv.tv_sec = utc_t;
		tv.tv_usec = 0;
		if (settimeofday(&tv, &tz) < 0)
		{
			return false;
		}
		return true;
	}

	StringList splitstr(const std::string& str, char tag)
	{
		StringList  li;
		std::string subStr;
		for(size_t i = 0; i < str.length(); i++)
		{
			if(tag == str[i]) //完成一次切割
			{
				if(!subStr.empty())
				{
					li.push_back(subStr);
					subStr.clear();
				}
			}
			else
			{
				subStr.push_back(str[i]);
			}
		}
		if(!subStr.empty())
		{
			li.push_back(subStr);
		}
		return li;
	}
}
