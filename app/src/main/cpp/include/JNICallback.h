//
// Created by Miracle on 2020/10/14.
//

#ifndef MEDIADEV_JNICALLBACK_H
#define MEDIADEV_JNICALLBACK_H

#include <jni.h>
#include "Constants.h"

class JNICallback {
private:
    JavaVM *javaVm = 0;
    JNIEnv *env = 0;
    jobject instance;

    //反射获取java 函数
    jmethodID jmd_repared;
    jmethodID jmd_error;
    jmethodID jmd_progress;

public:
    JNICallback(JavaVM *javaVm, JNIEnv *env, jobject instance);

    //回调
    void onPrepared(int thread_mode);

    void onErrorAction(int thread_mode, int error_code);

    void onProgress(int thread, int progress);

    ~JNICallback();
};


#endif //MEDIADEV_JNICALLBACK_H
