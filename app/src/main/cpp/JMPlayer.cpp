//
// Created by Miracle on 2020/10/14.
//

#include <jni.h>
#include "include/JMPlayer.h"

JMPlayer::JMPlayer(const char *data_source, JNICallback *callback) {
    // 这里有坑，这里赋值之后，不能给其他地方用，因为被释放了，变成了悬空指针
    // this->data_source = data_source;
    //解决上面的坑，自己 copy 才行 +1 在 C++ 中有一个 \n
    this->data_source = new char[strlen(data_source) + 1];
    strcpy(this->data_source, data_source);
    this->pCallback = callback;
    duration = 0;
    pthread_mutex_init(&seekMutex,0);
}


void JMPlayer::prepare_() {
    LOGD("第一步：打开流媒体地址");
    formatContext = avformat_alloc_context();

    AVDictionary *dictinonary = 0;
    av_dict_set(&dictinonary, "timeout", "5000000", 0);

    /**
     * @param AVFormatContext: 传入一个 format 上下文是一个二级指针
     * @param const char *url: 播放源
     * @param ff_const59 AVInputFormat *fmt: 输入的封住格式，一般让 ffmpeg 自己去检测，所以给了一个 0
     * @param AVDictionary **options: 字典参数
     */
    int result = avformat_open_input(&formatContext, data_source, 0, &dictinonary);

    LOGD("avformat_open_input--> %d,%s", result, data_source);
    av_dict_free(&dictinonary);

    if (result) {  //0 on success
        if (pCallback) {
            pCallback->onErrorAction(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);
        }
        return;
    }

}

