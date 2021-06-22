/*
 * air_phd2.h
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

Date:2020-2-28

Description:PHD2 client

**************************************************/

#pragma once

#ifndef _AIR_PHD2_H_
#define _AIR_PHD2_H_

#include <string>
#include <curl/curl.h>
#include <json/json.h>
#include <atomic>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace AstroAir
{
    class Guide
    {
        public:
            explicit Guide();
            ~Guide();
            virtual bool Connect(const char *hostname,int port);
            virtual bool Disconnect();
            std::string read();
            virtual bool WaitReadable();
            virtual bool send(std::string message);
            virtual bool WaitWritable();
            void GuiderThread();
            void readJson(const Json::Value & message);
        private:
            typedef struct
            {
                double RawRA;
                double RawDEC;
                std::string State;
                std::string Version;
                bool active = false;
            } GuideInfo;
            CURL *m_curl;
            curl_socket_t m_sockfd;
            std::deque<std::string> m_dq;
            std::ostringstream m_os;
            std::atomic_bool m_terminate;
            std::thread m_worker;
            std::mutex m_mutex;
            std::condition_variable m_cond;
            Json::Value m_response;
    };
}

#endif