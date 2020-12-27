/*
 * wsserver.h
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
 
Date:2020-12-25
 
Description:Main framework of astroair server
 
**************************************************/

#pragma once

#ifndef _WSSERVER_H_
#define _WSSERVER_H_

#define DebugMode true		//Debug模式是否开启

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <json/json.h>
#include <string>
#include <set>
#include <dirent.h>
#include <vector>
#include <atomic>
#include <fstream>

typedef websocketpp::server<websocketpp::config::asio> airserver;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
typedef airserver::message_ptr message_ptr;

namespace AstroAir
{
	class WSSERVER
	{
		public:
			/*WebSocket服务器主体函数*/
			explicit WSSERVER();
			~WSSERVER();
			void on_open(websocketpp::connection_hdl hdl);
			void on_close(websocketpp::connection_hdl hdl);
			void on_message(websocketpp::connection_hdl hdl,message_ptr msg);
			void send(std::string payload);
			void stop();
			virtual bool is_running();
			/*运行服务器*/
			void run(int port);
			/*转化Json信息*/
			void readJson(std::string message);	
			/*WebSocket服务器功能性函数*/
			void SetDashBoardMode();
			void GetAstroAirProfiles();
			void SetupConnect(int timeout);
			/*处理错误信息函数*/
			void UnknownMsg();
			void UnknownCamera();
			void UnknownMount();
			void Polling();
		private:
			Json::Value root;
			Json::String errs;
			Json::CharReaderBuilder reader;
			std::string method;
			std::string json_messenge;
			std::string camera,mount,focus,guide;
			std::string camera_name,mount_name,focus_name,guide_name;
			typedef std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> con_list;
			con_list m_connections;
			airserver m_server;
			
			std::atomic_bool isConnected;
	};
}

#endif
