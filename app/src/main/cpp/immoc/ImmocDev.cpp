//
// Created by Miracle on 2020/10/19.
//

#include <jni.h>

extern "C" {
#include <libavutil/log.h>
#include <libavformat/avformat.h>
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_mediadev_mooc_MoocActivity_getVideoInfo(JNIEnv *env, jobject thiz,
                                                         jstring input_path) {
    // TODO: implement getVideoInfo()
    int ret;
    AVFormatContext *fmt_ctx = NULL;

    av_log_set_level(AV_LOG_INFO);

    av_register_all();

    const char *input_str = env->GetStringUTFChars(input_path, NULL);

    ret = avformat_open_input(&fmt_ctx, input_str, NULL, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Can't open file:");
        return env->NewStringUTF("Failed");
    }
    av_dump_format(fmt_ctx, 0, input_str, 0);

    avformat_close_input(&fmt_ctx);

    return env->NewStringUTF("Success");
}