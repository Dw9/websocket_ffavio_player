

#########################################################################

###ffmpeg
#include_directories("/root/nvidia/tmp/FFmpeg/ffmpeg_build/include")
#link_directories("/root/nvidia/tmp/FFmpeg/ffmpeg_build/lib")


#include_directories(/Users/weishao/opt/ffmpeg@4/4.4.4/include/)
#link_directories(/Users/weishao/opt/ffmpeg@4/4.4.4/lib)


###sdl2.0
#set(FIND_SDL2_PATHS D:\\3rdpart\\SDL2-2.24.0)
#find_path(SDL2_INCLUDE_DIR SDL2
#        PATH_SUFFIXES include
#        PATHS ${FIND_SDL2_PATHS})
#
#find_library(SDL2_LIBRARY
#        NAMES SDL2 SDL2main
#        PATH_SUFFIXES lib
#        PATHS ${FIND_SDL2_PATHS})
#include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_SUPPRESS_REGENERATION true)


find_path(SDL2_INCLUDE_DIR SDL2/SDL.h)
find_library(SDL2_LIBRARY SDL2 SDL2main)

MESSAGE(${SDL2_INCLUDE_DIR})
MESSAGE(${SDL2_LIBRARY})


find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
find_library(AVDEVICE_LIBRARY avdevice)

find_path(AVFILTER_INCLUDE_DIR libavfilter/avfilter.h)
find_library(AVFILTER_LIBRARY avfilter)

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)
find_library(SWSCALE_LIBRARY swscale)

set(INC_DIRS ${PROJECT_SOURCE_DIR}/include ${SDL2_INCLUDE_DIR} ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${AVDEVICE_INCLUDE_DIR} ${AVFILTER_INCLUDE_DIR} ${SWSCALE_INCLUDE_DIR})
set(LIBS json ZLMediaKit::MediaKit zltoolkit ${SDL2_LIBRARY} ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${AVDEVICE_LIBRARY} ${AVFILTER_LIBRARY} ${SWSCALE_LIBRARY})

set(PROJECT_INCLUDE_DIRS ${INC_DIRS})
MESSAGE(${AVFORMAT_LIBRARY})

file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
add_executable(webscoket_ffavio_player ${SRC_LIST} )


target_include_directories(webscoket_ffavio_player PUBLIC ${PROJECT_INCLUDE_DIRS})
target_link_libraries(webscoket_ffavio_player ${LIBS})


#target_link_libraries(webscoket_ffavio_player json ZLMediaKit::MediaKit zltoolkit ${LINK_LIB_LIST}
#        ${SDL2_LIBRARIES}
#        avformat
#        avcodec
#        avdevice
#        avfilter
#        avutil
#        postproc
#        swresample
#        swscale
#        pthread
#        )
