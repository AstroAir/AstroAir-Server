/*
 * wsserver.cpp
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
 
Description:Main framework of astroair server

Using:Websocketpp<https://github.com/zaphoyd/websocketpp>
      
Using:JsonCpp<https://github.com/open-source-parsers/jsoncpp>

**************************************************/

#include "wsserver.h"
#include "logger.h"
#include "opencv.h"
//#include "base64.h"

#include "air_search.h"
#include "air_camera.h"
#include "air_mount.h"
#include "air_solver.h"
#include "air_script.h"

//#include "ccfits.h"

#include "air-asi/asi_ccd.h"
#include "air-qhy/qhy_ccd.h"
#include "air-gphoto2/gphoto2_ccd.h"
#include "telescope/ieqpro.h"

namespace AstroAir
{
    WSSERVER ws;
    std::string img_data,SequenceTarget;
    std::atomic_bool isCameraConnected;
    std::atomic_bool isMountConnected;
	std::atomic_bool isFocusConnected;
	std::atomic_bool isFilterConnected;
	std::atomic_bool isGuideConnected;

    /*服务器配置参数*/
	int MaxUsedTime = 0;		//解析最长时间
	int MaxThreadNumber = 0;		//最多能同时处理的事件数量
	int	MaxClientNumber = 0;		//最大客户端数量
    int thread_num = 0;
    std::string TargetRA,TargetDEC,MountAngle;

    std::string Camera,Mount,Focus,Filter,Guide;
	std::string Camera_name,Mount_name,Focus_name,Filter_name,Guide_name;

//----------------------------------------服务器----------------------------------------

#ifdef HAS_WEBSOCKET
    /*
     * name: WSSERVER()
     * describe: Constructor for initializing server parameters
     * 描述：构造函数，用于初始化服务器参数
     */
    WSSERVER::WSSERVER()
    {
        LoadConfigure();
        /*初始化WebSocket服务器*/
        /*加载设置*/
        m_server.clear_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);
        /*初始化服务器*/
        m_server.init_asio();
        /*设置重新使用端口*/
        m_server.set_reuse_addr(true);
        /*设置打开事件*/
        m_server.set_open_handler(bind(&WSSERVER::on_open, this , ::_1));
        /*设置关闭事件*/
        m_server.set_close_handler(bind(&WSSERVER::on_close, this , ::_1));
        /*设置事件*/
        m_server.set_message_handler(bind(&WSSERVER::on_message, this ,::_1,::_2));
        /*重置参数*/
        isConnected = false;            //客户端连接状态
        isCameraConnected = false;      //相机连接状态
        isMountConnected = false;       //赤道仪连接状态
        isFocusConnected = false;       //电动调焦座连接状态
        isFilterConnected = false;      //滤镜轮连接状态
        isGuideConnected = false;       //导星软件连接状态
    }
    
    /*
     * name: ~WSSERVER()
     * describe: Destructor
     * 描述：析构函数
     * calls: stop()
     */
    WSSERVER::~WSSERVER()
    {
		/*如果服务器正在工作，则在停止程序之前停止服务器*/
        if(isConnected ==true)
        {
            stop();
        }
        delete [] CCD;
        delete [] MOUNT;
        delete [] FOCUS;
        delete [] FILTER;
        delete [] GUIDE;
    }

    /*
     * name: on_open(websocketpp::connection_hdl hdl)
     * @param hdl:WebSocket句柄
     * describe: Insert handle when server connects
     * 描述：服务器连接时插入句柄
     */
    void WSSERVER::on_open(websocketpp::connection_hdl hdl)
    {
        lock_guard<mutex> guard(mtx);
        airserver::connection_ptr con = m_server.get_con_from_hdl( hdl );      // 根据连接句柄获得连接对象
        std::string path = con->get_resource();
        IDLog(_("Successfully established connection with client path %s\n"),path.c_str());
        m_connections.insert(hdl);
        isConnected = true;
        ClientNum++;
        if(ClientNum > MaxClientNumber)
            ClientNumError();
        m_server_cond.notify_one();
    }
    
    /*
     * name: on_close(websocketpp::connection_hdl hdl)
     * @param hdl:WebSocket句柄
     * describe: Clear data on server disconnection
     * 描述：服务器断开连接时清空数据
     */
    void WSSERVER::on_close(websocketpp::connection_hdl hdl)
    {
        lock_guard<mutex> guard(mtx);
        IDLog(_("Disconnect from client\n"));
        m_connections.erase(hdl);
        isConnected = false;
        /*防止多次连接导致错误*/
        memset(DeviceBuf,0,5);
        DeviceNum = 0;
        ClientNum--;
        m_server_cond.notify_one();
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
        /*将接收到的信息写入文件*/
        #ifdef DEBUG_MODE
            if(method != "Polling")
                IDLog_CMDL(message.c_str());
        #endif
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
            /*设置新的配置文件*/
            case "RemoteSetProfile"_hash:{
                std::thread ProfileThread(&WSSERVER::SetProfile,this,root["params"]["FileName"].asString());
                ProfileThread.detach();
                thread_num++;
                break;
            }
            /*连接设备*/
            case "RemoteSetupConnect"_hash:{
                std::thread ConnectThread(&WSSERVER::SetupConnect,this,root["params"]["TimeoutConnect"].asInt());
                ConnectThread.detach();
                thread_num++;
                break;
            }
            /*断开连接*/
            case "RemoteSetupDisconnect"_hash:{
                std::thread DisconnectThread(&WSSERVER::SetupDisconnect,this,root["params"]["TimeoutConnect"].asInt());
                DisconnectThread.detach();
                thread_num++;
                break;
            }
            /*相机开始拍摄*/
            case "RemoteCameraShot"_hash:{
                std::thread CamThread(&AIRCAMERA::StartExposureServer,CCD,root["params"]["Expo"].asInt(),root["params"]["Bin"].asInt(),root["params"]["IsSaveFile"].asBool(),root["params"]["FitFileName"].asString(),root["params"]["Gain"].asInt(),root["params"]["Offset"].asInt());
                CamThread.detach();
                thread_num++;
                break;
            }
            /*相机停止拍摄*/
            case "RemoteActionAbort"_hash:
				CCD->AbortExposure();
				break;
            /*相机制冷*/
            case "RemoteCooling"_hash:{
                std::thread CoolingThread(&AIRCAMERA::Cooling,CCD,root["params"]["IsSetPoint"].asBool(),root["params"]["IsCoolDown"].asBool(),root["params"]["IsASync"].asBool(),root["params"]["IsWarmup"].asBool(),root["params"]["IsCoolerOFF"].asBool(),root["params"]["Temperature"].asInt());
                CoolingThread.detach();
                thread_num++;
                break;
            }
            /*搜索天体*/
            case "RemoteSearchTarget"_hash:{
                Search a;
                std::thread SearchThread(&Search::SearchTarget,a,root["params"]["Name"].asString());
                SearchThread.detach();
                thread_num++;
                break;
            }
            /*获取滤镜轮设置*/
            case "RemoteGetFilterConfiguration"_hash:{
                GetFilterConfiguration();
                break;
            }
            /*获取已连接设备信息*/
            case "RemoteGetEnvironmentData"_hash:{
                EnvironmentDataSend();
                break;
            }
            /*赤道仪Goto*/
            case "RemotePrecisePointTarget"_hash:{
                std::thread GotoThread(&AIRMOUNT::GotoServer,MOUNT,root["params"]["RAText"].asString(),root["params"]["DECText"].asString());
                GotoThread.detach();
                thread_num++;
                break;
            }
            /*解析*/
            case "RemoteSolveActualPosition"_hash:{
                if(root["params"]["IsBlind"].asBool() == true)
                {
                    std::thread SolveThread(&AIRSOLVER::SolveActualPosition,SOLVER,root["params"]["IsBlind"].asBool(),root["params"]["IsSync"].asBool());
                    SolveThread.detach();
                }  
                else
                {
                    std::thread SolveThread(&AIRSOLVER::SolveActualPositionOnline,SOLVER,root["params"]["IsBlind"].asBool(),root["params"]["IsSync"].asBool());
                    SolveThread.detach();
                }
                thread_num++;
                break;
            }
            /*搜索所有可以执行的序列*/
            case "RemoteGetListAvalaibleSequence"_hash:{
                std::thread SequenceThread(&AIRSCRIPT::GetListAvalaibleSequence,SCRIPT);
                SequenceThread.detach();
                thread_num++;
                break;
            }
            /*运行拍摄序列*/
            case "RemoteSequence"_hash:{
                std::thread SequenceThreadRun(&AIRSCRIPT::RunSequence,SCRIPT,root["params"]["SequenceFile"].asString());
                SequenceThreadRun.detach();
                thread_num++;
                break;
            }
            /*搜索所有可以执行的脚本*/
            case "RemoteGetListAvalaibleDragScript"_hash:{
                std::thread DragScriptThread(&AIRSCRIPT::GetListAvalaibleDragScript,SCRIPT);
                DragScriptThread.detach();
                thread_num++;
                break;
            }
            /*运行脚本*/
            case "RemoteDragScript"_hash:{
                std::thread DragScriptThreadRun(&AIRSCRIPT::RemoteDragScript,SCRIPT,root["params"]["DragScriptFile"].asString());
                DragScriptThreadRun.detach();
                thread_num++;
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
     * describe: Send information to client both ws and wss
     * 描述：向ws和wss客户端发送信息
     * note: The message must be sent in the format of JSON
     */
    void WSSERVER::send(std::string message)
    {
        if(isConnected == true)
        {
            /*向WS客户端发送信息*/
            for (auto it : m_connections)
            {
                try
                {
                    m_server.send(it, message, websocketpp::frame::opcode::text);
                }
                catch (websocketpp::exception const &e)
                {
                    std::cerr << e.what() << std::endl;
                }
                catch (...)
                {
                    std::cerr << _("other exception") << std::endl;
                }
            }
        }
    }
    
    /*
     * name: stop()
     * describe: Stop the websocket server
     * 描述：停止WebSocket服务器
	 * calls: IDLog(const char *fmt, ...)
     */
    void WSSERVER::stop()
    {
        for (auto it : m_connections)
        {
            m_server.close(it, websocketpp::close::status::normal, _("Switched off by user."));
        }
        IDLog("Stop the server..\n");
        /*清除服务器句柄*/
        m_connections.clear();
        /*停止服务器*/
        m_server.stop();
    }

    /*
     * name: is_running()
     * @return Boolean function:服务器运行状态
     *  -false: server is not running||服务器不在运行
     *  -true: server is running||服务器正在运行
     * note: This function seems useless,Maybe you will never use it
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
	 * calls: IDLog(const char *fmt, ...)
     */
    void WSSERVER::run(int port)
    {
        try
        {
            IDLog(_("Start the server at port %d ...\n"),port);
            /*设置端口为IPv4模式并指定端口*/
            m_server.listen(websocketpp::lib::asio::ip::tcp::v4(),port);
            m_server.start_accept();
            m_server.run();
        }
        catch (websocketpp::exception const & e)
        {   
			std::cerr << e.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << _("other exception") << std::endl;
        }
    }
#endif

    /*
     * name: LoadConfigure()
     * describe: Load configure file
     * 描述：加载配置文件
     */
    bool WSSERVER::LoadConfigure()
    {
        std::string line,jsonStr;
        std::ifstream in("config/config.json", std::ios::binary);
        if (!in.is_open())
        {
            IDLog(_("Unable to open configuration file\n"));
            return false;
        }
        while (getline(in, line))
            jsonStr.append(line);
        in.close();
        std::unique_ptr<Json::CharReader>const json_read(reader.newCharReader());
        json_read->parse(jsonStr.c_str(), jsonStr.c_str() + jsonStr.length(), &root,&errs);
        /*获取基础配置信息*/
        MaxUsedTime = root["ServerConfig"]["Timeout"].asInt();
        MaxClientNumber = root["ServerConfig"]["MaxClientNum"].asInt();
        MaxThreadNumber = root["ServerConfig"]["MaxThreadNum"].asInt();
        return true;
    }

    /*
     * name: SetDashBoardMode()
     * describe: This is used to initialize the connection and send the version number.
     * 描述：初始化连接，并发送版本号
     * calls: send(std::string message)
     */
    void WSSERVER::SetDashBoardMode()
	{
        Json::Value Root;
		Root["result"] = Json::Value(1);
		Root["code"] = Json::Value();
		Root["Event"] = Json::Value("Version");
		Root["AIRVersion"] = Json::Value("2.0.0");
		json_message = Root.toStyledString();
		send(json_message);
	}

    /*
     * name: GetAstroAirProfiles()
     * describe: Gets the specified suffix file name in the folder
	 * 描述：获取文件夹中指定后缀文件名称
     * calls: send()
	 * note:The suffix of get file should be .air
     */
    void WSSERVER::GetAstroAirProfiles()
    {
        lock_guard<mutex> guard(mtx);
        /*寻找当前文件夹下的所有文件*/
        struct dirent *ptr;      
		DIR *dir;  
		std::string PATH = "config/";        //搜索目录，正式版本应该可以选择目录位置
		dir=opendir(PATH.c_str());      //打开目录
        std::vector<std::string> files;
        /*搜索所有符合条件的文件*/
		while((ptr = readdir(dir)) != NULL)  
		{  
			if(ptr->d_name[0] == '.' || strcmp(ptr->d_name,"..") == 0)  
				continue;
            /*判断文件后缀是否为.air*/
            int size = strlen(ptr->d_name);
            if(strcmp( ( ptr->d_name + (size - 4) ) , ".air") != 0)
                continue;
            files.push_back(ptr->d_name);
		}  
		closedir(dir);      //关闭目录
        /*判断是否找到配置文件*/
        Json::Value Root,profile;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteGetAstroAirProfiles");
        if(files.begin() == files.end())
        {
            IDLog(_("Cound not found any configration files,please check it\n"));
            Root["ActionResultInt"] = Json::Value(5);
            Root["Motivo"] = Json::Value(_("Cound not found any configration files"));
        }
        else
        {
            Root["ActionResultInt"] = Json::Value(4);
            Root["ParamRet"]["name"] = Json::Value(files[0]);
            FileName = files[0];
            FileBuf[0] = files[0];
            IDLog(_("Found configure file named %s\n"),files[0].c_str());
            files.erase(files.begin());
            for (int i = 0; i < files.size(); i++)  
            {
                FileBuf[i+1] = files[i];
                profile["name"] = Json::Value(files[i].c_str());
                Root["ParamRet"]["list"].append(profile);
                IDLog(_("Found configure file named %s\n"),files[i].c_str());
            }
        }
        /*整合信息并发送至客户端*/
        json_message = Root.toStyledString();
        send(json_message);
    }

    /*
     * name: SetProfile(std::string File_Name)
     * @param File_Name:Specify the file name
     * describe: Set up a new profile
     * 描述：设置新的配置文件
     * calls: send()
	 * note:The suffix of get file should be .air
     */
    void WSSERVER::SetProfile(std::string File_Name)
    {
        lock_guard<mutex> guard(mtx);
        IDLog(_("Change the configuration file to %s\n"),File_Name.c_str());
        FileName = File_Name;
        /*整合信息并发送至客户端*/
        Json::Value Root,profile;
		Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteGetAstroAirProfilesstartup");
        Root["ActionResultInt"] = Json::Value(4);
        Root["ParamRet"]["name"] = Json::Value(FileName);
        int i = 0;
        while(!FileBuf[i].empty())
        {
            i++;
            profile["name"] = Json::Value(FileBuf[i]);
            Root["ParamRet"]["list"].append(profile);
        }
        json_message = Root.toStyledString();
        send(json_message);
    }

    /*
     * name: SetupConnect(int timeout)
     * @param timeout:连接相机最长时间
     * describe: All connection profiles in the device
     * 描述：连接配置文件中的所有设备
     * calls: Connect(std::string Device_name)
     * calls: IDLog(const char *fmt, ...)
     * calls: IDLog_DEBUG(const char *fmt, ...)
     * calls: UnknownCamera()
     * calls: UnknownMount()
     * note: If it times out, an error message is returned
     */
    void WSSERVER::SetupConnect(int timeout)
    {
        /*读取config.air配置文件，并且存入参数中*/
        std::string line,jsonStr,path;
        path = "config/" + FileName;
        std::ifstream in(path.c_str(), std::ios::binary);
        /*打开文件*/
        if (!in.is_open())
        {
            IDLog(_("Unable to open configuration file\n"));
            IDLog_DEBUG(_("Unable to open configuration file\n"));
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
        bool connect_ok = false;
        auto start = std::chrono::high_resolution_clock::now();     //开始计时
        /*连接指定品牌的指定型号相机*/
        if(isCameraConnected == false)
        {
            bool camera_ok = false;
            bool Has_Camera = false;
            Camera = root["camera"]["brand"].asString();
            Camera_name = root["camera"]["name"].asString();
            if(!Camera.empty() && !Camera_name.empty())
            {
                Has_Camera = true;
                for(int i=1;i<=3;i++)
                {
                    const char* a = Camera.c_str();
                    switch(hash_(a))
                    {
                        #ifdef HAS_ASI
                        {
                            case "ZWOASI"_hash:{
                                /*初始化ASI相机，并赋值CCD*/
                                ASICCD *ASICamera = new ASICCD();
                                CCD = ASICamera;
                                camera_ok = CCD->Connect(Camera_name);
                                break;
                            }
                        }
                        #endif
                        #ifdef HAS_QHY
                        {
                            case "QHYCCD"_hash:{
                                /*初始化QHY相机，并赋值CCD*/
                                QHYCCD *QHYCamera = new QHYCCD();
                                CCD = QHYCamera;
                                camera_ok = CCD->Connect(Camera_name);
                                break;
                            }
                        }
                        #endif
                        #ifdef HAS_INDI
                        {
                            case "INDI"_hash:{
                                /*初始化INDI相机，并赋值CCD*/
                                INDICCD *INDIDevice = new INDICCD();
                                CCD = INDIDevice;
                                camera_ok = CCD->Connect(Camera_name);
                                break;
                            }
                        }
                        #endif
                        #ifdef HAS_GPhoto2
                        {
                            case "GPhoto2"_hash:{
                                /*初始化GPhoto2相机，并赋值CCD*/
                                GPhotoCCD *GPhotoCamera = new GPhotoCCD();
                                CCD = GPhotoCamera;
                                camera_ok = CCD->Connect(Camera_name);
                                break;
                            }
                        }
                        #endif
                        default:
                            UnknownDevice(301,_("Unknown camera"));		//未知相机返回错误信息
                    }
                    if(camera_ok == true)
                    {
                        isCameraConnected = true;
                        DeviceBuf[DeviceNum] = CCD->ReturnDeviceName();
                        WebLog(_("Connect to ")+DeviceBuf[DeviceNum]+_(" successfully"),2);
                        DeviceNum++;
                        connect_ok = true;
                        break;
                    }
                    if(i == 3&& camera_ok == false)
                    {
                        /*相机未连接成功，返回错误信息*/
                        SetupConnectError(5);
                        WebLog(_("Could not connect to ") + Camera_name,3);
                        connect_ok = false;
                        break;
                    }
                    sleep(4);
                }
            }
        }
        else
        {
            WebLog(Camera_name + _(" had already connected"),3);
        }
        /*连接指定品牌的指定型号赤道仪*/
        if(isMountConnected == false)
        {
            bool mount_ok = false;		//赤道仪连接状态
            bool Has_Mount = false;		//是否拥有赤道仪
            Mount = root["mount"]["brand"].asString();
            Mount_name = root["mount"]["name"].asString();
            if(!Mount.empty() && !Mount_name.empty())
            {
                Has_Mount = true;
                for(int i=1;i<=3;i++)
                {
                    const char* a = Mount.c_str();
                    switch(hash_(a))
                    {
                        /*初始化iOptron赤道仪，并赋值MOUNT*/
                        #ifdef HAS_IOPTRON
                        case "iOptron"_hash:{
                            IEQPro iOptronMount;
                            MOUNT = &iOptronMount;
                            mount_ok = MOUNT->Connect(Mount_name);
                            break;
                        }
                        #endif
                        #ifdef HAS_SKYWATCHER
                        case "SkyWatcher"_hash:{
                            /*初始化SkyWatcher赤道仪，并赋值MOUNT*/
                            SkyWatcher SkyWatcherMount;
                            MOUNT = &SkyWatcherMount;
                            mount_ok = MOUNT->Connect(Mount_name);
                            break;
                        }
                        #endif
                        #ifdef HAS_INDI
                        case "INDIMount"_hash:{
                            /*初始化INDI赤道仪，并赋值MOUNT*/
                            INDICCD *INDIDevice = new INDICCD();
                            MOUNT = INDIDevice;
                            mount_ok = MOUNT->Connect(Mount_name);
                            break;
                        }
                        #endif
                        default:
                            UnknownDevice(302,_("Unknown mount"));		//未知赤道仪返回错误信息
                    }
                    if(mount_ok == true)
                    {
                        isMountConnected = true;
                        DeviceBuf[DeviceNum] = MOUNT->ReturnDeviceName();
                        WebLog("Connect to "+DeviceBuf[DeviceNum]+" successfully",2);
                        DeviceNum++;
                        connect_ok = true;
                        break;
                    }
                    if(i == 3&& mount_ok == false)
                    {
                        /*赤道仪未连接成功，返回错误信息*/
                        SetupConnectError(5);
                        WebLog(_("Could not connect to ") + Mount_name,3);
                        connect_ok = false;
                        break;
                    }
                    sleep(4);
                }
            }
        }
        else
        {
            WebLog(Mount_name+" had already connected",3);
        }
        /*连接指定品牌的指定型号电动调焦座*/
        if(isFocusConnected == false)
        {
            bool focus_ok = false;
            bool Has_Focus = false;
            Focus = root["focus"]["brand"].asString();
            Focus_name = root["focus"]["name"].asString();
            if(!Focus.empty() && !Focus_name.empty())
            {
                Has_Focus = true;
                for(int i = 0;i<=3;i++)
                {
                    const char* a = Focus.c_str();
                    switch(hash_(a))
                    {
                        #ifdef HAS_ASIEAF
                        case "ASIEAF"_hash:{
                            /*初始化EAF电动调焦座，并赋FOCUS*/
                            FOCUS = &EAFFocus;
                            focus_ok = FOCUS->Connect(Focus_name);
                            break;
                        }
                        #endif
                        #ifdef HAS_GRUS
                        case "Grus"_hash:{
                            /*初始化Grus电动调焦座，并赋FOCUS*/
                            FOCUS = &GRUSFocus;
                            focus_ok = FOCUS->Connect(Focus_name);
                            break;
                        }
                        #endif
                        #ifdef HAS_INDI
                        case "INDIFocus"_hash:{
                            /*初始化INDI电动调焦座，并赋FOCUS*/
                            INDICCD *INDIDevice = new INDICCD();
                            FOCUS = INDIDevice;
                            focus_ok = FOCUS->Connect(Focus_name);
                            break;
                        }
                        #endif
                        default:
                            UnknownDevice(303,_("Unknown focus"));		//未知电动调焦座返回错误信息
                    }
                    if(focus_ok == true)
                    {
                        isFocusConnected = true;
                        DeviceBuf[DeviceNum] = FOCUS->ReturnDeviceName();
                        WebLog("Connect to "+DeviceBuf[DeviceNum]+" successfully",2);
                        DeviceNum++;
                        connect_ok = true;
                        break;
                    }
                    if(i == 3&& focus_ok == false)
                    {
                        /*电动调焦座未连接成功，返回错误信息*/
                        SetupConnectError(5);
                        WebLog("Could not connect to "+Focus_name,3);
                        connect_ok = false;
                        break;
                    }
                    sleep(4);
                }
            }
        }
        else
        {
            WebLog(Focus_name+" had already connected",3);
        }
        /*连接指定品牌的指定型号滤镜轮*/
        if(isFilterConnected == false)
        {
            bool filter_ok = false;
            bool Has_Filter = false;
            Filter = root["filter"]["brand"].asString();
            Filter_name = root["filter"]["name"].asString();
            if(!Filter.empty() && !Filter_name.empty())
            {
                Has_Filter = true;
                for(int i = 1;i<=3;i++)
                {
                    const char* a = Filter.c_str();
                    switch(hash_(a))
                    {
                        #ifdef HAS_ASIEFW
                        case "ASIEFW"_hash:{
                            /*初始化EFW滤镜轮，并赋FILTER*/
                            FILTER = &ASIFilter;
                            filter_ok = FILTER->Connect(Filter_name);
                            break;
                        }
                        #endif
                        #ifdef HAS_QHYCFW
                        case "QHYCFW"_hash:{
                            /*初始化QHY滤镜轮，并赋FILTER*/
                            FILTER = &QHYFilter
                            filter_ok = FILTER->Connect(Filter_name);
                            break;
                        }
                        #endif
                        #ifdef HAS_INDI
                        case "INDIFilter"_hash:{
                            /*初始化INDI滤镜轮，并赋FILTER*/
                            INDICCD *INDIDevice = new INDICCD();
                            FILTER = INDIDevice;
                            filter_ok = FILTER->Connect(Filter_name);
                            break; 
                        }
                        #endif
                        default:
                            UnknownDevice(304,"Unknown filter");		//未知滤镜轮返回错误信息
                    }
                    if(filter_ok == true)
                    {
                        isFilterConnected = true;
                        DeviceBuf[DeviceNum] = FILTER->ReturnDeviceName();
                        WebLog("Connect to "+DeviceBuf[DeviceNum]+" successfully",2);
                        DeviceNum++;
                        connect_ok = true;
                        break;
                    }
                    if(i == 3&& filter_ok == false)
                    {
                        /*滤镜轮未连接成功，返回错误信息*/
                        SetupConnectError(5);
                        WebLog("Could not connect to "+Filter_name,3);
                        connect_ok = false;
                        break;
                    }
                    sleep(4);
                }
            }
        }
        else
        {
            WebLog(Filter_name+" had already connected",3);
        }
        /*连接指定品牌的指定型号导星软件*/
        if(isGuideConnected == false)
        {
            bool guide_ok = false;
            bool Has_Guide = false;
            Guide = root["Guide"]["brand"].asString();
            Guide_name = root["Guide"]["name"].asString();
            if(!Guide.empty() && !Guide_name.empty())
            {
                Has_Guide = true;
                for(int i = 1;i<=3;i++)
                {
                    const char* a = Guide.c_str();
                    switch(hash_(a))
                    {
                        /*Max：事实上我们一般只会使用PHD2，所以LinGuider可以等其他做好以后再做*/
                        #ifdef HAS_PHD2
                        case "PHD2"_hash:{
                            GUIDE = &PHD2;
                            guide_ok = GUIDE->Connect(Guide_name);
                            break;
                        }
                        #endif
                        #ifdef HAS_LINGUIDER
                        case "LinGuider"_hash:{
                            GUIDE = &LinGuider
                            guide_ok = GUIDE->Connect(Guide_name);
                            break;
                        }
                        #endif
                        default:
                            UnknownDevice(305,"Unknown guide server");		//未知导星软件返回错误信息
                    }
                    if(guide_ok == true)
                    {
                        isGuideConnected = true;
                        DeviceBuf[DeviceNum] = GUIDE->ReturnDeviceName();
                        WebLog("Connect to "+DeviceBuf[DeviceNum]+" successfully",2);
                        DeviceNum++;
                        connect_ok = true;
                        break;
                    }
                    if(i == 3&& guide_ok == false)
                    {
                        /*导星软件未连接成功，返回错误信息*/
                        SetupConnectError(5);
                        WebLog("Could not connect to "+Guide_name,3);
                        connect_ok = false;
                        break;
                    }
                    sleep(4);
                }
            }
        }
        else
        {
            WebLog(Guide_name+" had already connected",3);
        }
        auto end = std::chrono::high_resolution_clock::now();       //停止计时
        std::chrono::duration<double> diff = end - start;
        IDLog("Connecting to device took %g seconds\n", diff.count());
        if(diff.count() >= 20)
        {
            SetupConnectError(8);
            return;
        }
        /*判断设备是否完全连接成功*/
        if(connect_ok == true)
        {
            SetupConnectSuccess();		//将连接上的设备列表发送给客户端
            EnvironmentDataSend();
            WebLog(_("All devices connected successfully"),2);
        }
        else
        {
            SetupConnectError(5);
            WebLog(_("There were some errors in connecting the device"),3);
        }
        return;
    }
    
    void WSSERVER::SetupDisconnect(int timeout)
    {
        bool disconnect_ok = false;
        if(isCameraConnected == true)
        {
            if((disconnect_ok = CCD->Disconnect()) == true)
                WebLog("Disconnect from"+Camera_name,2);
            else
                WebLog("Could not Disconnect from"+Camera_name,3);
            isCameraConnected = false;
            delete [] CCD;
        }
        if(isMountConnected == true)
        {
            if((disconnect_ok = MOUNT->Disconnect()) == true)
                WebLog("Disconnect from"+Mount_name,2);
            else
                WebLog("Could not Disconnect from"+Mount_name,3);
            isMountConnected = false;
            delete [] MOUNT;    
        }
        if(isFocusConnected == true)
        {
            if((disconnect_ok = FOCUS->Disconnect()) == true)
                WebLog("Disconnect from"+Focus_name,2);
            else
                WebLog("Could not Disconnect from"+Focus_name,3);
            isFocusConnected = false;
            delete [] FOCUS;
        }
        if(isFilterConnected == true)
        {
            if((disconnect_ok = FILTER->Disconnect()) == true)
                WebLog("Disconnect from"+Filter_name,2);
            else
                WebLog("Could not Disconnect from"+Filter_name,3);
            isFilterConnected = false;
            delete [] FILTER;
        }
        if(isGuideConnected == true)
        {
            if((disconnect_ok = GUIDE->Disconnect()) == true)
                WebLog("Disconnect from"+Guide_name,2);
            else
                WebLog("Could not Disconnect from"+Guide_name,3);
            isGuideConnected = false;
        }
        if(disconnect_ok == true)
        {
            WebLog(_("Successfully disconnected from all devices"),2);
            SetupDisconnectSuccess();
        }
    }

    /*
     * name: GetFilterConfiguration()
     * describe: Get filter wheel list
     * 描述： 获取滤镜轮列表
     * calls: send()
     */
    void WSSERVER::GetFilterConfiguration()
    {
        /*整合信息并发送至客户端*/
        Json::Value Root;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteGetFilterConfiguration");
        Root["ActionResultInt"] = Json::Value(4);
        json_message = Root.toStyledString();
        send(json_message);
    }

    /*
     * name: EnvironmentDataSend()
     * describe: Return to the list of connected devices
     * 描述： 返回已连接设备列表
     * calls: send()
     */
    void WSSERVER::EnvironmentDataSend()
    {
        /*整合信息并发送至客户端*/
        Json::Value Root,profile;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteGetEnvironmentData");
        Root["ActionResultInt"] = Json::Value(4);
        for(int i = 0;i<DeviceNum;i++)
            Root["ParamRet"].append(DeviceBuf[i]);
        json_message = Root.toStyledString();
        send(json_message);
    }

    /*
     * name: Connect(std::string Device_name)
     * @param Device_name:连接相机名称
     * describe: Connect the camera
     * 描述： 连接相机
     * calls: IDLog(const char *fmt, ...)
     * calls: IDLog_DEBUG(const char *fmt, ...)
	 * note:This function should not be executed normally
     */
    bool WSSERVER::Connect(std::string Device_name)
    {
		/*默认情况下不应该执行这个函数*/
        IDLog("Try to establish a connection with %s,Should never get here.\n",Device_name.c_str());
        IDLog_DEBUG("Try to establish a connection with %s,Should never get here.\n",Device_name.c_str());
        return true;
    }
    
    /*
     * name: Disconnect()
     * describe: Disconnect from camera
     * 描述：与相机断开连接
     * calls: IDLog(const char *fmt, ...)
     * calls: IDLog_DEBUG(const char *fmt, ...)
     * note: This function should not be executed normally
     */
    bool WSSERVER::Disconnect()
    {
		/*默认情况下不应该执行这个函数*/
        IDLog("Try to disconnect from %s,Should never get here.\n",Camera_name.c_str());
        IDLog_DEBUG("Try to disconnect from %s,Should never get here.\n",Camera_name.c_str());
        return true;
    }

    /*
     * name: ReturnDeviceName()
     * describe: Get device name
     * 描述：获取设备名称
     * calls: IDLog(const char *fmt, ...)
     * calls: IDLog_DEBUG(const char *fmt, ...)
     * note: This function should not be executed normally
     */
    std::string WSSERVER::ReturnDeviceName()
    {
		/*默认情况下不应该执行这个函数*/
        IDLog("Try to disconnect from %s,Should never get here.\n",Camera_name.c_str());
        IDLog_DEBUG("Try to disconnect from %s,Should never get here.\n",Camera_name.c_str());
        return "False";
    }

    /*
     * name: SetupConnectSuccess()
     * describe: Successfully connect device
     * 描述：成功连接设备
     * calls: IDLog(const char *fmt, ...)
     * calls: send()
     */
    void WSSERVER::SetupConnectSuccess()
    {
        IDLog("Successfully connect device\n");
        /*整合信息并发送至客户端*/
        Json::Value Root;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteSetupConnect");
        Root["ActionResultInt"] = Json::Value(4);
        json_message = Root.toStyledString();
        send(json_message);
    }
    
    /*
     * name: SetupConnectError(int id)
     * describe: Error handling connection to device
     * 描述：处理连接设备时的错误
     * calls: IDLog(const char *fmt, ...)
     * calls: IDLog_DEBUG(const char *fmt, ...)
     * calls: send()
     */
    void WSSERVER::SetupConnectError(int id)
    {
        IDLog("Unable to connect device\n");
        IDLog_DEBUG("Unable to connect device\n");
        /*整合信息并发送至客户端*/
        Json::Value Root;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteSetupConnect");
        Root["ActionResultInt"] = Json::Value(id);
        json_message = Root.toStyledString();
        send(json_message);
    }

    /*
     * name: SetupDisconnectSuccess()
     * describe: Successfully disconnect from device
     * 描述：与设备断开连接
     * calls: IDLog(const char *fmt, ...)
     * calls: send()
     */
    void WSSERVER::SetupDisconnectSuccess()
    {
        /*防止多次连接导致错误*/
        memset(DeviceBuf,0,5);
        DeviceNum = 0;
        IDLog("Successfully disconnect from devices\n");
        /*整合信息并发送至客户端*/
        Json::Value Root;
        Root["Event"] = Json::Value("RemoteActionResult");
        Root["UID"] = Json::Value("RemoteSetupDisconnect");
        Root["ActionResultInt"] = Json::Value(4);
        json_message = Root.toStyledString();
        send(json_message);
    }

//----------------------------------------对焦----------------------------------------

    bool WSSERVER::MoveTo(int TargetPosition)
    {
        return true;
    }

//----------------------------------------滤镜轮----------------------------------------

    bool WSSERVER::FilterMoveTo(int TargetPosition)
    {
        return true;
    }

//----------------------------------------解析----------------------------------------

    

//----------------------------------------计划拍摄----------------------------------------

    

//----------------------------------------日志----------------------------------------

    /*
	 * name: WebLog(std::string message,int type)
     * @param message:发送的信息
     * @param type：信息类型
	 * describe: Send message and display on client
	 * 描述：发送日志信息并在客户端显示
     * calls: send()
	 */
    void WebLog(std::string message,int type)
    {
        Json::Value Root;
        Root["Event"] = Json::Value("LogEvent");
        Root["Type"] = Json::Value(type);
        Root["Text"] = Json::Value(message);
        Root["TimeInfo"] = Json::Value(timestamp());
        std::string json_message = Root.toStyledString();
        ws.send(json_message);
    }

//----------------------------------------错误代码----------------------------------------

    /*
     * name: UnknownMsg()
     * describe: Processing unknown information from clients
     * 描述：处理来自客户端的未知信息
     * calls: IDLog(const char *fmt, ...)
     * calls: IDLog_DEBUG(const char *fmt, ...)
     * calls: send()
	 * note:If this function is executed, an error will appear on the web page
     */
    void WSSERVER::UnknownMsg()
    {
        IDLog("An unknown message was received from the client\n");
        IDLog_DEBUG("An unknown message was received from the client\n");
        /*整合信息并发送至客户端*/
        Json::Value Root,error;
        Root["result"] = Json::Value(1);
		Root["code"] = Json::Value();
        Root["id"] = Json::Value(403);
        error["message"] = Json::Value("Unknown information");
        Root["error"] = error;
        json_message = Root.toStyledString();
        send(json_message);
    }
    
    /*
     * name: UnknownDevice(int id,std::string message);
     * describe: Process the unknown device information and return to the client
     * 描述：处理未知设备信息，并返回至客户端
     * calls: IDLog(const char *fmt, ...)
     * calls: send()
	 * note:Execute this function if an unknown device is found
     */
    void WSSERVER::UnknownDevice(int id,std::string message)
    {
        IDLog("An unknown device was found,please check the connection\n");
        /*整合信息并发送至客户端*/
        Json::Value Root,error;
        Root["result"] = Json::Value(1);
		Root["code"] = Json::Value();
        Root["id"] = Json::Value(id);
        error["message"] = Json::Value(message);
        Root["error"] = error;
        json_message = Root.toStyledString();
        send(json_message);
    }

    void WSSERVER::ClientNumError()
    {
        IDLog("There are too many clients connected with server\n");
        /*整合信息并发送至客户端*/
        Json::Value Root,error;
        Root["result"] = Json::Value(1);
		Root["code"] = Json::Value();
        Root["id"] = Json::Value(601);
        error["message"] = Json::Value("客户端数量过多");
        Root["error"] = error;
        json_message = Root.toStyledString();
        send(json_message);
    }
    
    void WSSERVER::ErrorCode()
    {
		
	}
	
    /*
     * name: Polling()
     * describe: The client remains connected to the server
     * 描述：客户端与服务器保持连接
     * calls: send()
	 * note:This function is most commonly used
     */
    void WSSERVER::Polling()
    {
        Json::Value Root;
        Root["result"] = Json::Value(1);
		Root["code"] = Json::Value();
        Root["Event"] = Json::Value("Polling");
        json_message = Root.toStyledString();
        send(json_message);
    }
        
}
