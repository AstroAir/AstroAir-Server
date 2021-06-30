/*
 * TcpSocket.cpp
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
 
Date:2021-6-29
 
Description:Socket Tools
 
**************************************************/

#include "TcpSocket.h"
#include "../logger.h"

#define BUFFER_SIZE 1024

int sock_cli;
fd_set rfds;
struct timeval tv;
int retval, maxfd;
struct sockaddr_in servaddr;
bool isTCPConnected;

namespace AstroAir
{
    TCP::TCP()
    {
        
    }
    
    TCP::~TCP()
    {
        if(isTCPConnected)
            Disconnect();
    }

    bool TCP::Connect(const char * TcpHost,int TcpPort)
    {
        sock_cli = socket(AF_INET,SOCK_STREAM, 0);
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(TcpPort);  ///服务器端口
        servaddr.sin_addr.s_addr = inet_addr(TcpHost);  ///服器ip
        if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            IDLog_Error(_("Failed to connect to TCP Server\n"));
            return false;
        }
        isTCPConnected = true;
        return true;
    }

    bool TCP::Disconnect()
    {
        isTCPConnected = false;
        close(sock_cli);
        return true;
    }

    std::string TCP::ReadMessage()
    {
        std::string message;
        /*把可读文件描述符的集合清空*/
        FD_ZERO(&rfds);
        /*把标准输入的文件描述符加入到集合中*/
        FD_SET(0, &rfds);
        maxfd = 0;
        /*把当前连接的文件描述符加入到集合中*/
        FD_SET(sock_cli, &rfds);
        /*找出文件描述符集合中最大的文件描述符*/   
        if(maxfd < sock_cli)
            maxfd = sock_cli;
        /*设置超时时间*/
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        /*等待聊天*/
        retval = select(maxfd+1, &rfds, NULL, NULL, &tv);
        if(retval == -1)
        {
            IDLog_Error(_("Failed to read message from server\n"));
            return 0;
        }
        else
        {
            if(retval == 0)     //服务器无消息
                return "No message";
            else
            {
                if(FD_ISSET(sock_cli,&rfds))
                {
                    
                    char recvbuf[BUFFER_SIZE];
                    int len;
                    len = recv(sock_cli, recvbuf, sizeof(recvbuf),0);
                    message = recvbuf;
                    memset(recvbuf, 0, sizeof(recvbuf));
                    
                }
            }
        }
        return message;
    }

    bool TCP::SendMessage(const char * message)
    {
        if (FD_ISSET(0, &rfds))
        {
            send(sock_cli, message, strlen(message), 0);
        }
        return true;
    }
}