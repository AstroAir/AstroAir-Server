/*
 * wsserver.cpp 
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

Using:Websocketpp<https://github.com/zaphoyd/websocketpp>
      JsonCpp<https://github.com/open-source-parsers/jsoncpp>
 
**************************************************/

#include <fstream>

#include "wsserver.h"
#include "logger.h"
#include "air-asi/asi_ccd.h"

namespace AstroAir
{
    /*
     * name: WSSERVER()
     * describe: Constructor for initializing server parameters
     * 描述：构造函数，用于初始化服务器参数
     */
    WSSERVER::WSSERVER()
    {
        /*加载设置*/
        m_server.set_access_channels(websocketpp::log::alevel::all);
        m_server.clear_access_channels(websocketpp::log::alevel::frame_payload);
        /*初始化服务器*/
        m_server.init_asio();
        /*设置打开事件*/
        m_server.set_open_handler(bind(&WSSERVER::on_open, this , ::_1));
        /*设置关闭事件*/
        m_server.set_close_handler(bind(&WSSERVER::on_close, this , ::_1));
        /*设置事件*/
        m_server.set_message_handler(bind(&WSSERVER::on_message, this ,::_1,::_2));
    }
    
    /*
     * name: ~WSSERVER()
     * describe: Destructor
     * 描述：析构函数
     * calls: stop()
     */
    WSSERVER::~WSSERVER()
    {
        stop();
    }

    /*
     * name: on_open(websocketpp::connection_hdl hdl)
     * @param hdl:WebSocket句柄
     * describe: Clear data on server connection
     * 描述：服务器连接时插入句柄
     */
    void WSSERVER::on_open(websocketpp::connection_hdl hdl)
    {
        IDLog("Successfully established connection with client\n");
        m_connections.insert(hdl);
    }
    
    /*
     * name: on_close(websocketpp::connection_hdl hdl)
     * @param hdl:WebSocket句柄
     * describe: Clear data on server disconnection
     * 描述：服务器断开连接时清空数据
     */
    void WSSERVER::on_close(websocketpp::connection_hdl hdl)
    {
        IDLog("Disconnect from client\n");
        m_connections.erase(hdl);
    }
    
    /*
     * name: on_message(websocketpp::connection_hdl hdl,message_ptr msg)
     * @param hdl:WebSocket句柄
     * @param msg：服务器信息
     * describe: Processing information from clients
     * 描述：处理来自客户端的信息
     * calls: readJson(std::string message)
     */
    void WSSERVER::on_message(websocketpp::connection_hdl hdl,message_ptr msg)
    {
        std::string message = msg->get_payload();
        /*将接收到的信息打印至终端*/
        IDLog("Get information from client: %s\n",message.c_str());
        /*将接收到的信息写入文件*/
        IDLog_CMDL(message.c_str());
        /*处理信息*/
        readJson(message);
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
    
    /*
     * name: readJson(std::string message)
     * @param message:客户端信息
     * describe: Process information and complete
     * 描述：处理信息并完成对应任务
     * note: This is the heart of the whole process!!!
     */
    void WSSERVER::readJson(std::string message)
    {
        /*运用JsonCpp拆分JSON数组*/
        std::unique_ptr<Json::CharReader>const json_read(reader.newCharReader());
        json_read->parse(message.c_str(), message.c_str() + message.length(), &root,&errs);
        /*将string格式转化为const char*/
        method = root["method"].asString();
        const char* road = method.c_str();
        /*判断客户端需要执行的命令*/
        switch(hash_(road))
        {
            /*返回服务器版本号*/
            case "RemoteSetDashboardMode"_hash:
                SetDashBoardMode();
                break;
            /*返回当前目录下的文件*/
            case "RemoteGetAstroAirProfiles"_hash:
                GetAstroAirProfiles();
                break;
            /*连接设备*/
            case "RemoteSetupConnect"_hash:{
                std::thread t1(&WSSERVER::SetupConnect,this,root["method"]["params"]["TimeoutConnect"].asInt());
                t1.join();
                break;
            }
            /*轮询，保持连接*/
            case "Polling"_hash:
                Polling();
                break;
            /*默认返回未知信息*/
            default:
                UnknownMsg();
        }
    }
    
    /*
     * name: send(std::string payload)
     * @param message:需要发送的信息
     * describe: Send information to client
     * 描述：向客户端发送信息
     * note: The message must be sent in the format of JSON
     */
    void WSSERVER::send(std::string message)
    {
        for (auto it : m_connections)
        {
            try
            {
                m_server.send(it, message, websocketpp::frame::opcode::text);
            }
            catch (websocketpp::exception const &e)
            {
                IDLog("Unable to send message, please check connection\nThe reason is");
                IDLog_DEBUG("%s\n",e.what());
            }
            catch (...)
            {
                std::cerr << "other exception" << std::endl;
            }
        }
    }
    
    /*
     * name: stop()
     * describe: Stop the websocket server
     * 描述：停止WebSocket服务器
     */
    void WSSERVER::stop()
    {
        for (auto it : m_connections)
            m_server.close(it, websocketpp::close::status::normal, "Switched off by user.");
        IDLog("Stop the server...\n");
        m_connections.clear();
        m_server.stop();
        IDLog("Good bye\n");
    }

    /*
     * name: is_running()
     * @return Boolean function:服务器运行状态
     *  -false: server is not running||服务器不在运行
     *  -true: server is running||服务器正在运行
     */
    bool WSSERVER:: is_running()
    {
        return m_server.is_listening();
    }
    
    /*
     * name: run(int port)
     * @param port:服务器端口
     * describe: This is used to start the websocket server
     * 描述：启动WebSocket服务器
     */
    void WSSERVER::run(int port)
    {
        try
        {
            IDLog("Start the server...\n");
            m_server.listen(port);
            m_server.start_accept();
            m_server.run();
        }
        catch (websocketpp::exception const &e)
        {   
            IDLog("Unable to start the server\nThe reason is:");
            IDLog_DEBUG("%s\n",e.what());
            //std::cerr << e.what() << std::endl;
            
        }
        catch (...)
        {
            std::cerr << "other exception" << std::endl;
        }
    }
    
    /*
     * name: SetDashBoardMode()
     * describe: This is used to initialize the connection and send the version number.
     * 描述：初始化连接，并发送版本号
     * calls: send(std::string message)
     * calls: IDLog(const char *fmt, ...)
     */
    void WSSERVER::SetDashBoardMode()
	{
        Json::Value Root;
		Root["result"] = Json::Value(1);
		Root["code"] = Json::Value();
		Root["Event"] = Json::Value("Version");
		Root["AIRVersion"] = Json::Value("2.0.0");
		json_messenge = Root.toStyledString();
		send(json_messenge);
	}

    /*
     * name: GetAstroAirProfiles()
     * describe: Get the local file name and upload it to the client
     * 描述：获取本地文件名称并上传至客户端
     * describe: Gets the specified suffix file name in the folder
	 * 描述：获取文件夹中指定后缀文件名称
	 * note:The suffix of get file should be .air or .json
     */
    void WSSERVER::GetAstroAirProfiles()
    {
        /*寻找当前文件夹下的所有文件*/
        struct dirent *ptr;      
		DIR *dir;  
		std::string PATH = "./";  
		dir=opendir(PATH.c_str());   
		std::vector<std::string> files;  
		while((ptr=readdir(dir))!=NULL)  
		{  
			if(ptr->d_name[0] == '.')  
				continue;  
			files.push_back(ptr->d_name);  
		}  
		for (int i = 0; i < files.size(); ++i)  
		{  
			std::cout << files[i] << std::endl;  
		}  
		closedir(dir);
        /*整合信息并发送至客户端*/
        Json::Value Root,list,name;
        Root["result"] = Json::Value(1);
		Root["code"] = Json::Value();
		Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteGetAstroAirProfiles");
        name["name"] = Json::Value("config.air");
        Root["list"] = Json::Value("config2.air");
        list["list"] = name;
        Root["ParamRet"] = list;
        Root["ActionResultInt"] = Json::Value(4);
        json_messenge = Root.toStyledString();
        send(json_messenge);
    }
    
    /*
     * name: SetupConnect(int timeout)
     * @param timeout:连接相机最长时间
     * describe: All connection profiles in the device
     * 描述：连接配置文件中的所有设备
     * note: If it times out, an error message is returned
     */
    void WSSERVER::SetupConnect(int timeout)
    {
        /*读取config.air配置文件，并且存入参数中*/
        std::string line,jsonStr;
        std::ifstream in("config.air", std::ios::binary);
        /*打开文件*/
        if (!in.is_open())
        {
            IDLog("Unable to open configuration file\n");
            IDLog_DEBUG("Unable to open configuration file\n");
            return;
        }
        /*将文件转化为string格式*/
        while (getline(in, line))
        {
            jsonStr.append(line);
        }
        /*关闭文件*/
        in.close();
        /*将读取出的json数组转化为string*/
        std::unique_ptr<Json::CharReader>const json_read(reader.newCharReader());
        json_read->parse(jsonStr.c_str(), jsonStr.c_str() + jsonStr.length(), &root,&errs);
        camera = root["camera"]["brand"].asString();
        camera_name = root["camera"]["name"].asString();
        if(!camera.empty() && !camera_name.empty())
        {
            const char* a = camera.c_str();
            switch(hash_(a))
            {
                case "ZWOASI"_hash:{
                    ASICCD CCD,*ccd = &CCD;
                    ccd->Connect(camera_name);
                    break;
                }
                /*
                case "QHYCCD"_hash:{
                    QHYCCD CCD,*ccd = &CCD;
                    ccd->Connect(camera_name);
                    break;
                }
                */
                default:
                    UnknownCamera();
            }
        }
        mount = root["mount"]["brand"].asString();
        mount_name = root["mount"]["name"].asString();
        if(!mount.empty() && !mount_name.empty())
        {
            const char* a = mount.c_str();
            switch(hash_(a))
            {
                /*
                case "iOptron"_hash:{
                    iOptron Mount,*mount = &Mount;
                    mount->Connect(mount_name);
                    break;
                }
                case "SkyWatcher"_hash:{
                    SkyWatcher Mount,*mount = &Mount;
                    mount->Connect(mount_name);
                    break;
                }
                */
                default:
                    UnknownMount();
            }
        }
    }

    /*
     * name: UnknownMsg()
     * describe: Processing unknown information from clients
     * 描述：处理来自客户端的未知信息
	 * note:If this function is executed, an error will appear on the web page
     */
    void WSSERVER::UnknownMsg()
    {
        IDLog("An unknown message was received from the client.\n");
        IDLog_DEBUG("An unknown message was received from the client.\n");
        Json::Value Root,error;
        Root["result"] = Json::Value(1);
		Root["code"] = Json::Value();
        Root["id"] = Json::Value(403);
        error["message"] = Json::Value("Unknown information");
        Root["error"] = error;
        json_messenge = Root.toStyledString();
        send(json_messenge);
    }
    
    /*
     * name: UnknownCamera()
     * describe: Processing connection camera error
     * 描述：如果发现未知相机，则执行此函数
	 * note: This function is executed if an unknown camera is found
     */
    void WSSERVER::UnknownCamera()
    {
        IDLog("An unknown camera was chosen.\n");
        IDLog_DEBUG("An unknown camera was chosen.\n");
        Json::Value Root,error;
        Root["result"] = Json::Value(1);
		Root["code"] = Json::Value();
        Root["id"] = Json::Value(301);
        error["message"] = Json::Value("Unknown camera");
        Root["error"] = error;
        json_messenge = Root.toStyledString();
        send(json_messenge);
    }
    
    /*
     * name: UnknownMount()
     * describe: Processing connection mount error
     * 描述：如果发现未知赤道仪，则执行此函数
	 * note: This function is executed if an unknown mount is found
     */
    void WSSERVER::UnknownMount()
    {
        IDLog("An unknown mount was chosen.\n");
        IDLog_DEBUG("An unknown mount was chosen.\n");
        Json::Value Root,error;
        Root["result"] = Json::Value(1);
		Root["code"] = Json::Value();
        Root["id"] = Json::Value(302);
        error["message"] = Json::Value("Unknown mount");
        Root["error"] = error;
        json_messenge = Root.toStyledString();
        send(json_messenge);
    }
    
    /*
     * name: Polling()
     * describe: The client remains connected to the server
     * 描述：客户端与服务器保持连接
	 * note:This function is most commonly used
     */
    void WSSERVER::Polling()
    {
        Json::Value Root;
        Root["result"] = Json::Value(1);
		Root["code"] = Json::Value();
        Root["Event"] = Json::Value("Polling");
        json_messenge = Root.toStyledString();
        send(json_messenge);
    }
        
}
