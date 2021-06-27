/*
 * air_com.h
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
 
Date:2021-6-27
 
Description:TTY Port

**************************************************/

#ifndef _AIR_COM_H_
#define _AIR_COM_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdint.h>

namespace AstroAir
{
    enum TTY_ERROR
    {
        TTY_OK           = 0,
        TTY_READ_ERROR   = -1,
        TTY_WRITE_ERROR  = -2,
        TTY_SELECT_ERROR = -3,
        TTY_TIME_OUT     = -4,
        TTY_PORT_FAILURE = -5,
        TTY_PARAM_ERROR  = -6,
        TTY_ERRNO        = -7,
        TTY_OVERFLOW     = -8,
        TTY_PORT_BUSY    = -9,
    };

    class AIRCOM
    {
        public:
            explicit AIRCOM();
            ~AIRCOM();
            virtual bool Open(const char *device, int bit_rate, int word_size, int parity, int stop_bits, int *fd);
            virtual int Close(int fd);
            virtual int Initialization(int speed);
            virtual int Read(int fd, char *buf, int nbytes, int timeout, int *nbytes_read);
            virtual int ReadSection(int fd, char *buf, char stop_char, int timeout, int *nbytes_read);
            virtual int ReadNSection(int fd, char *buf, int nsize, char stop_char, int timeout, int *nbytes_read);
            virtual int Write(int fd, const char *buf, int nbytes, int *nbytes_written);
            virtual int WriteString(int fd, const char *buf, int *nbytes_written);
            virtual int COM_Timeout(int fd, int timeout);
            virtual void COM_ERRORMSG(int err_code, char *err_msg, int err_msg_len);
            //int m_PortFD;
        private:
            
            int speed_arr[15] = { B38400, B19200, B9600, B4800, B2400, B1200, B300,B38400, B19200, B9600, B4800, B2400, B1200, B300,B115200};
            int name_arr[15] = {38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,19200,  9600, 4800, 2400, 1200,  300,115200};
    };

    /*以下函数为数据处理模块*/
    double rangeHA(double r);
    double range24(double r);
    double range360(double r);
    double rangeDec(double decdegrees);

}

#endif