//
// Created by Miracle on 2020/10/17.
//

#include "include/AudioChannel.h"

AudioChannel::AudioChannel(int stream_index, AVCodecContext *pContext, AVRational avRational,
                           JNICallback *jniCallback)
        : BaseChannel(stream_index, pContext, avRational, jniCallback) {
    //初始化缓冲区 out_buffers
    //动态计算
    // 或者写死 out_buffer_size = 44100 * 2 * 2

    //双通道
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    //采样大小
    out_sample_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    //采样率
    out_sample_rate = AUDIO_SAMPLE_RATE;
    //计算缓冲大小
    out_buffers_size = out_sample_rate * out_sample_size * out_channels;
    //分配缓冲内存
    out_buffers = static_cast<uint8_t *>(av_mallocz(out_buffers_size));
    //根据通道数、采样大小、采样率，返回分配的转换上下文SwrContext 指针
    swr_ctx = swr_alloc_set_opts(0, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, out_sample_rate,
                                 pContext->channel_layout, pContext->sample_fmt,
                                 pContext->sample_rate, 0, 0);
    //初始化上下文
    swr_init(swr_ctx);
    LOGD("AudioChannel 构造函数")
}

void AudioChannel::stop() {

}

void AudioChannel::start() {

}

void AudioChannel::audio_decode() {

}

void AudioChannel::audio_player() {

}

int AudioChannel::getPCM() {
    return 0;
}

void AudioChannel::release() {

}

void AudioChannel::restart() {

}

AudioChannel::~AudioChannel() {

}
