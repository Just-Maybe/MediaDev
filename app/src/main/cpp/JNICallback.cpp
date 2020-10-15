//
// Created by Miracle on 2020/10/14.
//

#include "JNICallback.h"

JNICallback::JNICallback(JavaVM *javaVm, JNIEnv *env, jobject instance) {
    this->javaVm = javaVm;
    this->env = env;
    this->instance = env->NewGlobalRef(instance);// 坑，需要是全局（jobject一旦涉及到跨函数，跨线程，必须是全局引用）

    jclass playerManager = env->GetObjectClass(instance);

    this->jmd_repared = env->GetMethodID(playerManager, "onPrepared", "()V");  //空参无返回值
    this->jmd_progress = env->GetMethodID(playerManager, "onProgress", "(I)V"); //Int 参数，无返回值
    this->jmd_error = env->GetMethodID(playerManager, "onError", "(I)V"); //Int 参数，无返回值
}

void JNICallback::onErrorAction(int thread_mode, int error_code) {
    if (thread_mode == THREAD_MAIN) {//主线程可以直接调用 Java 方法
        env->CallVoidMethod(this->instance, jmd_error, error_code);
    } else {
        //子线程，用附加 native 线程到 JVM 的方式，来获取到权限 env
        JNIEnv *jniEnv = nullptr;
        int ret = javaVm->AttachCurrentThread(&jniEnv, 0);
        if (ret != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(this->instance, jmd_error, error_code);
    }
}

void JNICallback::onPrepared(int thread_mode) {
    if (thread_mode == THREAD_MAIN) {
        env->CallVoidMethod(this->instance, this->jmd_repared);//主线程可以直接调用 Java 方法
    } else {
        //子线程，用附加 native 线程到 JVM 的方式，来获取到权限 env
        JNIEnv *jniEnv = nullptr;
        jint ret = javaVm->AttachCurrentThread(&jniEnv, 0);
        if (ret != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(this->instance, this->jmd_repared);
        javaVm->DetachCurrentThread();//解除附加
    }
}

void JNICallback::onProgress(int thread_mode, int progress) {
    if (thread_mode == THREAD_MAIN) {
        env->CallVoidMethod(this->instance, jmd_progress, progress);
    } else {
        JNIEnv *jniEnv = nullptr;
        int ret = javaVm->AttachCurrentThread(&jniEnv, 0);
        if (ret != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(this->instance, jmd_progress, progress);
    }
}

JNICallback::~JNICallback() {
    LOGD("~JNICallback");
    this->javaVm = 0;
    env->DeleteLocalRef(this->instance);
    this->instance = 0;
    env = 0;
}
