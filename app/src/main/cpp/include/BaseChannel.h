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
    AVRational base_time;
    double audio_time;
    double video_time;

    //AVPacket 音频：AAC，视频H264
    SafeQueue<AVPacket *> packages; //音视频的编码数据

    //AVFrame 音频：PCM，视频YUV
    SafeQueue<AVFrame *> frames;  //音视频未编码数据 (直接 渲染 和 播放)

    BaseChannel(int stream_index, AVCodecContext *pContext, AVRational av_base_time,
                JNICallback *jniCallback) {
        this->stream_index = stream_index;
        this->pContext = pContext;
        this->base_time = av_base_time;
        this->javaCallHelper = jniCallback;
        packages.setReleaseCallback(releaseAVPacket);
        frames.setReleaseCallback(releaseAVFrame);
    }

    /**
     * 释放AVPacket队列
     * @param avPacket
     */
    static void releaseAVPacket(AVPacket **avPacket) {
        if (avPacket) {
            av_packet_free(avPacket);
            *avPacket = 0;
        }
    }

    /**
     * 释放AVFrame 队列
     * @param avFrame
     */
    static void releaseAVFrame(AVFrame **avFrame) {
        if (avFrame) {
            av_frame_free(avFrame);
            *avFrame = 0;
        }
    }

    void clear() {
        packages.clearQueue();
        frames.clearQueue();
    }

    ~BaseChannel() {
        packages.clearQueue();
        frames.clearQueue();
    }
};