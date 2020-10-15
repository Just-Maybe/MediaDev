//
// Created by Miracle on 2020/10/14.
//

#ifndef MEDIADEV_JM_PLAYER_H
#define MEDIADEV_JM_PLAYER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}
#include <pthread.h>
#include "JNICallback.h"

class JMPlayer {
private:
    char *data_source = 0;

    pthread_t pid_thread;

    AVFormatContext *formatContext;

    JNICallback *pCallback = 0;

    pthread_mutex_t seekMutex;

    long duration = 0;
public:
    JMPlayer();

    JMPlayer(const char *data_source, JNICallback *callback);


    void prepare_();
};


#endif //MEDIADEV_JM_PLAYER_H
