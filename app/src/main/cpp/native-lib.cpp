#include <jni.h>
#include <string>
#include "MyPlayer.h"

extern "C" {
#include <libavutil/avutil.h>
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_wangxingxing_myffmpegplayer_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "FFmpeg version:";
    hello.append(av_version_info());
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wangxingxing_myffmpegplayer_MyPlayer_prepareNative(JNIEnv *env, jobject thiz,
                                                            jstring data_source) {
    // TODO: implement prepareNative()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wangxingxing_myffmpegplayer_MyPlayer_startNative(JNIEnv *env, jobject thiz) {
    // TODO: implement startNative()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wangxingxing_myffmpegplayer_MyPlayer_stopNative(JNIEnv *env, jobject thiz) {
    // TODO: implement stopNative()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wangxingxing_myffmpegplayer_MyPlayer_releaseNative(JNIEnv *env, jobject thiz) {
    // TODO: implement releaseNative()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wangxingxing_myffmpegplayer_MyPlayer_setSurfaceNative(JNIEnv *env, jobject thiz,
                                                               jobject surface) {
    // TODO: implement setSurfaceNative()
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_wangxingxing_myffmpegplayer_MyPlayer_getFFmpegVersion(JNIEnv *env, jobject thiz) {
    // TODO: implement getFFmpegVersion()
}