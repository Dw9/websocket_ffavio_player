include_directories(./)

#########################################################################
set(SRC_LIST  webscoket_ffavio_player.cpp)
###ffmpeg
include_directories("/root/nvidia/tmp/FFmpeg/ffmpeg_build/include")
link_directories("/root/nvidia/tmp/FFmpeg/ffmpeg_build/lib")



add_executable(webscoket_ffavio_player ${SRC_LIST})
target_link_libraries(webscoket_ffavio_player json ZLMediaKit::MediaKit zltoolkit ${LINK_LIB_LIST}
        avformat
        avcodec
        avutil
        swresample
        swscale
        avfilter
        pthread
        )
