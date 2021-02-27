/*
 * aircom.h
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
 
Date:2021-2-27
 
Description:tty
 
**************************************************/

#pragma once

#ifndef _AIRCOM_H_
#define _AIRCOM_H_

#define MAXRBUF 2048

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

namespace AstroAir
{
    int tty_read(int fd, char *buf, int nbytes, int timeout, int *nbytes_read);
    int tty_read_section(int fd, char *buf, char stop_char, int timeout, int *nbytes_read);
    int tty_nread_section(int fd, char *buf, int nsize, char stop_char, int timeout, int *nbytes_read);
    int tty_write(int fd, const char *buffer, int nbytes, int *nbytes_written);
    int tty_write_string(int fd, const char *buffer, int *nbytes_written);
    int tty_connect(const char *device, int bit_rate, int word_size, int parity, int stop_bits, int *fd);
    int tty_disconnect(int fd);
    void tty_error_msg(int err_code, char *err_msg, int err_msg_len);
    void tty_set_debug(int debug);
    void tty_set_gemini_udp_format(int enabled);
    void tty_set_generic_udp_format(int enabled);
    void tty_clr_trailing_read_lf(int enabled);
    int tty_timeout(int fd, int timeout);
}

#endif