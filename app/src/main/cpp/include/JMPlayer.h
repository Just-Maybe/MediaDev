//
// Created by Miracle on 2020/10/14.
//

#ifndef MEDIADEV_JM_PLAYER_H
#define MEDIADEV_JM_PLAYER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
}

#include <pthread.h>
#include "JNICallback.h"
#include "AudioChannel.h"
#include "VideoChannel.h"

class JMPlayer {
private:
    char *data_source = 0;

    pthread_t pid_thread;

    AVFormatContext *formatContext;

    AVCodecContext *codecContext;

    JNICallback *pCallback = 0;

    pthread_mutex_t seekMutex;

    long duration = 0;

    AudioChannel *audioChannel = 0;

    VideoChannel *videoChannel = 0;

    RenderCallback renderCallback;

public:

    bool isPlaying;

    bool isStop= false;

    JMPlayer();

    JMPlayer(const char *data_source, JNICallback *callback);

    void prepare();

    void prepare_();

    void start();

    void start_();

    void setRenderCallback(RenderCallback renderCallback);

    long getDuration() {
        return duration;
    }

    void stop();

    void release();

    void restart();
};


#endif //MEDIADEV_JM_PLAYER_H
