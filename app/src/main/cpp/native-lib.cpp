#include <jni.h>
#include <string>

extern "C" {
#include <libavutil/avutil.h>
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_mediadev_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    return env->NewStringUTF(av_version_info());
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_mediadev_MainActivity_getFFmpegVersion(JNIEnv *env, jobject thiz) {
    // TODO: implement getFFmpegVersion()
    return env->NewStringUTF(av_version_info());

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediadev_MainActivity_setSurfaceNative(JNIEnv *env, jobject thiz,
                                                        jobject surface) {
    // TODO: implement setSurfaceNative()

}extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediadev_MainActivity_prepareNative(JNIEnv *env, jobject thiz,
                                                     jstring m_data_source) {
    // TODO: implement prepareNative()
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediadev_MainActivity_startNative(JNIEnv *env, jobject thiz) {
    // TODO: implement startNative()
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediadev_MainActivity_restartNative(JNIEnv *env, jobject thiz) {
    // TODO: implement restartNative()
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediadev_MainActivity_stopNative(JNIEnv *env, jobject thiz) {
    // TODO: implement stopNative()
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediadev_MainActivity_releaseNative(JNIEnv *env, jobject thiz) {
    // TODO: implement releaseNative()
}extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_mediadev_MainActivity_isPlayerNative(JNIEnv *env, jobject thiz) {
    // TODO: implement isPlayerNative()
}