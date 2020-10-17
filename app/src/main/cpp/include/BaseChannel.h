//
// Created by Miracle on 2020/10/17.
//

#ifndef MEDIADEV_BASECHANNEL_H
#define MEDIADEV_BASECHANNEL_H

#endif //MEDIADEV_BASECHANNEL_H
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
};

#include "JNICallback.h"
#include "safe_queue.h"
#include "Constants.h"

class BaseChannel {
public:
    int stream_index;

    bool isPlaying = 1;
    bool isStop = false;

    AVCodecContext *pContext;
    JNICallback *javaCallHelper;

    // 音视频同步需要用到
    AVRational *base_time;
    double audio_time;
    double video_time;


    BaseChannel(int stream_index, AVCodecContext *pContext, AVRational *av_base_time,
                JNICallback *jniCallback) {
        this->stream_index = stream_index;
        this->pContext = pContext;
        this->base_time = av_base_time;

    }
};