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
    //写入头部信息
    ret = avformat_write_header(output_fmt_ctx, NULL);
    if (ret < 0) {
        LOGD("输出文件头部信息写入失败");
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
    //写入尾部信息
    av_write_trailer(output_fmt_ctx);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediadev_mooc_MoocActivity_cutVideo(JNIEnv *env, jobject thiz, jstring input_path,
                                                     jstring output_path, jdouble start_time,
                                                     jdouble end_time) {
    const char *input_file_path = env->GetStringUTFChars(input_path, 0);
    const char *output_file_path = env->GetStringUTFChars(output_path, 0);

    AVOutputFormat *ofmt = 0;
    AVFormatContext *ifmt_ctx = 0;
    AVFormatContext *ofmt_ctx = 0;
    AVPacket pkt;
    int ret, i;


    if ((ret = avformat_open_input(&ifmt_ctx, input_file_path, NULL, NULL)) < 0) {
        LOGD("打开文件失败");
        return;
    }
    ret = avformat_find_stream_info(ifmt_ctx, NULL);
    if (ret < 0) {
        LOGD("获取输入文件的数据流失败");
        return;
    }
    ret = avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, output_file_path);
    if (ret < 0) {
        LOGD("输出文件分配内存失败");
        return;
    }
    ofmt = ofmt_ctx->oformat;

    for (int i = 0; i < ifmt_ctx->nb_streams; ++i) {
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            LOGD("创建数据流失败");
            return;
        }
        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if (ret < 0) {
            LOGD("数据流参数copy 失败");
            return;
        }
        out_stream->codecpar->codec_tag = 0;
    }
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, output_file_path, AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOGD("创建IO流 上下文失败");
            return;
        }
    }

    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        LOGD("写入头部信息失败");
        return;
    }

    ret = av_seek_frame(ifmt_ctx, -1, start_time * AV_TIME_BASE, AVSEEK_FLAG_ANY);
    if (ret < 0) {
        LOGD("seek 失败");
        return;
    }
    int64_t *dts_start_from = static_cast<int64_t *>(malloc(
            sizeof(int64_t) * ifmt_ctx->nb_streams));

    memset(dts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);

    int64_t *pts_start_from = static_cast<int64_t *>(malloc(
            sizeof(int64_t) * ifmt_ctx->nb_streams));

    memset(pts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);

    while (1) {
        AVStream *in_stream;
        AVStream *out_stream;

        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0) {
            break;
        }
        in_stream = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];

        if (av_q2d(in_stream->time_base) * pkt.pts > end_time) {  //如果进度超过设置的截取时间则停止
            av_packet_unref(&pkt);
            break;
        }
        if (dts_start_from[pkt.stream_index] == 0) {
            dts_start_from[pkt.stream_index] = pkt.dts;
        }

        if (pts_start_from[pkt.stream_index] == 0) {
            pts_start_from[pkt.stream_index] = pkt.pts;
        }
        //时间基 转换
        pkt.pts = av_rescale_q_rnd(pkt.pts - pts_start_from[pkt.stream_index],
                                   in_stream->time_base, out_stream->time_base,
                                   static_cast<AVRounding>(AV_ROUND_NEAR_INF |
                                                           AV_ROUND_PASS_MINMAX));

        pkt.dts = av_rescale_q_rnd(pkt.dts - dts_start_from[pkt.stream_index],
                                   in_stream->time_base, out_stream->time_base,
                                   static_cast<AVRounding>(AV_ROUND_NEAR_INF |
                                                           AV_ROUND_PASS_MINMAX));

        if (pkt.pts < 0) {
            pkt.pts = 0;
        }
        if (pkt.dts < 0) {
            pkt.dts = 0;
        }
        pkt.duration = (int)av_rescale_q((int64_t)pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;

        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) {
            LOGD("数据写入失败");
            break;
        }
        av_packet_unref(&pkt);
    }
    free(dts_start_from);
    free(pts_start_from);
    av_write_trailer(ofmt_ctx);

}