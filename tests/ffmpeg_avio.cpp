//
// Created by 韦少 on 2023/6/21.
//

#include "avffmpeg.h"
#include "ffmpeg_avio.h"
#include "ringbuffer.h"
#include "Util/logger.h"
#include "YuvDisplayer.h"
using namespace toolkit;


struct buffer_data {
    uint8_t *ptr;
    uint8_t *ptr_start;
    size_t size; ///< buffer size
};

Demuxer::Demuxer()
{
    m_is = std::make_shared<VideoState>();
}

Demuxer::~Demuxer()
{
    avformat_close_input(&m_ic);
    // 对于自定义的AVIOContext，先释放里面的buffer，在释放AVIOContext对象
    if (m_ioctx) {
        av_freep(&m_ioctx->buffer);
    }
    avio_context_free(&m_ioctx);

}


static int io_read(void *opaque, uint8_t *buf, int buf_size)
{
    ringbuffer* ring = (ringbuffer*)opaque;
    //printf("io_read buf_size: %d  ringbuffer_len(ring):%d \n",buf_size,ringbuffer_len(ring));
//
    while(1){
        if(ringbuffer_len(ring) >= buf_size){
            break;
        }
        av_usleep(100);
    }
    ringbuffer_out(ring,buf,buf_size);

    return buf_size;
}

void Demuxer::doDemuxer(std::shared_ptr<ringbuffer> ring)
{
    AVFormatContext *m_ic = nullptr;
    int ret = 0;

    size_t m_m_ioctx_buffer_size = 4096;

    m_ic = avformat_alloc_context();
    if (m_ic == nullptr) {
        WarnL<<"avformat_alloc_context fail";
        return;
    }
    m_ioctx_buffer = (uint8_t*)av_mallocz(m_m_ioctx_buffer_size);
    m_ioctx = avio_alloc_context(m_ioctx_buffer,(int)m_m_ioctx_buffer_size,0,ring.get(),&io_read,nullptr,nullptr);
    if (m_ioctx == nullptr) {
        WarnL<<"avio_alloc_context fail";
        return;
    }
    m_ic->pb = m_ioctx;
    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "max_delay", "300", 0);
    av_dict_set(&opts, "probesize", "2048", 0);

    ret = avformat_open_input(&m_ic,nullptr,nullptr,&opts);
    if (ret < 0) {
        WarnL<<"avformat_open_input fail"<<av_err2str(ret);
        return;
    }

    ret = avformat_find_stream_info(m_ic,nullptr);
    if (ret < 0) {
        WarnL<<"avformat_find_stream_info fail"<<av_err2str(ret);
        return;
    }

    for (int i = 0;i<m_ic->nb_streams;i++) {
        AVStream *stream = m_ic->streams[i];
        enum AVCodecID cId = stream->codecpar->codec_id;
        int format = stream->codecpar->format;
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_is->videoStream = i;
            AVStream *as = m_ic->streams[m_is->videoStream];
            m_is->width = as->codecpar->width;
            m_is->height = as->codecpar->height;
            m_is->fps = av_q2d(as->avg_frame_rate);
            m_is->code_id_v = as->codecpar->codec_id;

            auto displayer = std::make_shared<YuvDisplayer>(nullptr);
            m_vd = std::make_shared<XDecodeVideo>();
            m_vd->Open(stream->codecpar, nullptr, 0);
//            auto fp_yuv = std::shared_ptr<FILE>(fopen("weixuan.yuv","wb+"),[](FILE* fp){ fclose(fp);});
//            if (!fp_yuv) {
//                printf("open yuv file error");
//                return;
//            }
            m_vd->setOnDecode([this,displayer](const AVFrame *rFrame) {
                //WarnL<<"got yuv";
                AVFrame* yuv = av_frame_alloc();
                av_frame_move_ref(yuv, (AVFrame*)rFrame);
                int y_size = m_is->width * m_is->height;
                m_cb(reinterpret_cast<const char *>(yuv->data[0]), y_size, reinterpret_cast<const char *>(yuv->data[1]), y_size / 4,
                     reinterpret_cast<const char *>(yuv->data[2]), y_size / 4, m_is->width, m_is->height);
                SDLDisplayerHelper::Instance().doTask([yuv, displayer]() mutable {
                    //sdl要求在main线程渲染
                    displayer->displayYUV(yuv);
                    av_frame_free(&yuv);
                    return true;
                });

            });
        } else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            m_is->audioStream = i;

        }
    }


    std::thread([m_ic,this] () mutable {
        int ret = 0;
        AVPacket pkt, *packet = &pkt;
        while (1) {
            ret = av_read_frame(m_ic, &pkt);
            if(ret < 0){
                WarnL<<"read packet error:"<<av_err2str(ret);
                return -1;
            }

            if (m_is->videoStream == packet->stream_index) {
                //printf("video size %d  pos %ld",packet->size,packet->pos);
                m_vd->decodeFrame(packet);

            } else if (m_is->audioStream == packet->stream_index){
                //printf("audio size %d  pos %ld pts(%s)",packet->size,packet->pos);
            }
        }

    }


            
    ).detach();



}
