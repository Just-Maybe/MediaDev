//
// Created by Miracle on 2020/10/17.
//

#ifndef MEDIADEV_AUDIOCHANNEL_H
#define MEDIADEV_AUDIOCHANNEL_H

#include "BaseChannel.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>
};

#define AUDIO_SAMPLE_RATE 44100

class AudioChannel : public BaseChannel {
public:
    uint8_t *out_buffers = 0;

    int out_channels;
    int out_sample_size;
    int out_sample_rate;
    int out_buffers_size;

    AudioChannel(int stream_index, AVCodecContext *pContext, AVRational, JNICallback *jniCallback);

    void stop();

    void start();

    void audio_decode();

    void audio_player();

    int getPCM();

    void release();

    void restart();

    ~AudioChannel();

private:
    //线程ID
    pthread_t pid_audio_decode;
    pthread_t pid_audio_player;

    //引擎
    SLObjectItf engineObject;
    //引擎接口
    SLEngineItf engineInterface;
    //混音器
    SLObjectItf outputMixObject;
    //播放器的
    SLObjectItf bqPlayerObject;
    //播放器接口
    SLPlayItf bqPlayerPlay;
    //获取播放器队列接口
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;

    SwrContext *swr_ctx;
};


#endif //MEDIADEV_AUDIOCHANNEL_H
