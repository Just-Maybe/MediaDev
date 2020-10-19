//
// Created by Miracle on 2020/10/17.
//

#ifndef MEDIADEV_AUDIOCHANNEL_H
#define MEDIADEV_AUDIOCHANNEL_H

#include "BaseChannel.h"

class AudioChannel : public BaseChannel {
public:
    AudioChannel(int stream_index, AVCodecContext *pContext, AVRational, JNICallback *jniCallback);
};


#endif //MEDIADEV_AUDIOCHANNEL_H
