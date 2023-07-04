//
// Created by 韦少 on 2023/6/21.
//

#ifndef WEBSCOKET_FFAVIO_PLAYER_XDECODE_H
#define WEBSCOKET_FFAVIO_PLAYER_XDECODE_H
#pragma once
#ifdef D3D_ON
#include "D3DVidRender.h"
#include "ffmpeg_dxva2.h"
#endif // D3D_ON
#include "avffmpeg.h"
#include <memory>
#include <mutex>
#include <functional>


#define DVX2 0


class Ticker_ {
public:
    Ticker_() {
        _created = _begin = av_gettime() / 1000;
    }
    uint64_t createdTime() const {
        return av_gettime() / 1000 - _created;
    }

private:
    uint64_t _begin;
    uint64_t _created;
};

class FFmpegFrame {
public:
    using Ptr = std::shared_ptr<FFmpegFrame>;

    FFmpegFrame(std::shared_ptr<AVFrame> frame = nullptr);
    ~FFmpegFrame();

    AVFrame* get() const;

private:
    char* _data = nullptr;
    std::shared_ptr<AVFrame> _frame;
};


class XDecodeVideo
{
public:
    XDecodeVideo();
    virtual ~XDecodeVideo();
    //�򿪽�����,���ܳɹ�����ͷ�para�ռ�
    virtual bool Open(AVCodecParameters* para, AVFormatContext* inputContext,  int media_type = 0);
    void*  getVideoHwnd() {
        return m_videoH ? m_videoH : nullptr;
    }
    void onDecode(const AVFrame* frame);
    void setOnDecode(std::function<void(const AVFrame*)> cb);

    void decodeFrame(const AVPacket* avpkt);
    int InitRenderWindows(const char* videoHwnd);
    int getSnap(std::string filepath);

    AVCodecContext* getVideoContext() {
        return _context.get();
    }
private:
    int initEncoderCodec(AVStream* inputStream, AVCodecContext** encodeContext);
    void EncodePic(AVCodecContext* encodeContext, AVFrame* frame);
    void onGetSnap(const AVFrame* pFrame);
    int  WritePacket(std::shared_ptr<AVPacket> packet);
    int OpenOutput(std::string outUrl);
    void CloseOutput();
protected:
    std::function<void(const AVFrame*)> _cb;
    Ticker_ _Ticker_;


    AVCodecContext* m_encodeContext = nullptr;   //��ͼ����codec
    bool m_getPict = false;                    //��ͼ��־λ
    bool m_bAccel = true;
    std::mutex m_mux;
    void* m_videoH = nullptr;
    AVFormatContext* m_outputContext = nullptr;  //����Ҫ�ͷ�  ����demux��
    AVFormatContext* m_inputContext =  nullptr;



public:
    std::shared_ptr<AVCodecContext> _context;
    AVFrame* m_pFrame = nullptr;
private:
    AVFrame* m_YUVPic = nullptr;
};


#endif //WEBSCOKET_FFAVIO_PLAYER_XDECODE_H
