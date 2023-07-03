cmake_minimum_required(VERSION 3.24)
project(webscoket_ffavio_player)

set(CMAKE_CXX_STANDARD 14)

include(CheckStructHasMember)
include(CheckSymbolExists)

list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
check_struct_has_member("struct mmsghdr" msg_hdr sys/socket.h HAVE_MMSG_HDR)
check_symbol_exists(sendmmsg sys/socket.h HAVE_SENDMMSG_API)
check_symbol_exists(recvmmsg sys/socket.h HAVE_RECVMMSG_API)

if(HAVE_MMSG_HDR)
    add_definitions(-DHAVE_MMSG_HDR)
endif()
if(HAVE_SENDMMSG_API)
    add_definitions(-DHAVE_SENDMMSG_API)
endif()
if(HAVE_RECVMMSG_API)
    add_definitions(-DHAVE_RECVMMSG_API)
endif()

#设置库文件路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
#设置可执行程序路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

link_directories(${LIBRARY_OUTPUT_PATH})

#设置工程源码根目录
message(${CMAKE_SOURCE_DIR})
set(ToolKit_Root ${CMAKE_SOURCE_DIR}/deps/ZLMediaKit/3rdpart/ZLToolKit/src)
set(Json_Root ${CMAKE_SOURCE_DIR}/deps/ZLMediaKit/3rdpart/jsoncpp)
#set(Proxy_Root ${CMAKE_SOURCE_DIR}/src)
set(Zlm_Root_src ${CMAKE_SOURCE_DIR}/deps/ZLMediaKit/src)



#设置头文件目录
INCLUDE_DIRECTORIES(${ToolKit_Root})
INCLUDE_DIRECTORIES(${Json_Root}/include/)
INCLUDE_DIRECTORIES(${Zlm_Root_src}/)


#收集源代码
file(GLOB ToolKit_src_list ${ToolKit_Root}/*/*.cpp ${ToolKit_Root}/*/*.h ${ToolKit_Root}/*/*.c)
file(GLOB Json_src_list ${Json_Root}/src/lib_json/*.cpp)
#file(GLOB Proxy_src_list ${Proxy_Root}/*.cpp ${Proxy_Root}/*.h ${Proxy_Root}/*.c)
file(GLOB MediaKit_SRC_LIST
        ${Zlm_Root_src}/*/*.c
        ${Zlm_Root_src}/*/*.cpp
        ${Zlm_Root_src}/*/*.h)

if(USE_SOLUTION_FOLDERS AND (NOT GROUP_BY_EXPLORER))
    # 在 IDE 中对文件进行分组, 源文件和头文件分开
    set_file_group("${CMAKE_CURRENT_SOURCE_DIR}" ${MediaKit_SRC_LIST})
endif()




#去除win32的适配代码
if (NOT WIN32)
    list(REMOVE_ITEM ToolKit_src_list ${ToolKit_Root}/win32/getopt.c)
endif ()



#添加库
add_library(zltoolkit STATIC ${ToolKit_src_list})
add_library(json STATIC ${Json_src_list})
#add_library(proxy STATIC ${Proxy_src_list})
# 添加库
add_library(zlmediakit STATIC ${MediaKit_SRC_LIST})
add_library(ZLMediaKit::MediaKit ALIAS zlmediakit)


set(LINK_LIB_LIST pthread)
option(ENABLE_OPENSSL "Enable openssl" off)


add_subdirectory(tests)
