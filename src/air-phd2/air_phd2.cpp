/*
 * air_phd2.cpp
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

#include "air_phd2.h"
#include "../logger.h"

#include <poll.h>
#include <iostream>
#include <sstream>

namespace AstroAir
{
    Guide::Guide()
    {
        m_terminate = false;
    }

    Guide::~Guide()
    {

    }

    bool Guide::Connect(const char *hostname,int port)
    {
        Disconnect();
        m_terminate = false;
        m_curl = curl_easy_init();
        if (!m_curl)
            return false;
        std::ostringstream os;
        os << "http://" << hostname << ':' << port;
        curl_easy_setopt(m_curl, CURLOPT_URL, os.str().c_str());
        curl_easy_setopt(m_curl, CURLOPT_CONNECT_ONLY, 1L);
        try
        {
            CURLcode res = curl_easy_perform(m_curl);
            if (res != CURLE_OK)
                throw res;
            res = curl_easy_getinfo(m_curl, CURLINFO_ACTIVESOCKET, &m_sockfd);
            if (res != CURLE_OK)
                throw res;
        }
        catch (CURLcode)
        {
            curl_easy_cleanup(m_curl);
            m_curl = nullptr;
            return false;
        }
        m_terminate = false;
        m_worker = std::thread(&Guide::GuiderThread, this);
        return true;
    }

    bool Guide::Disconnect()
    {
        if (m_curl)
        {
            curl_easy_cleanup(m_curl);
            m_curl = nullptr;
        }
    }

    std::string Guide::read()
    {
        while (m_dq.empty())
        {
            char buf[1024];
            size_t nbuf;
            while (true)
            {
                CURLcode res = curl_easy_recv(m_curl, buf, sizeof(buf), &nbuf);
                if (res == CURLE_OK)
                    break;
                else if (res == CURLE_AGAIN)
                {
                    IDLog("%s", "waitreadable");
                    if (!WaitReadable())
                    {
                        IDLog("%s", "waitreadable ret false");
                        return "";
                    }
                }
                else
                {
                    // server disconnected
                    IDLog("Error: %s", curl_easy_strerror(res));
                    return "";
                }
            }
            const char *p0 = &buf[0];
            const char *p = p0;
            while (p < &buf[nbuf])
            {
                if (*p == '\r' || *p == '\n')
                {
                    m_os.write(p0, p - p0);
                    if (m_os.tellp() > 0)
                    {
                        m_dq.push_back(std::move(m_os.str()));
                        m_os.str("");
                    }
                    p0 = ++p;
                }
                else
                {
                    ++p;
                }
            }
            m_os.write(p0, p - p0);
        }
        std::string sret = std::move(m_dq.front());
        m_dq.pop_front();
        return sret;
    }

    bool Guide::WaitReadable()
    {
        struct pollfd pfd;
        pfd.fd = m_sockfd;
        pfd.events = POLLIN;
        while (!m_terminate)
        {
            int ret = poll(&pfd, 1, 500);
            if (ret == 1)
                return true;
        }
        return false;
    }

    bool Guide::send(std::string message)
    {
        size_t rem = message.size();
        const char *pos = message.c_str();
        while (rem > 0)
        {
            size_t nwr;
            CURLcode res = curl_easy_send(m_curl, pos, rem, &nwr);
            if (res == CURLE_AGAIN)
            {
                WaitWritable();
                continue;
            }
            if (res != CURLE_OK)
                return false;
            pos += nwr;
            rem -= nwr;
        }
        return true;
    }

    bool Guide::WaitWritable()
    {
        struct pollfd pfd;
        pfd.fd = m_sockfd;
        pfd.events = POLLOUT;
        int ret = poll(&pfd, 1, -1);
        return true;
    }

    void Guide::GuiderThread()
    {
        while (!m_terminate)
        {
            std::string line = read();
            if (line.empty())
                break;
            IDLog("L: %s", line.c_str());
            std::istringstream is(line);
            Json::Value j;
            try
            {
                is >> j;
            }
            catch (const std::exception& ex)
            {
                (ex); // suppress MSVC unused variable warning
                IDLog("ignoring invalid json from server: %s: %s", ex.what(), line.c_str());
                continue;
            }
            if (j.isMember("jsonrpc"))
            {
                // a response
                IDLog("R: %s", line.c_str());
                std::unique_lock<decltype(m_mutex)> _lock(m_mutex);
                m_response = j;
                m_cond.notify_one();
            }
            else
            {
                readJson(j);
            }
        }
    }

    /*以下三个函数均是用于switch支持string*/
	constexpr hash_t hash_compile_time(char const* str, hash_t last_value = basis)  
    {  
        return *str ? hash_compile_time(str+1, (*str ^ last_value) * prime) : last_value;  
    }  

    hash_t hash_(char const* str)  
    {  
        hash_t ret{basis};  
        while(*str){  
            ret ^= *str;  
            ret *= prime;  
            str++;  
        }  
        return ret;  
    }  
    
    constexpr unsigned long long operator "" _hash(char const* p, size_t)
    {
        return hash_compile_time(p);
    }

    void Guide::readJson(const Json::Value & message)
    {
        const std::string e = message["Event"].asString();
        switch (hash_(e.c_str()))
        {
            case "AppState"_hash:
                
                break;
            
            default:
                break;
        }
    }
}