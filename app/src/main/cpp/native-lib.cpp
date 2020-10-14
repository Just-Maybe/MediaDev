#include <jni.h>
#include <string>

extern "C"{
#include <libavutil/avutil.h>
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_mediadev_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    const char* version = av_version_info();
    return env->NewStringUTF(version);
}
