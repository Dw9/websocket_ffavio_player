//
// Created by 韦少 on 2023/6/21.
//
#pragma once
#include <stdint.h>
#include <string>
#include "avffmpeg.h"
#include "sdl2.h"
#include "XDecode.h"
#include "PacketQueue.h"

enum {
    AV_SYNC_AUDIO_MASTER,
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_MASTER,
};

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000
#define MAX_AUDIOQ_SIZE (40)
#define MAX_VIDEOQ_SIZE (30)
#define MAX_AUDIO_FRAME_BUF_SIZE (4096 * 10)
#define MAX_VIDEO_FRAME_NUM_SIZE 1
#define MAXVIDEOFRAMENUM 10
#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0
#define SAMPLE_CORRECTION_PERCENT_MAX 10
#define AUDIO_DIFF_AVG_NB 20
#define FF_ALLOC_EVENT   (SDL_USEREVENT)
#define FF_REFRESH_EVENT (SDL_USEREVENT + 1)
#define FF_QUIT_EVENT (SDL_USEREVENT + 2)
#define VIDEO_PICTURE_QUEUE_SIZE 1
#define DEFAULT_AV_SYNC_TYPE AV_SYNC_AUDIO_MASTER

typedef struct VideoState {
    struct PlayerCallback* pCallback = NULL;
    AVFormatContext* pFormatCtx = NULL;
    int             videoStream = -1, audioStream = -1;
    int             video_started = 0, audio_started = 0, draw_started = 0;
    int             mediaType = -1;   // 0音频   1视频 2 音视频
    int             playId = -1;


    //默认开启音视频同步 0 不开启  1 开启  但实际测试的时候经常有音视频pts差距太多，音频解码数据不对不够导致的不同步  视频卡住或者音频卡住等待 所以增加此参数
    int             enable_avsync = 1;

    //默认sdl 硬件渲染 0 不开启  1 开启   实测部分客户机器在硬件渲染的时候内存占用极高 在打包32位的时候 内存也只能到1g左右  后续不在分配内存了 导致程序出现异常
    int             enable_hardward_render = 1;

    //动态调整音视频以防不正常的track导致崩溃
    int enable_video = 1;
    int enable_audio = 1;

    //视频参数相关
    int width = 0;
    int height = 0;
    int fps = 0;
    enum AVCodecID code_id_v;
    std::string code_name_v;
    enum AVPixelFormat pix_fmt;
    AVRational time_base;
    AVRational sample_aspect_ratio;
    //AVRational frame_rate;

    //音频相关参数
    int sample_rate;
    int channels;
    int frame_size;
    enum AVCodecID code_id_a;
    std::string code_name_a;

    int sdl_len = 0; //sdl 回调每次渲染帧长

    int             av_sync_time = 3; // 同步的阈值 3秒
    int             av_sync_type = DEFAULT_AV_SYNC_TYPE;
    double          external_clock = 0; /* external clock base */
    double          external_clock_time = 0;
    int             seek_req = 0;        //seek 请求
    int             seek_end_video = 1;  //视频是否已经seek到指定位置
    int             seek_end_audio = 1;  //音频是否已经seek到指定位置
    int             seek_end_av = 1;     //总的标志位
    int             seek_flags = 0;   //seek的标志
    int64_t         seek_pos = 0;    // 需要seek到的位置         单位毫秒
    int64_t         now_pos = 0;     // 当前音视频所播放的位置   单位毫秒
    int64_t         video_pos = 0;     // 当前视频所播放的位置   单位毫秒
    int64_t         audio_pos = 0;     // 当前音频所播放的位置   单位毫秒

    int             eof_end_video = 1;  //视频是否已经eof
    int             eof_end_audio = 1;  //音频是否已经eof
    int             eof_end_av = 1;     //总的eof标志位

    double          audio_clock = 0;  //单位 s
    AVStream* audio_st = NULL;
    PacketQueue* audioq = NULL;
    AVFrame* audio_frame = NULL;
    uint8_t         audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2] = { 0 };
    unsigned int    audio_buf_size = 0;
    unsigned int    audio_buf_index = 0;
    AVPacket        audio_pkt;
    uint8_t* audio_pkt_data = NULL;
    int             audio_pkt_size = 0;
    int             audio_hw_buf_size = 0;
    double          audio_diff_cum = 0; /* used for AV difference average computation */
    double          audio_diff_avg_coef = 0;
    double          audio_diff_threshold = 0;
    int             audio_diff_avg_count = 0;
    double          frame_timer = 0;
    double          frame_last_extern_stamp = 0;
    double          frame_last_pts = 0;
    double          frame_last_delay = 0;
    double          video_clock = 0; ///<pts of last decoded frame / predicted pts of next decoded frame
    double          video_current_pts = 0; ///<current displayed pts (different from video_clock if frame fifos are used)
    int64_t         video_current_pts_time = 0;  ///<time (av_gettime) at which we updated video_current_pts - used to have running video pts
    AVStream* video_st = NULL;
    PacketQueue* videoq = NULL;
    AVFrame* video_frame;
    //VideoPicture    pictq[VIDEO_PICTURE_QUEUE_SIZE];
    int             pictq_size = 0, pictq_rindex = 0, pictq_windex = 0;
    SDL_mutex* pictq_mutex;
    SDL_cond* pictq_cond;
    //SDL_Thread* parse_tid;
    //SDL_Thread* video_tid;

    char            filename[1024];
    int             quit;

    int video_frame_size      = MAX_VIDEO_FRAME_NUM_SIZE;
    int audio_frame_buf_size  = MAX_AUDIO_FRAME_BUF_SIZE;

    int audio_max_packet_size = MAX_AUDIOQ_SIZE;
    int video_max_packet_size = MAX_VIDEOQ_SIZE;


} VideoState;

