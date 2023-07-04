//
// Created by 韦少 on 2023/6/21.
//
#include "XDecode.h"
#include <sstream>
#include <iostream>

#ifdef USELIBYUV
extern "C" {
#include "libyuv.h"
}
#pragma comment(lib,"yuv.lib")
#endif // LIBYUV



#define MAX_DELAY_SECOND 3


//////////////////////////////////////////////////////////////////////////////////////////

FFmpegFrame::FFmpegFrame(std::shared_ptr<AVFrame> frame) {
    if (frame) {
        _frame = std::move(frame);
    }
    else {
        _frame.reset(av_frame_alloc(), [](AVFrame* ptr) {
            av_frame_free(&ptr);
        });
    }
}

FFmpegFrame::~FFmpegFrame() {
    if (_data) {
        delete[] _data;
        _data = nullptr;
    }
}

AVFrame* FFmpegFrame::get() const {
    return _frame.get();
}
//////////////////////////////////////////////////////////////////////////////////////////


static std::string ffmpeg_err(int errnum) {
    char errbuf[AV_ERROR_MAX_STRING_SIZE];
    av_strerror(errnum, errbuf, AV_ERROR_MAX_STRING_SIZE);
    return errbuf;
}

std::shared_ptr<AVPacket> alloc_av_packet() {
    auto pkt = std::shared_ptr<AVPacket>(av_packet_alloc(), [](AVPacket* pkt) {
        av_packet_free(&pkt);
    });
    pkt->data = NULL;    // packet data will be allocated by the encoder
    pkt->size = 0;
    return pkt;
}


//////////////////////////////////////////////////////////////////////////////////////////

template<bool decoder = true, typename ...ARGS>
AVCodec* getCodec(ARGS ...names);

template<bool decoder = true>
AVCodec* getCodec(const char* name) {
    auto codec = decoder ? avcodec_find_decoder_by_name(name) : avcodec_find_encoder_by_name(name);
    //if (codec) {
    //	//std::cout << (decoder ? "got decoder:" : "got encoder:") << name;
    //}
    return codec;
}

template<bool decoder = true>
AVCodec* getCodec(enum AVCodecID id) {
    auto codec = decoder ? avcodec_find_decoder(id) : avcodec_find_encoder(id);
    //if (codec) {
    //	//std::cout << (decoder ? "got decoder:" : "got encoder:") << avcodec_get_name(id);
    //}
    return codec;
}

template<bool decoder = true, typename First, typename ...ARGS>
AVCodec* getCodec(First first, ARGS ...names) {
    auto codec = getCodec<decoder>(names...);
    if (codec) {
        return codec;
    }
    return getCodec<decoder>(first);
}



#ifdef D3D_ON
AVPixelFormat GetHwFormat(AVCodecContext* s, const AVPixelFormat* pix_fmts)
{
	InputStream* ist = (InputStream*)s->opaque;
	ist->active_hwaccel_id = HWACCEL_DXVA2;
	ist->hwaccel_pix_fmt = AV_PIX_FMT_DXVA2_VLD;
	return ist->hwaccel_pix_fmt;
}

#endif // D3D_ON




//static AVBufferRef* hw_device_ctx = NULL;
//static enum AVPixelFormat hw_pix_fmt;



static int hw_decoder_init(AVCodecContext* ctx, const enum AVHWDeviceType type)
{
    int err = 0;
    static AVBufferRef* hw_device_ctx = NULL;
    if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type, NULL, NULL, 0)) < 0) {
        //fprintf(stderr, "Failed to create specified HW device.\n");
        return err;
    }
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);

    return err;
}


XDecodeVideo::XDecodeVideo() {

    m_pFrame = av_frame_alloc();
}


XDecodeVideo::~XDecodeVideo() {
    ////av_log(NULL, //av_log_WARNING, "~XDecodeVideo()\n");
    if (m_pFrame) {
        av_frame_free(&m_pFrame);
        m_pFrame = nullptr;
    }
    if (m_YUVPic) {
        av_frame_free(&m_YUVPic);
        m_YUVPic = nullptr;
    }
    if (m_encodeContext) {
        avcodec_close(m_encodeContext);
        avcodec_free_context(&m_encodeContext);
        m_encodeContext = nullptr;
    }
}


//句柄由于是从上层发的,此函数需要在XPlay::waitHwnd之后调用
int XDecodeVideo::InitRenderWindows(const char* videoHwnd) {
    if (videoHwnd == nullptr) {
        //av_log(NULL, //av_log_ERROR, "videoHwnd == nullptr\n");
        return -1;
    }
    m_videoH = (void*)videoHwnd;
    return 0;
}


int XDecodeVideo::getSnap(std::string filepath) {

    m_getPict = OpenOutput(filepath) == 0 ? true : false;
    return m_getPict;
}

int  XDecodeVideo::OpenOutput(std::string outUrl)
{

    int ret = avformat_alloc_output_context2(&m_outputContext, nullptr, "singlejpeg", outUrl.c_str());
    if (ret < 0)
    {
        //av_log(NULL, //av_log_ERROR, "open output context failed\n");
        goto Error;
    }

    ret = avio_open2(&m_outputContext->pb, outUrl.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr);
    if (ret < 0)
    {
        //av_log(NULL, //av_log_ERROR, "open avio failed");
        goto Error;
    }

    for (int i = 0; i < m_inputContext->nb_streams; i++)
    {
        if (m_inputContext->streams[i]->codec->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO)
        {
            AVStream* stream = avformat_new_stream(m_outputContext, m_inputContext->streams[i]->codec->codec);
            ret = avcodec_copy_context(stream->codec, m_inputContext->streams[i]->codec);
            if (ret < 0)
            {
                //av_log(NULL, //av_log_ERROR, "copy coddec context failed");
                goto Error;
            }
        }
    }

    ret = avformat_write_header(m_outputContext, nullptr);
    if (ret < 0)
    {
        //av_log(NULL, //av_log_ERROR, "format write header failed");
        goto Error;
    }

    //av_log(NULL, //av_log_FATAL, " Open output file success %s\n", outUrl.c_str());
    return ret;
    Error:
    CloseOutput();
    return ret;
}

void XDecodeVideo::CloseOutput()
{
    if (m_outputContext != nullptr)
    {
        int ret = av_write_trailer(m_outputContext);
        for (int i = 0; i < m_outputContext->nb_streams; i++)
        {
            AVCodecContext* codecContext = m_outputContext->streams[i]->codec;
            avcodec_close(codecContext);
        }
        avformat_close_input(&m_outputContext);
    }
}


int XDecodeVideo::initEncoderCodec(AVStream* inputStream, AVCodecContext** encodeContext)
{
    AVCodec* picCodec;

    picCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    (*encodeContext) = avcodec_alloc_context3(picCodec);

    (*encodeContext)->codec_id = picCodec->id;
    (*encodeContext)->time_base.num = inputStream->codec->time_base.num;
    (*encodeContext)->time_base.den = inputStream->codec->time_base.den;
    (*encodeContext)->pix_fmt = *picCodec->pix_fmts;
    (*encodeContext)->width = inputStream->codec->width;
    (*encodeContext)->height = inputStream->codec->height;
    int ret = avcodec_open2((*encodeContext), picCodec, nullptr);
    if (ret < 0)
    {
        //std::cout << "open video codec failed， snapshot disable" << endl;
        return  ret;
    }
    return 1;
}

//编码完毕之后 自动结束文件
void  XDecodeVideo::EncodePic(AVCodecContext* encodeContext, AVFrame* frame)
{
    std::shared_ptr<AVPacket> pkt(static_cast<AVPacket*>(av_malloc(sizeof(AVPacket))), [&](AVPacket* p) { av_packet_free(&p); av_freep(&p); });
    av_init_packet(pkt.get());
    pkt->data = NULL;
    pkt->size = 0;
    int ret;

    ret = avcodec_send_frame(encodeContext, frame);
    if (ret < 0) {
        //std::cout << "avcodec_send_frame failed:" << ffmpeg_err(ret);
        return;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(encodeContext, pkt.get());
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            //std::cout << "avcodec_receive_packet failed:" << ffmpeg_err(ret);
            break;
        }
        ret = WritePacket(pkt);
        if (ret >= 0)
        {
            //av_log(NULL, //av_log_ERROR, "get snap success\n");
        }
        else {
            //av_log(NULL, //av_log_ERROR, "get snap failed\n");
        }

    }

    CloseOutput();
}

int  XDecodeVideo::WritePacket(std::shared_ptr<AVPacket> packet)
{
    auto inputStream = m_inputContext->streams[packet->stream_index];
    auto outputStream = m_outputContext->streams[packet->stream_index];
    return av_interleaved_write_frame(m_outputContext, packet.get());
}
// 0 video 1 audio
bool XDecodeVideo::Open(AVCodecParameters* para, AVFormatContext* inputContext, int media_type) {

    m_inputContext = inputContext;
    AVCodec* vcodec = nullptr;
    AVCodec* codec_default = nullptr;
    switch (para->codec_id) {
        case AV_CODEC_ID_VP8:
            codec_default = getCodec(AV_CODEC_ID_VP8);
            vcodec = getCodec( "libvpx-vp8", "vp8_cuvid", AV_CODEC_ID_VP8);
            break;
        case AV_CODEC_ID_VP9:
            codec_default = getCodec(AV_CODEC_ID_VP9);
            vcodec = getCodec( "libvpx-vp9", "vp9_cuvid", AV_CODEC_ID_VP9);
            break;
        case AV_CODEC_ID_H264:
            codec_default = getCodec(AV_CODEC_ID_H264);
            vcodec = getCodec("libopenh264", "h264_videotoolbox", "h264_cuvid", AV_CODEC_ID_H264);
            break;
        case AV_CODEC_ID_H265:
            codec_default = getCodec(AV_CODEC_ID_HEVC);
            vcodec = getCodec("hevc_videotoolbox", "hevc_cuvid", AV_CODEC_ID_HEVC);
            break;
        case AV_CODEC_ID_PCM_ALAW:
            codec_default = getCodec(AV_CODEC_ID_PCM_ALAW);
            vcodec = getCodec({AV_CODEC_ID_PCM_ALAW});
            break;
        case AV_CODEC_ID_PCM_MULAW:
            codec_default = getCodec(AV_CODEC_ID_PCM_MULAW);
            vcodec = getCodec({AV_CODEC_ID_PCM_MULAW});
            break;
        case AV_CODEC_ID_OPUS:
            codec_default = getCodec(AV_CODEC_ID_OPUS);
            vcodec = getCodec({AV_CODEC_ID_OPUS});
            break;
        default: break;
    }

    if (!vcodec) {
        throw std::runtime_error("未找到解码器");
    }

    while (true) {
        _context.reset(avcodec_alloc_context3(vcodec), [](AVCodecContext* ctx) {
            avcodec_close(ctx);
            avcodec_free_context(&ctx);
        });

        if (!_context) {
            throw std::runtime_error("create codec failed");
        }


        avcodec_parameters_to_context(_context.get(), para);

        if(media_type == 0) {
            _context->coded_width = _context->width;
            _context->coded_height = _context->height;

            //保存AVFrame的引用
            _context->refcounted_frames = 1;
            _context->flags |= AV_CODEC_FLAG_LOW_DELAY;
            _context->flags2 |= AV_CODEC_FLAG2_FAST;

            AVDictionary *dict = nullptr;
            av_dict_set(&dict, "threads", "auto", 0);
            av_dict_set(&dict, "zerolatency", "1", 0);
            av_dict_set(&dict, "strict", "-2", 0);

            if (vcodec->capabilities & AV_CODEC_CAP_TRUNCATED) {
                /* we do not send complete frames */
                _context->flags |= AV_CODEC_FLAG_TRUNCATED;
            }
        }else{
            _context->channels = para->channels;
            _context->sample_rate = para->sample_rate;
            _context->channel_layout = av_get_default_channel_layout(_context->channels);
        }

        int ret = avcodec_open2(_context.get(), vcodec, nullptr);
        //av_dict_free(&dict);
        if (ret >= 0) {
            //成功
            ////std::cout << "打开视频解码器成功:" << vcodec->name << std::endl;
            break;
        }

        if (codec_default && codec_default != vcodec) {
            //硬件编解码器打开失败，尝试软件的
            //std::cout << "打开视频解码器" << vcodec->name << "失败，原因是:" << ffmpeg_err(ret) << ", 再尝试打开视频解码器" << codec_default->name;
            vcodec = codec_default;
            continue;
        }
        std::cout<<"open codec failed: " << ffmpeg_err(ret)<<std::endl;
        throw std::runtime_error("open codec failed");
    }
    return true;
}


void XDecodeVideo::setOnDecode(std::function<void(const AVFrame*)> cb) {
    _cb = std::move(cb);
}

void XDecodeVideo::onDecode(const AVFrame* frame) {
    ////std::cout << "onDecode" << std::endl;
    if (_cb) {
        _cb(frame);
    }
}



void XDecodeVideo::onGetSnap(const AVFrame* pFrame)
{

    //SaveFrame((AVFrame*)pFrame, _context->width, _context->height,0);
    if (pFrame->data[2] == nullptr) //说明是nv12格式的 需要转化为yuv
    {
        int ylen_dst = _context->width;
        int ulen_dst = _context->width / 2;
        int vlen_dst = _context->width / 2;

        if (m_YUVPic == nullptr) {
            m_YUVPic = av_frame_alloc();
            m_YUVPic->width = _context->width;
            m_YUVPic->height = _context->height;
            m_YUVPic->format = (int)AV_PIX_FMT_YUV420P;
            av_frame_get_buffer(m_YUVPic, 0);//手动创建avframe对象的内存
        }

        av_frame_copy_props(m_YUVPic, pFrame);
#ifdef USELIBYUV
        libyuv::NV12ToI420(
			pFrame->data[0], pFrame->linesize[0],
			pFrame->data[1], pFrame->linesize[1],
			m_YUVPic->data[0], ylen_dst,
			m_YUVPic->data[1], ulen_dst,
			m_YUVPic->data[2], vlen_dst,
			_context->width, _context->height);
#endif


        EncodePic(m_encodeContext, m_YUVPic);
    }
    else {
        EncodePic(m_encodeContext, (AVFrame*)pFrame);
    }

}


void XDecodeVideo::decodeFrame(const AVPacket* avpkt) {
    auto pkt = alloc_av_packet();
    pkt->data = (uint8_t*)avpkt->data;
    pkt->size = avpkt->size;
    pkt->dts = avpkt->dts;
    pkt->pts = avpkt->pts;

    auto ret = avcodec_send_packet(_context.get(), pkt.get());
    if (ret < 0) {
        if (ret != AVERROR_INVALIDDATA) {
            std::cout << "[video]avcodec_send_packet failed:" << ffmpeg_err(ret)<<std::endl;
        }
        return;
    }

    while (true) {

        ret = avcodec_receive_frame(_context.get(), m_pFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }
        if (ret < 0) {
            std::cout << "[video]avcodec_receive_frame failed:" << ffmpeg_err(ret)<<std::endl;
            break;
        }

        ////软解状态下正常，硬解截图有问题
        if (m_getPict)
        {
            m_getPict = false;
            onGetSnap(m_pFrame);
        }

        onDecode(m_pFrame);


    }
}
