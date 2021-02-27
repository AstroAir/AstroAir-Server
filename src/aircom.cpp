/*
 * aircom.cpp
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
 
Description: tty
 
**************************************************/

#include "aircom.h"
#include "logger.h"

#include <string.h>
#include <unistd.h>

static int tty_debug = 0;
static int tty_gemini_udp_format = 0;
static int tty_generic_udp_format = 0;
static int tty_sequence_number = 1;
static int tty_clear_trailing_lf = 0;

namespace AstroAir
{
    void tty_set_debug(int debug)
    {
        tty_debug = debug;
    }

    void tty_set_gemini_udp_format(int enabled)
    {
        tty_gemini_udp_format = enabled;
    }

    void tty_set_generic_udp_format(int enabled)
    {
        tty_generic_udp_format = enabled;
    }

    void tty_clr_trailing_read_lf(int enabled)
    {
        tty_clear_trailing_lf = enabled;
    }

    int tty_timeout(int fd, int timeout)
    {
        #if defined(_WIN32) || defined(ANDROID)
            INDI_UNUSED(fd);
            INDI_UNUSED(timeout);
            return TTY_ERRNO;
        #else

            if (fd == -1)
                return TTY_ERRNO;

            struct timeval tv;
            fd_set readout;
            int retval;

            FD_ZERO(&readout);
            FD_SET(fd, &readout);

            /* wait for 'timeout' seconds */
            tv.tv_sec  = timeout;
            tv.tv_usec = 0;

            /* Wait till we have a change in the fd status */
            retval = select(fd + 1, &readout, NULL, NULL, &tv);

            /* Return 0 on successful fd change */
            if (retval > 0)
                return TTY_OK;
            /* Return -1 due to an error */
            else if (retval == -1)
                return TTY_SELECT_ERROR;
            /* Return -2 if time expires before anything interesting happens */
            else
                return TTY_TIME_OUT;

        #endif
    }

    int tty_write(int fd, const char *buf, int nbytes, int *nbytes_written)
    {
        #ifdef _WIN32
            return TTY_ERRNO;
        #else
            int geminiBuffer[66]={0};
            char *buffer = (char *)buf;

            if (tty_gemini_udp_format)
            {
                buffer = (char*)geminiBuffer;
                geminiBuffer[0] = ++tty_sequence_number;
                geminiBuffer[1] = 0;
                memcpy((char *)&geminiBuffer[2], buf, nbytes);
                // Add on the 8 bytes for the header and 1 byte for the null terminator
                nbytes += 9;
            }

            if (fd == -1)
                return TTY_ERRNO;

            int bytes_w     = 0;
            *nbytes_written = 0;

            if (tty_debug)
            {
                int i = 0;
                for (i = 0; i < nbytes; i++)
                    IDLog("%s: buffer[%d]=%#X (%c)\n", __FUNCTION__, i, (unsigned char)buf[i], buf[i]);
            }

            while (nbytes > 0)
            {
                bytes_w = write(fd, buffer + (*nbytes_written), nbytes);

                if (bytes_w < 0)
                    return TTY_WRITE_ERROR;

                *nbytes_written += bytes_w;
                nbytes -= bytes_w;
            }

            if (tty_gemini_udp_format)
                *nbytes_written -= 9;

            return TTY_OK;

        #endif
    }

    int tty_write_string(int fd, const char *buf, int *nbytes_written)
    {
        unsigned int nbytes;
        nbytes = strlen(buf);

        return tty_write(fd, buf, nbytes, nbytes_written);
    }

    int tty_read(int fd, char *buf, int nbytes, int timeout, int *nbytes_read)
    {
        #ifdef _WIN32
            return TTY_ERRNO;
        #else

            if (fd == -1)
                return TTY_ERRNO;

            int numBytesToRead =  nbytes;
            int bytesRead = 0;
            int err       = 0;
            *nbytes_read  = 0;

            if (nbytes <= 0)
                return TTY_PARAM_ERROR;

            if (tty_debug)
                IDLog("%s: Request to read %d bytes with %d timeout for fd %d\n", __FUNCTION__, nbytes, timeout, fd);

            char geminiBuffer[257]={0};
            char* buffer = buf;

            if (tty_gemini_udp_format)
            {
                numBytesToRead = nbytes + 8;
                buffer = geminiBuffer;
            }

            while (numBytesToRead > 0)
            {
                if ((err = tty_timeout(fd, timeout)))
                    return err;

                bytesRead = read(fd, buffer + (*nbytes_read), ((uint32_t)numBytesToRead));

                if (bytesRead < 0)
                    return TTY_READ_ERROR;

                if (tty_debug)
                {
                    IDLog("%d bytes read and %d bytes remaining...\n", bytesRead, numBytesToRead - bytesRead);
                    int i = 0;
                    for (i = *nbytes_read; i < (*nbytes_read + bytesRead); i++)
                        IDLog("%s: buffer[%d]=%#X (%c)\n", __FUNCTION__, i, (unsigned char)buf[i], buf[i]);
                }

                if (*nbytes_read == 0 && tty_clear_trailing_lf && *buffer == 0x0A)
                {
                    if (tty_debug)
                        IDLog("%s: Cleared LF char left in buf\n", __FUNCTION__);

                    memcpy(buffer, buffer+1,bytesRead);
                    --bytesRead;
                }

                *nbytes_read += bytesRead;
                numBytesToRead -= bytesRead;
            }


            if (tty_gemini_udp_format)
            {
                int *intSizedBuffer = (int *)geminiBuffer;
                if (intSizedBuffer[0] != tty_sequence_number)
                {
                    // Not the right reply just do the read again.
                    return tty_read(fd, buf, nbytes, timeout, nbytes_read);
                }

                *nbytes_read -= 8;
                memcpy(buf, geminiBuffer+8, *nbytes_read);
            }

            return TTY_OK;

        #endif
    }

    int tty_read_section(int fd, char *buf, char stop_char, int timeout, int *nbytes_read)
    {
        #ifdef _WIN32
            return TTY_ERRNO;
        #else

            char readBuffer[257]={0};

            if (fd == -1)
                return TTY_ERRNO;

            int bytesRead = 0;
            int err       = TTY_OK;
            *nbytes_read  = 0;

            uint8_t *read_char = 0;

            if (tty_debug)
                IDLog("%s: Request to read until stop char '%#02X' with %d timeout for fd %d\n", __FUNCTION__, stop_char, timeout, fd);

            if (tty_gemini_udp_format)
            {
                bytesRead = read(fd, readBuffer, 255);

                if (bytesRead < 0)
                    return TTY_READ_ERROR;

                int *intSizedBuffer = (int *)readBuffer;
                if (intSizedBuffer[0] != tty_sequence_number)
                {
                    // Not the right reply just do the read again.
                    return tty_read_section(fd, buf, stop_char, timeout, nbytes_read);
                }

                for (int index = 8; index < bytesRead; index++)
                {
                    (*nbytes_read)++;

                    if (*(readBuffer+index) == stop_char)
                    {
                        strncpy(buf, readBuffer+8, *nbytes_read);
                        return TTY_OK;
                    }
                }
            }
            else if (tty_generic_udp_format)
            {
                bytesRead = read(fd, readBuffer, 255);
                if (bytesRead < 0)
                    return TTY_READ_ERROR;
                for (int index = 0; index < bytesRead; index++)
                {
                    (*nbytes_read)++;

                    if (*(readBuffer+index) == stop_char)
                    {
                        strncpy(buf, readBuffer, *nbytes_read);
                        return TTY_OK;
                    }
                }
            }
            else
            {
                for (;;)
                {
                    if ((err = tty_timeout(fd, timeout)))
                        return err;

                    read_char = (uint8_t*)(buf + *nbytes_read);
                    bytesRead = read(fd, read_char, 1);

                    if (bytesRead < 0)
                        return TTY_READ_ERROR;

                    if (tty_debug)
                        IDLog("%s: buffer[%d]=%#X (%c)\n", __FUNCTION__, (*nbytes_read), *read_char, *read_char);

                    if (!(tty_clear_trailing_lf && *read_char == 0X0A && *nbytes_read == 0))
                        (*nbytes_read)++;
                    else {
                        if (tty_debug)
                            IDLog("%s: Cleared LF char left in buf\n", __FUNCTION__);
                    }

                    if (*read_char == stop_char) {
                        return TTY_OK;
                    }
                }
            }

            return TTY_TIME_OUT;

        #endif
    }

    int tty_nread_section(int fd, char *buf, int nsize, char stop_char, int timeout, int *nbytes_read)
    {
        #ifdef _WIN32
            return TTY_ERRNO;
        #else

            if (fd == -1)
                return TTY_ERRNO;

            // For Gemini
            if (tty_gemini_udp_format || tty_generic_udp_format)
                return tty_read_section(fd, buf, stop_char, timeout, nbytes_read);

            int bytesRead = 0;
            int err       = TTY_OK;
            *nbytes_read  = 0;
            uint8_t *read_char = 0;
            memset(buf, 0, nsize);

            if (tty_debug)
                IDLog("%s: Request to read until stop char '%#02X' with %d timeout for fd %d\n", __FUNCTION__, stop_char, timeout, fd);

            for (;;)
            {
                if ((err = tty_timeout(fd, timeout)))
                    return err;

                read_char = (uint8_t*)(buf + *nbytes_read);
                bytesRead = read(fd, read_char, 1);

                if (bytesRead < 0)
                    return TTY_READ_ERROR;

                if (tty_debug)
                    IDLog("%s: buffer[%d]=%#X (%c)\n", __FUNCTION__, (*nbytes_read), *read_char, *read_char);

                if (!(tty_clear_trailing_lf && *read_char == 0X0A && *nbytes_read == 0))
                    (*nbytes_read)++;
                else {
                    if (tty_debug)
                        IDLog("%s: Cleared LF char left in buf\n", __FUNCTION__);
                }

                if (*read_char == stop_char)
                    return TTY_OK;
                else if (*nbytes_read >= nsize)
                    return TTY_OVERFLOW;
            }

        #endif
    }
    void tty_error_msg(int err_code, char *err_msg, int err_msg_len)
    {
        switch (err_code)
        {
            case TTY_OK:
                snprintf(err_msg, err_msg_len, "No Error");
                break;

            case TTY_READ_ERROR:
                snprintf(err_msg, err_msg_len, "Read Error: %s", strerror(errno));
                break;

            case TTY_WRITE_ERROR:
                snprintf(err_msg, err_msg_len, "Write Error: %s", strerror(errno));
                break;

            case TTY_SELECT_ERROR:
                snprintf(err_msg, err_msg_len, "Select Error: %s", strerror(errno));
                break;

            case TTY_TIME_OUT:
                snprintf(err_msg, err_msg_len, "Timeout error");
                break;

            case TTY_PORT_FAILURE:
                if (errno == EACCES)
                    snprintf(err_msg, err_msg_len,
                            "Port failure Error: %s. Try adding your user to the dialout group and restart (sudo adduser "
                            "$USER dialout)",
                            strerror(errno));
                else
                    snprintf(err_msg, err_msg_len, "Port failure Error: %s. Check if device is connected to this port.",
                            strerror(errno));

                break;

            case TTY_PARAM_ERROR:
                snprintf(err_msg, err_msg_len, "Parameter error");
                break;

            case TTY_ERRNO:
                snprintf(err_msg, err_msg_len, "%s", strerror(errno));
                break;

            case TTY_OVERFLOW:
                snprintf(err_msg, err_msg_len, "Read overflow");
                break;

            case TTY_PORT_BUSY:
                snprintf(err_msg, err_msg_len, "Port is busy");
                break;

            default:
                snprintf(err_msg, err_msg_len, "Error: unrecognized error code");
                break;
        }
    }
}