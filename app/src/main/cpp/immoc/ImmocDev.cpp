//
// Created by Miracle on 2020/10/19.
//

#include <jni.h>
#include "macro.h"

extern "C" {
#include <libavutil/log.h>
#include <libavformat/avformat.h>
#include <libavcodec/codec.h>
#include <libavutil/mathematics.h>
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediadev_mooc_MoocActivity_mp4ToFlv(JNIEnv *env, jobject thiz,
                                                     jstring input_path, jstring output_path) {
    // TODO: implement getVideoInfo()
    AVOutputFormat *out_fmt = 0;
    AVFormatContext *input_fmt_ctx = 0;
    AVFormatContext *output_fmt_ctx = 0;
    AVPacket pkt;
    int ret, i;
    int stream_index = 0;
    int *stream_mapping = 0;
    int stream_mapping_size = 0;
    const char *input_file_name = env->GetStringUTFChars(input_path, NULL);
    const char *output_file_name = env->GetStringUTFChars(output_path, NULL);

    if (avformat_open_input(&input_fmt_ctx, input_file_name, 0, NULL) < 0) {
        LOGD("打开输入文件失败");
        return;
    };
    if (avformat_find_stream_info(input_fmt_ctx, NULL) < 0) {
        LOGD("找不到流信息");
        return;
    }
    avformat_alloc_output_context2(&output_fmt_ctx, NULL, NULL, output_file_name); //打开输出文件
    if (!output_fmt_ctx) {
        LOGD("打开输出文件失败");
        return;
    }
    stream_mapping_size = input_fmt_ctx->nb_streams;
    stream_mapping = static_cast<int *>(av_mallocz_array(stream_mapping_size,
                                                         sizeof(*stream_mapping)));
    if (!stream_mapping) {
        LOGD("数据流分配内存失败");
        return;
    }
    out_fmt = output_fmt_ctx->oformat;
    for (int i = 0; i < stream_mapping_size; ++i) {
        AVStream *out_stream;
        AVStream *in_stream = input_fmt_ctx->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;
        if (in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            continue;
        }
        stream_mapping[i] = stream_index++;

        out_stream = avformat_new_stream(output_fmt_ctx, NULL);
        if (!out_stream) {
            LOGD("创建输出数据流失败");
            return;
        }
        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if (ret < 0) {
            LOGD("输出流编解码器复制失败");
            return;
        }
        out_stream->codecpar->codec_tag = 0;
    }
    if (!(output_fmt_ctx->flags & AVFMT_NOFILE)) {
        ret = avio_open(&output_fmt_ctx->pb, output_file_name, AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOGD("avio_open 输出文件打开失败");
            return;
        }
    }
    ret = avformat_write_header(output_fmt_ctx, NULL);
    if (ret < 0) {
        LOGD("输出流头部信息写入失败");
        return;
    }
    while (1) {
        AVStream *in_stream;
        AVStream *out_stream;
        ret = av_read_frame(input_fmt_ctx, &pkt);
        if (ret < 0) {
            break;
        }
        in_stream = input_fmt_ctx->streams[pkt.stream_index];
        // 过滤无效的数据流
        if (pkt.stream_index >= stream_mapping_size || stream_mapping[pkt.stream_index] < 0) {
            av_packet_unref(&pkt);
            continue;
        }
        pkt.stream_index = stream_mapping[pkt.stream_index];
        out_stream = output_fmt_ctx->streams[pkt.stream_index];

        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                                   static_cast<AVRounding>(AV_ROUND_NEAR_INF |
                                                           AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                                   static_cast<AVRounding>(AV_ROUND_NEAR_INF |
                                                           AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);

        pkt.pos = -1;
        //将编码数据写入到输出文件
        ret = av_interleaved_write_frame(output_fmt_ctx, &pkt);
        if (ret < 0) {
            LOGD("错误数据");
            break;
        }
        av_packet_unref(&pkt);
    }
    av_write_trailer(output_fmt_ctx);
}