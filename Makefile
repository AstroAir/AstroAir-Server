#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#  

CC = g++

OUTPUT = -g -std=c++17 -c
OUT = -g -std=c++17 -o
#颜色配置文件
GREEN = "\e[32;1m"
BLUE = "\e[34;1m"
RED = "\e[31;1m"
#定义使用平台
PLATFORM = armv7
#定义库
WEBSOCKETPP = -D_WEBSOCKETPP_CPP11_STL_ -DASIO_STANDALONE -lpthread
JSON = -ljsoncpp
ASILIB = -Iair-asi/ -Llibasi/armv7/ -Ilibasi/ -lASICamera2 -lusb-1.0
NOVA = -lnova

all: wsserver astroair-server
#构建主服务器
astroair-server:wsserver main.cpp
	@echo $(GREEN)"Building C++ WebSocket Server ... \033[0m"
	@$(CC) $(OUT) server main.o wsserver.o asi_ccd.o logger.o $(WEBSOCKETPP) $(JSON) $(ASILIB)
	@echo $(BLUE)"Finished Building C++ WebSocket Server ... \033[0m"
#构建WebSocketpp库
wsserver:wsserver.cpp wsserver.h
	@echo $(GREEN)"Building C++ WebSocket Server LIB ... \033[0m"
	@$(CC) $(OUTPUT) main.cpp libastro.cpp wsserver.cpp air-asi/asi_ccd.cpp logger.cpp $(WEBSOCKETPP) $(JSON) $(ASILIB) $(NOVA)
	@echo $(BLUE)"Finished Building C++ WebSocket Server LIB ... \033[0m"
#本地化构建
install:
	@echo "-- Install C++ WebSocket Server "
	@sudo cp astroair-server /usr/bin
#清理编译文件
clean:
	@echo $(RED)"Deleting All Building Files ... \033[0m"
	@rm server *.o server *.txt
	@echo $(RED)"Finished Deleting All Building Files ... \033[0m"
