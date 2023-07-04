
extern "C" {

#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"
#include "libavutil/parseutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/avassert.h"
#include "libavutil/time.h"
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
#include "libavutil/opt.h"
#include "libavutil/avutil.h"
#include "libavutil/eval.h"
#include "libavutil/fifo.h"
#include "libavutil/bprint.h"
#include "libavutil/file.h"

#include "libavformat/avformat.h"
//#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"

#include "libavcodec/codec_id.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"



#include "libavcodec/avcodec.h"
#include "libavcodec/avfft.h"
#include "libswresample/swresample.h"


#include <libavfilter/avfilter.h>
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"

}

// fix temporary array error in c++1x
#ifdef av_err2str
#undef av_err2str
av_always_inline char* av_err2str(int errnum)
{
    char str[AV_ERROR_MAX_STRING_SIZE];
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}
#endif
