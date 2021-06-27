/*
 * air_com.cpp
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

#include "air_com.h"
#include "../logger.h"

#include <sys/ioctl.h>
#include <string.h>

#define PARITY_NONE 0
#define PARITY_EVEN 1
#define PARITY_ODD  2

#define MAXRBUF 2048

static int tty_debug = 0;
static int tty_gemini_udp_format = 0;
static int tty_generic_udp_format = 0;
static int tty_sequence_number = 1;
static int tty_clear_trailing_lf = 0;

namespace AstroAir
{
    /*
     * name: AIRCOM()
     * describe: Constructor for initializing com parameters
     * 描述：构造函数，用于初始化串口参数
     */
    AIRCOM::AIRCOM()
    {
        //m_PortFD = -1;
    }

    /*
     * name: ~AIRCOM()
     * describe: Destructor
     * 描述：析构函数
     * calls: Close()
     */
    AIRCOM::~AIRCOM()
    {
        //if (m_PortFD != -1)
        //    Close(m_PortFD);
    }

    /*
     * name: Open(const char * dev)
     * @param dev:serial port
     * describe: Open serial port
     * 描述：打开串口
     * calls: open()
     * calls: IDLog()
     * calls: ioctl()
     */
    bool AIRCOM::Open(const char *device, int bit_rate, int word_size, int parity, int stop_bits, int *fd)
    {
        int t_fd = -1;
        int i = 0;
        char msg[128]={0};
        int bps;
        struct termios tty_setting;
        int bt = strstr(device, "rfcomm") || strstr(device, "Bluetooth");
        for (i = 0 ; i < 3 ; i++)
        {
            t_fd = open(device, O_RDWR | O_NOCTTY | (bt ? 0 : O_CLOEXEC));      //使用蓝牙时不要用O_CLOEXEC
            if (t_fd > 0)
                break;
            else
            {
                *fd = -1;
                if (errno == EBUSY)
                {
                    usleep(1e6);
                    continue;
                }
                else
                    return TTY_PORT_FAILURE;
            }
        }
        if (t_fd == -1)
            return TTY_PORT_BUSY;
        if (bt == 0 && ioctl(t_fd, TIOCEXCL) == -1)     //蓝牙支持
        {
            IDLog_Error(_("tty_connect: Error setting TIOCEXC."));
            close(t_fd);
            return TTY_PORT_FAILURE;
        }
        if (tcgetattr(t_fd, &tty_setting) == -1)
        {
            IDLog_Error(_("tty_connect: failed getting tty attributes."));
            close(t_fd);
            return TTY_PORT_FAILURE;
        }
        switch (bit_rate)
        {
            case 0:      bps = B0;      break;
            case 50:     bps = B50;     break;
            case 75:     bps = B75;     break;
            case 110:    bps = B110;    break;
            case 134:    bps = B134;    break;
            case 150:    bps = B150;    break;
            case 200:    bps = B200;    break;
            case 300:    bps = B300;    break;
            case 600:    bps = B600;    break;
            case 1200:   bps = B1200;   break;
            case 1800:   bps = B1800;   break;
            case 2400:   bps = B2400;   break;
            case 4800:   bps = B4800;   break;
            case 9600:   bps = B9600;   break;
            case 19200:  bps = B19200;  break;
            case 38400:  bps = B38400;  break;
            case 57600:  bps = B57600;  break;
            case 115200: bps = B115200; break;
            case 230400: bps = B230400; break;
            case 460800: bps = B460800; break;
            case 576000: bps = B576000; break;
            case 921600: bps = B921600; break;
            default:
                if (snprintf(msg, sizeof(msg), _("tty_connect: %d is not a valid bit rate."), bit_rate) < 0)
                    perror(NULL);
                else
                    IDLog_Error(msg);
                close(t_fd);
                return TTY_PARAM_ERROR;
        }
        if ((cfsetispeed(&tty_setting, bps) < 0) || (cfsetospeed(&tty_setting, bps) < 0))
        {
            IDLog_Error(_("tty_connect: failed setting bit rate."));
            close(t_fd);
            return TTY_PORT_FAILURE;
        }
        tty_setting.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD | HUPCL | CRTSCTS);       //控制模式
        tty_setting.c_cflag |= (CLOCAL | CREAD);
        switch (word_size)      //数据位
        {
            case 5: 
                tty_setting.c_cflag |= CS5; 
                break;
            case 6: 
                tty_setting.c_cflag |= CS6; 
                break;
            case 7: 
                tty_setting.c_cflag |= CS7; 
                break;
            case 8: 
                tty_setting.c_cflag |= CS8; 
                break;
            default:
                fprintf(stderr, "Default\n");
                if (snprintf(msg, sizeof(msg), "tty_connect: %d is not a valid data bit count.", word_size) < 0)
                    perror(NULL);
                else
                    perror(msg);
                close(t_fd);
                return TTY_PARAM_ERROR;
        }
        switch (parity)     //奇偶校验
        {
            case PARITY_NONE:
                break;
            case PARITY_EVEN:
                tty_setting.c_cflag |= PARENB;
                break;
            case PARITY_ODD:
                tty_setting.c_cflag |= PARENB | PARODD;
                break;
            default:
                fprintf(stderr, "Default1\n");
                if (snprintf(msg, sizeof(msg), "tty_connect: %d is not a valid parity selection value.", parity) < 0)
                    perror(NULL);
                else
                    perror(msg);
                close(t_fd);
                return TTY_PARAM_ERROR;
        }
        switch (stop_bits)      //停止位
        {
            case 1:
                break;
            case 2:
                tty_setting.c_cflag |= CSTOPB;
                break;
            default:
                fprintf(stderr, "Default2\n");
                if (snprintf(msg, sizeof(msg), _("tty_connect: %d is not a valid stop bit count."), stop_bits) < 0)
                    perror(NULL);
                else
                    IDLog_Error(msg);
                close(t_fd);
                return TTY_PARAM_ERROR;
        }
        tty_setting.c_iflag &= ~(PARMRK | ISTRIP | IGNCR | ICRNL | INLCR | IXOFF | IXON | IXANY);
        tty_setting.c_iflag |= INPCK | IGNPAR | IGNBRK;
        tty_setting.c_oflag &= ~(OPOST | ONLCR);
        tty_setting.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | IEXTEN | NOFLSH | TOSTOP);
        tty_setting.c_lflag |= NOFLSH;
        tty_setting.c_cc[VMIN]  = 1;
        tty_setting.c_cc[VTIME] = 0;
        tcflush(t_fd, TCIOFLUSH);
        if (tcsetattr(t_fd, TCSANOW, &tty_setting))
        {
            IDLog_Error(_("tty_connect: failed setting attributes on serial port."));
            Close(t_fd);
            return TTY_PORT_FAILURE;
        }
        *fd = t_fd;
        return TTY_OK;
    }

    /*
     * name: Close()
     * describe: Close serial port
     * 描述：关闭串口
     * calls: close()
     */
    int AIRCOM::Close(int fd)
    {
        if (fd == -1)
            return TTY_ERRNO;
        int err;
        tcflush(fd, TCIOFLUSH);
        err = close(fd);
        if (err != 0)
            return TTY_ERRNO;
        return TTY_OK;
    }

    /*
     * name: Read(int fd, char *buf, int nbytes, int timeout, int *nbytes_read)
     * @param fd: file descriptor
     * @param buf: pointer to store data. Must be initilized and big enough to hold data.
     * @param nbytes: number of bytes to read.
     * @param timeout: number of seconds to wait for terminal before a timeout error is issued.
     * @param nbytes_read: the number of bytes read.
     * describe: Read from serial port
     * 描述：从串口读取信息
     * calls: COM_Timeout()
     * return: On success, it returns TTY_OK, otherwise, a TTY_ERROR code.
     */
    int AIRCOM::Read(int fd, char *buf, int nbytes, int timeout, int *nbytes_read)
    {
        if (fd == -1)
            return TTY_ERRNO;
        int numBytesToRead = nbytes;
        int bytesRead = 0;
        int err = 0;
        *nbytes_read = 0;
        if (nbytes <= 0)
            return TTY_PARAM_ERROR;
        if (tty_debug)
            IDLog(_("%s: Request to read %d bytes with %d timeout for fd %d\n"), __FUNCTION__, nbytes, timeout, fd);
        char geminiBuffer[257] = {0};
        char *buffer = buf;
        if (tty_gemini_udp_format)
        {
            numBytesToRead = nbytes + 8;
            buffer = geminiBuffer;
        }
        while (numBytesToRead > 0)
        {
            if ((err = COM_Timeout(fd, timeout)))
                return err;
            bytesRead = read(fd, buffer + (*nbytes_read), ((uint32_t)numBytesToRead));
            if (bytesRead < 0)
                return TTY_READ_ERROR;
            if (tty_debug)
            {
                IDLog(_("%d bytes read and %d bytes remaining...\n"), bytesRead, numBytesToRead - bytesRead);
                int i = 0;
                for (i = *nbytes_read; i < (*nbytes_read + bytesRead); i++)
                    IDLog(_("%s: buffer[%d]=%#X (%c)\n"), __FUNCTION__, i, (unsigned char)buf[i], buf[i]);
            }
            if (*nbytes_read == 0 && tty_clear_trailing_lf && *buffer == 0x0A)
            {
                if (tty_debug)
                    IDLog(_("%s: Cleared LF char left in buf\n"), __FUNCTION__);

                memcpy(buffer, buffer + 1, bytesRead);
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
                return Read(fd, buf, nbytes, timeout, nbytes_read);     //重新执行并返回结果
            }
            *nbytes_read -= 8;
            memcpy(buf, geminiBuffer + 8, *nbytes_read);
        }
        return TTY_OK;
    }

    /*
     * name: ReadSection(int fd, char *buf, char stop_char, int timeout, int *nbytes_read)
     * @param fd: file descriptor
     * @param buf: pointer to store data. Must be initilized and big enough to hold data.
     * @param stop_char: if the function encounters \e stop_char then it stops reading and returns the buffer.
     * @param timeout: number of seconds to wait for terminal before a timeout error is issued.
     * @param nbytes_read: the number of bytes read.
     * describe: Read from serial port
     * 描述：从串口读取信息
     * calls: COM_Timeout()
     * calls: IDLog()
     * return: On success, it returns TTY_OK, otherwise, a TTY_ERROR code.
     */
    int AIRCOM::ReadSection(int fd, char *buf, char stop_char, int timeout, int *nbytes_read)
    {
        char readBuffer[257] = {0};
        if (fd == -1)
            return TTY_ERRNO;
        int bytesRead = 0;
        int err = TTY_OK;
        *nbytes_read = 0;
        uint8_t *read_char = 0;
        if (tty_debug)
            IDLog(_("%s: Request to read until stop char '%#02X' with %d timeout for fd %d\n"), __FUNCTION__, stop_char, timeout, fd);
        if (tty_gemini_udp_format)
        {
            bytesRead = read(fd, readBuffer, 255);
            if (bytesRead < 0)
                return TTY_READ_ERROR;
            int *intSizedBuffer = (int *)readBuffer;
            if (intSizedBuffer[0] != tty_sequence_number)
            {
                return ReadSection(fd, buf, stop_char, timeout, nbytes_read);     //重新执行并返回结果
            }
            for (int index = 8; index < bytesRead; index++)
            {
                (*nbytes_read)++;
                if (*(readBuffer + index) == stop_char)
                {
                    strncpy(buf, readBuffer + 8, *nbytes_read);
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
                if (*(readBuffer + index) == stop_char)
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
                if ((err = COM_Timeout(fd, timeout)))
                    return err;
                read_char = (uint8_t *)(buf + *nbytes_read);
                bytesRead = read(fd, read_char, 1);

                if (bytesRead < 0)
                    return TTY_READ_ERROR;

                if (tty_debug)
                    IDLog("%s: buffer[%d]=%#X (%c)\n", __FUNCTION__, (*nbytes_read), *read_char, *read_char);

                if (!(tty_clear_trailing_lf && *read_char == 0X0A && *nbytes_read == 0))
                    (*nbytes_read)++;
                else
                {
                    if (tty_debug)
                        IDLog("%s: Cleared LF char left in buf\n", __FUNCTION__);
                }

                if (*read_char == stop_char)
                {
                    return TTY_OK;
                }
            }
        }
        return TTY_TIME_OUT;
    }

    /*
     * name: ReadNSection(int fd, char *buf, int nsize, char stop_char, int timeout, int *nbytes_read)
     * @param fd: file descriptor
     * @param buf: pointer to store data. Must be initilized and big enough to hold data.
     * @param nsize: size of buf. If stop character is not encountered before nsize, the function aborts.
     * @param stop_char: if the function encounters \e stop_char then it stops reading and returns the buffer.
     * @param timeout: number of seconds to wait for terminal before a timeout error is issued.
     * @param nbytes_read: the number of bytes read.
     * describe: Read from serial port
     * 描述：从串口读取信息
     * calls: COM_Timeout()
     * calls: IDLog()
     * return: On success, it returns TTY_OK, otherwise, a TTY_ERROR code.
     */
    int AIRCOM::ReadNSection(int fd, char *buf, int nsize, char stop_char, int timeout, int *nbytes_read)
    {
        if (fd == -1)
            return TTY_ERRNO;
        if (tty_gemini_udp_format || tty_generic_udp_format)
            return ReadSection(fd, buf, stop_char, timeout, nbytes_read);
        int bytesRead = 0;
        int err       = TTY_OK;
        *nbytes_read  = 0;
        uint8_t *read_char = 0;
        memset(buf, 0, nsize);
        if (tty_debug)
            IDLog(_("%s: Request to read until stop char '%#02X' with %d timeout for fd %d\n"), __FUNCTION__, stop_char, timeout, fd);
        for (;;)
        {
            if ((err = COM_Timeout(fd, timeout)))
                return err;
            read_char = (uint8_t*)(buf + *nbytes_read);
            bytesRead = read(fd, read_char, 1);
            if (bytesRead < 0)
                return TTY_READ_ERROR;
            if (tty_debug)
                IDLog(_("%s: buffer[%d]=%#X (%c)\n"), __FUNCTION__, (*nbytes_read), *read_char, *read_char);
            if (!(tty_clear_trailing_lf && *read_char == 0X0A && *nbytes_read == 0))
                (*nbytes_read)++;
            else {
                if (tty_debug)
                    IDLog(_("%s: Cleared LF char left in buf\n"), __FUNCTION__);
            }
            if (*read_char == stop_char)
                return TTY_OK;
            else if (*nbytes_read >= nsize)
                return TTY_OVERFLOW;
        }
    }

    /*
     * name: Write(int fd, const char *buf, int nbytes, int *nbytes_written)
     * @param fd: file descriptor
     * @param buf: a null-terminated buffer to write to fd.
     * @param nbytes: number of bytes to write from \e buffer
     * @param nbytes_written: the number of bytes written
     * describe: Wirte to serial port
     * 描述：向串口写入信息
     * calls: write()
     * calls: IDLog()
     * return: On success, it returns TTY_OK, otherwise, a TTY_ERROR code.
     */
    int AIRCOM::Write(int fd, const char *buf, int nbytes, int *nbytes_written)
    {
        int geminiBuffer[66] = {0};
        char *buffer = (char *)buf;
        if (tty_gemini_udp_format)
        {
            buffer = (char *)geminiBuffer;
            geminiBuffer[0] = ++tty_sequence_number;
            geminiBuffer[1] = 0;
            memcpy((char *)&geminiBuffer[2], buf, nbytes);
            nbytes += 9;        //为头部增加8字节，以及一字节的空请求
        }
        if (fd == -1)
            return TTY_ERRNO;
        int bytes_w = 0;
        *nbytes_written = 0;
        if (tty_debug)
        {
            int i = 0;
            for (i = 0; i < nbytes; i++)
                IDLog(_("%s: buffer[%d]=%#X (%c)\n"), __FUNCTION__, i, (unsigned char)buf[i], buf[i]);
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
    }

    /*
     * name: Write(int fd, const char *buf, int nbytes, int *nbytes_written)
     * @param fd: file descriptor
     * @param buf: a null-terminated buffer to write to fd.
     * @param nbytes: number of bytes to write from \e buffer
     * @param nbytes_written: the number of bytes written
     * describe: Wirte string to serial port
     * 描述：将字符串写入串口
     * calls: write()
     * return: On success, it returns TTY_OK, otherwise, a TTY_ERROR code.
     */
    int AIRCOM::WriteString(int fd, const char *buf, int *nbytes_written)
    {
        unsigned int nbytes;
        nbytes = strlen(buf);
        return Write(fd, buf, nbytes, nbytes_written);
    }

    /*
     * name: COM_Timeout(int fd, int timeout)
     * @param fd: file descriptor
     * @param timeout: timeout
     * describe: timeout
     * 描述：串口超时
     * calls: write()
     * return: On success, it returns TTY_OK, otherwise, a TTY_ERROR code.
     */
    int AIRCOM::COM_Timeout(int fd, int timeout)
    {
        if (fd == -1)
            return TTY_ERRNO;
        struct timeval tv;
        fd_set readout;
        int retval;
        FD_ZERO(&readout);
        FD_SET(fd, &readout);
        tv.tv_sec = timeout;        //等待
        tv.tv_usec = 0;
        retval = select(fd + 1, &readout, NULL, NULL, &tv);     //等待知道改变fd状态
        if (retval > 0)     //当修改成功是返回0
            return TTY_OK;
        else if (retval == -1)      //当发生错误时返回-1
            return TTY_SELECT_ERROR;
        else        //如果在一定时间内没有任何改动，就返回-2
            return TTY_TIME_OUT;
    }

    /*
     * name: COM_ERRORMSG(int err_code, char *err_msg, int err_msg_len)
     * @param err_code: the error code return by any TTY function.
     * @param err_msg: an initialized buffer to hold the error message.
     * @param err_msg_len: length in bytes of \e err_msg
     * describe: serial port's error message
     * 描述：串口错误信息
     */
    void AIRCOM::COM_ERRORMSG(int err_code, char *err_msg, int err_msg_len)
    {
        switch (err_code)
        {
            case TTY_OK:        //无错误
                snprintf(err_msg, err_msg_len, _("No Error"));
                break;
            case TTY_READ_ERROR:        //串口读取错误
                snprintf(err_msg, err_msg_len, _("Read Error: %s"), strerror(errno));
                break;
            case TTY_WRITE_ERROR:       //串口写入错误
                snprintf(err_msg, err_msg_len, _("Write Error: %s"), strerror(errno));
                break;
            case TTY_SELECT_ERROR:      //串口选择错误
                snprintf(err_msg, err_msg_len, _("Select Error: %s"), strerror(errno));
                break;
            case TTY_TIME_OUT:          //串口通信超时
                snprintf(err_msg, err_msg_len, _("Timeout error"));
                break;
            case TTY_PORT_FAILURE:      //串口端口错误
                if (errno == EACCES)
                    snprintf(err_msg, err_msg_len,
                            _("Port failure Error: %s. Try adding your user to the dialout group and restart (sudo adduser "
                            "$USER dialout)"),
                            strerror(errno));
                else
                    snprintf(err_msg, err_msg_len, _("Port failure Error: %s. Check if device is connected to this port."),
                            strerror(errno));
                break;
            case TTY_PARAM_ERROR:       //串口参数错误
                snprintf(err_msg, err_msg_len, _("Parameter error"));
                break;
            case TTY_ERRNO:             //串口的奇怪错误增加了
                snprintf(err_msg, err_msg_len, "%s", strerror(errno));
                break;
            case TTY_OVERFLOW:          //串口重置
                snprintf(err_msg, err_msg_len, _("Read overflow"));
                break;
            case TTY_PORT_BUSY:         //串口繁忙
                snprintf(err_msg, err_msg_len, _("Port is busy"));
                break;
            default:                    //未知串口错误
                snprintf(err_msg, err_msg_len, _("Error: unrecognized error code"));
                break;
        }
    }

    
}
