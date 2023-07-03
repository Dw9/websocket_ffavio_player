

#########################################################################

###ffmpeg
#include_directories("/root/nvidia/tmp/FFmpeg/ffmpeg_build/include")
#link_directories("/root/nvidia/tmp/FFmpeg/ffmpeg_build/lib")


include_directories(/Users/weishao/opt/ffmpeg@4/4.4.4/include/)
link_directories(/Users/weishao/opt/ffmpeg@4/4.4.4/lib)

###sdl2.0
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})



file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
add_executable(webscoket_ffavio_player ${SRC_LIST} )

target_link_libraries(webscoket_ffavio_player json ZLMediaKit::MediaKit zltoolkit ${LINK_LIB_LIST}
        ${SDL2_LIBRARIES}
        avformat
        avcodec
        avdevice
        avfilter
        avutil
        postproc
        swresample
        swscale
        pthread
        )
