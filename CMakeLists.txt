cmake_minimum_required(VERSION 3.16)

project(RbProxy VERSION 0.1)

add_library(RbProxyObjs OBJECT src/socks5.cpp)

target_include_directories(RbProxyObjs PRIVATE include)

target_precompile_headers(RbProxyObjs PRIVATE include/IOInterface.hpp include/socks5.hpp include/RBCommon.hpp)

include(CheckIncludeFileCXX)

# 主要是为了判断一下 ntohs 这个函数可以从 Winsock2.h 里面获得还是 arpa/inet.h 里面获得
CHECK_INCLUDE_FILE_CXX(arpa/inet.h _having_inet_h)
if(_having_inet_h)
	target_compile_definitions(RbProxyObjs PUBLIC HAVING_INET_H)
endif()

# 主要是为了判断一下 ntohs 这个函数可以从 Winsock2.h 里面获得还是 arpa/inet.h 里面获得
CHECK_INCLUDE_FILE_CXX(Winsock2.h _having_winsock2_h)
if(_having_winsock2_h)
	target_compile_definitions(RbProxyObjs PUBLIC HAVING_WINSOCK2_H)
endif()

# 如果能直接使用 asio.hpp，通常意味着这是在linux底下并且已经安装了 libasio-dev，直接使用即可
# 否则可能是 windows 底下的 vcpkg，需要手动链接一下文件
CHECK_INCLUDE_FILE_CXX(asio.hpp _having_asio_hpp)
if("${_having_asio_hpp}" STREQUAL "")
	find_package(asio CONFIG REQUIRED)
	target_link_libraries(RbProxyObjs PRIVATE asio asio::asio)
endif()
