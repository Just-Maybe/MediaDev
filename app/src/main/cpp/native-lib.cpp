#include <jni.h>
#include <string>
#include "JNICallback.h"
#include "JMPlayer.h"
#include <android/native_window_jni.h>
extern "C" {
#include <libavutil/avutil.h>
}

JavaVM *javaVM = 0;
JMPlayer *player = 0;


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
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediadev_player_PlayerManger_stopNative(JNIEnv *env, jobject thiz) {
    // TODO: implement stopNative()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediadev_player_PlayerManger_releaseNative(JNIEnv *env, jobject thiz) {
    // TODO: implement releaseNative()
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_mediadev_player_PlayerManger_isPlayerNative(JNIEnv *env, jobject thiz) {
    // TODO: implement isPlayerNative()
}