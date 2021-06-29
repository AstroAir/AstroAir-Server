/*
 * check.cpp
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
 
Date:2021-6-23
 
Description:Auto update
 
**************************************************/

#include "AutoUpdate.h"

#include <curl/curl.h>
#include <fstream>
#include <iostream>

namespace AstroAir
{
    struct FtpFile
    {
        const char *filename;
        FILE *stream;
    };

    static size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
    {
        struct FtpFile *out = (struct FtpFile *)stream;
        if (out && !out->stream)
        {
            /* 打开文件以进行写操作 */
            out->stream = fopen(out->filename, "wb");
            if (!out->stream)
                return -1; /* failure, can't open file to write */
        }
        return fwrite(buffer, size, nmemb, out->stream);
    }

    bool AutoCheckVersion()
    {
        std::string temp,temp1;
        CURL *curl;
        CURLcode res;
        struct FtpFile ftpfile = {
            "version.txt.temp", /* 若FTP下载成功，名命下载后的文件为"version.txt.temp" */
            NULL};
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (curl)
        {
            curl_easy_setopt(curl, CURLOPT_URL,
                             "https://astroair.cn/version.txt"); //下载指定的文件
            /* 定义回调函数，以便在需要写入数据时进行调用 */
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
            /*设置一个指向我们的结构的指针传递给回调函数*/
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);
            /* 打开完整的协议/调试输出*/
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            res = curl_easy_perform(curl);
            /* 释放所有curl资源*/
            curl_easy_cleanup(curl);
            if (CURLE_OK != res)
            {
                /*容错处理 */
                fprintf(stderr, "curl told us %d\n", res);
            }
        }
        if (ftpfile.stream)
            fclose(ftpfile.stream); /* 关闭本地文件 */  
        curl_global_cleanup();/*释放所有curl资源*/
        /*读取已有版本*/
        std::ifstream in("version.txt");
        getline(in,temp);
        /*读取最新版本文件*/
        std::ifstream in_temp("version.txt.temp");
        getline(in_temp,temp1);
        /*判断是否为当前版本*/
        if(temp == temp1)
            return true;
        return false;
    }
}