//
// Created by 韦少 on 2023/6/21.
//

#ifndef WEBSCOKET_FFAVIO_PLAYER_FFMPEG_AVIO_H
#define WEBSCOKET_FFAVIO_PLAYER_FFMPEG_AVIO_H
#include <string>
#include "ringbuffer.h"
#include "XDecode.h"
#include "XPlayerAVSync.h"

using namespace std;

class Demuxer
{
public:
    Demuxer();
    ~Demuxer();

    void doDemuxer(std::shared_ptr<ringbuffer> ring);
    void setYuvCallback(std::function<void (const char* y,int y_len,const char* u,int u_len,const char* v,int v_len,int width,int heigh)> cb){
        m_cb = cb;
    }
public:
    uint8_t *m_ioctx_buffer;
    AVIOContext *m_ioctx;
    AVFormatContext * m_ic;
    std::shared_ptr<XDecodeVideo> m_vd;
    std::shared_ptr<VideoState> m_is;
    std::function<void (const char* y,int y_len,const char* u,int u_len,const char* v,int v_len,int width,int heigh)> m_cb;

};

#endif //WEBSCOKET_FFAVIO_PLAYER_FFMPEG_AVIO_H
