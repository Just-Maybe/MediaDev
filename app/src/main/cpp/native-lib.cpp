#include <jni.h>
#include <string>
#include "JNICallback.h"
#include "JMPlayer.h"
#include <android/native_window_jni.h>
#include <android/native_window.h>

extern "C" {
#include <libavutil/avutil.h>
}

JavaVM *javaVM = 0;
JMPlayer *player = 0;
ANativeWindow *nativeWindow = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;// 静态初始化 互斥锁

int JNI_OnLoad(JavaVM *javaVM1, void *pVoid) {
    javaVM = javaVM1;
    // 坑，这里记得一定要返回，和异步线程指针函数一样（记得返回）
    return JNI_VERSION_1_6;
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_example_mediadev_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    return env->NewStringUTF(av_version_info());
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_mediadev_player_PlayerManger_getFFmpegVersion(JNIEnv *env, jobject thiz) {
    // TODO: implement getFFmpegVersion()
    return env->NewStringUTF(av_version_info());

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediadev_player_PlayerManger_setSurfaceNative(JNIEnv *env, jobject thiz,
                                                               jobject surface) {
    // TODO: implement setSurfaceNative()
    pthread_mutex_lock(&mutex);
    if (nativeWindow) {
        ANativeWindow_release(nativeWindow);
        nativeWindow = 0;
    }
    //创建新的窗口用于视频显示
    nativeWindow = ANativeWindow_fromSurface(env, surface);

    pthread_mutex_unlock(&mutex);
}

/**
 *
 * 专门渲染的函数
 * @param src_data  解码后的视频 rgba 数据
 * @param width  视频宽
 * @param height 视频高
 * @param src_size 行数 size 相关信息
 *
 */
void renderFrame(uint8_t *src_data, int width, int height, int src_size) {
    pthread_mutex_lock(&mutex);

    if (!nativeWindow) {
        pthread_mutex_unlock(&mutex);
        nativeWindow = 0;
        return;;
    }

    //设置窗口属性
    ANativeWindow_setBuffersGeometry(nativeWindow, width, height, WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer window_buffer;

    //锁定创建 准备绘制
    if (ANativeWindow_lock(nativeWindow, &window_buffer, 0)) {
        ANativeWindow_release(nativeWindow);
        nativeWindow = 0;
        pthread_mutex_unlock(&mutex);
        return;
    }
    //填数据到 buffer 其实就是修改数据
    uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    int lineSize = window_buffer.stride * 4; //RGBA;

    // 逐行copy
    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst_data + i * lineSize, src_data + i * src_size, lineSize);
    }
    ANativeWindow_unlockAndPost(nativeWindow);
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediadev_player_PlayerManger_prepareNative(JNIEnv *env, jobject thiz,
                                                            jstring m_data_source) {
    // TODO: implement prepareNative()
    JNICallback *jniCallback = new JNICallback(javaVM, env, thiz);
    const char *data_source = env->GetStringUTFChars(m_data_source, NULL);
    player = new JMPlayer(data_source, jniCallback);
    player->setRenderCallback(renderFrame);
    player->prepare();
    env->ReleaseStringUTFChars(m_data_source, data_source);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediadev_player_PlayerManger_startNative(JNIEnv *env, jobject thiz) {
    // TODO: implement startNative()
    if (player) {
        player->start();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediadev_player_PlayerManger_restartNative(JNIEnv *env, jobject thiz) {
    // TODO: implement restartNative()
    if (player) {
        player->restart();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediadev_player_PlayerManger_stopNative(JNIEnv *env, jobject thiz) {
    // TODO: implement stopNative()
    if (player) {
        player->stop();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediadev_player_PlayerManger_releaseNative(JNIEnv *env, jobject thiz) {
    // TODO: implement releaseNative()
    if (player) {
        player->release();
    }
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_mediadev_player_PlayerManger_isPlayerNative(JNIEnv *env, jobject thiz) {
    // TODO: implement isPlayerNative()
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediadev_player_PlayerManger_seekNative(JNIEnv *env, jobject thiz, jint progress) {
    // TODO: implement seekNative()
    if (player) {
        player->seek(progress);
    }
}extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_mediadev_player_PlayerManger_getDurationNative(JNIEnv *env, jobject thiz) {
    // TODO: implement getDurationNative()
    if (player) {
        return player->getDuration();
    }
    return 0;
}