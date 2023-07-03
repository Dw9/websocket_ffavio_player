/*
 * Copyright (c) 2016 The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/xia-chu/ZLMediaKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#include <string>
#include "Util/logger.h"
#include "Http/WebSocketClient.h"
#include "ringbuffer.h"
#include "XPlayerAVSync.h"
#include "ffmpeg_avio.h"
#include "Util/logger.h"
#include "YuvDisplayer.h"
using namespace toolkit;


using namespace std;
using namespace toolkit;
using namespace mediakit;

class WsSession : public TcpClient {
public:
    WsSession(const EventPoller::Ptr &poller = nullptr){
        InfoL;
    }

    ~WsSession() override {
        InfoL;
    }

    void setRing(std::shared_ptr<ringbuffer> ring){
        m_ring = ring;
    }

protected:
    void onRecv(const Buffer::Ptr &pBuf) override {
        //DebugL << pBuf->size();
        if(m_ring){
            ringbuffer_in(m_ring.get(),(char *)pBuf->data(),pBuf->size());
        }
#ifdef DUMP_FILE
        if(fp){
            fwrite(pBuf->data(),pBuf->size(),1,fp.get());
        }
#endif
    }
    //被动断开连接回调
    void onError(const SockException &ex) override {
        WarnL << ex;
    }
    //tcp连接成功后每2秒触发一次该事件
    void onManager() override {
        //SockSender::send("echo test!");
    }
    //连接服务器结果回调
    void onConnect(const SockException &ex) override{
        DebugL << ex;
#ifdef DUMP_FILE
        fp = std::shared_ptr<FILE>(fopen("test.flv", "wb+"), [](FILE *fp) {
            if (fp) {
                fclose(fp);
            }
        });
        if(!fp){
            WarnL<<"打开文件失败";
        }
#endif
    }

    //数据全部发送完毕后回调
    void onFlush() override{
        DebugL;
    }

private:
#ifdef DUMP_FILE
    std::shared_ptr<FILE> fp = nullptr;
#endif
    std::shared_ptr<ringbuffer> m_ring = nullptr;
    std::shared_ptr<VideoState> is;
    std::shared_ptr<Demuxer> m_demuxer;
    std::shared_ptr<XDecodeVideo> m_vd;

};

class wsPlayer{
public:
    wsPlayer(){
        m_ring = std::shared_ptr<ringbuffer>(ringbuffer_malloc(1024 * 1024 * 4),[](ringbuffer* ring){ ringBuffer_free(ring);});
        if(!m_ring){
            WarnL<<"创建环形缓存失败";
        }
        m_demuxer = std::make_shared<Demuxer>();
        client = std::make_shared<WebSocketClient<WsSession> >();
    }
    ~wsPlayer(){
        m_ring = nullptr;
        m_demuxer = nullptr;
    }

    void start_l(std::string& url) {
        client->setRing(m_ring);
        client->startWebSocket(url);
        m_demuxer->setYuvCallback(std::bind(&wsPlayer::onDecode,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5,std::placeholders::_6,std::placeholders::_7,std::placeholders::_8));
        m_demuxer->doDemuxer(m_ring);

    }

    void onDecode(const char* y,int y_len,const char* u,int u_len,const char* v,int v_len,int width,int height){
        //WarnL<<"onDecode:"<<y_len<<" "<<width<<" "<<height;
    }
public:
    std::shared_ptr<ringbuffer> m_ring = nullptr;
    std::shared_ptr<VideoState> is = nullptr;
    std::shared_ptr<Demuxer> m_demuxer= nullptr;
    std::shared_ptr<XDecodeVideo> m_vd= nullptr;
    WebSocketClient<WsSession>::Ptr client = nullptr;


};

int main(int argc, char *argv[]) {


    //设置日志
    Logger::Instance().add(std::make_shared<ConsoleChannel>());
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

    std::string url  = "ws://192.168.3.72/live/haikang.live.flv";

    auto player = std::make_shared<wsPlayer>();
    player->start_l(url);

    SDLDisplayerHelper::Instance().runLoop();

    return 0;
}

