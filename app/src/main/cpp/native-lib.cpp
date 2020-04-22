#include <jni.h>
#include <string>
#include "MyPlayer.h"
#include "JNICallback.h"
#include <android/native_window_jni.h> // 是为了 渲染到屏幕支持的

extern "C" {
#include <libavutil/avutil.h>
}

JavaVM * javaVm = 0; // JNI_OnLoad 执行完毕后，此javaVm 就有值了
MyPlayer * player = 0;
ANativeWindow * nativeWindow = 0; // 为什么定义到上面 就是为了和 Surface 对象关联
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 静态初始化 互斥锁

// 动态注册（什么时候被调用的）
int JNI_OnLoad(JavaVM * javaVm, void * pVoid) {
    ::javaVm = javaVm;
    return JNI_VERSION_1_6; // 坑，这里记得一定要返回，和异步线程指针函数一样（记得返回）
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_wangxingxing_myffmpegplayer_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "FFmpeg version:";
    hello.append(av_version_info());
    return env->NewStringUTF(hello.c_str());
}

/**
 * 专门渲染的函数
 * @param src_data 解码后的视频 rgba 数据
 * @param width 宽信息
 * @param height 高信息
 * @param src_line_size 行数size相关信息
 */
void renderFrame(uint8_t * src_data, int width, int height, int src_line_size) {
    pthread_mutex_lock(&mutex);

    if (!nativeWindow) {
        pthread_mutex_unlock(&mutex);
    }

    // 设置窗口属性
    ANativeWindow_setBuffersGeometry(nativeWindow, width, height, WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer windowBuffer; // 缓冲区

    if (ANativeWindow_lock(nativeWindow, &windowBuffer, 0)) {
        ANativeWindow_release(nativeWindow);
        nativeWindow = 0;
        pthread_mutex_unlock(&mutex);
        return;
    }

    // 填数据到buffer，其实就是修改数据
    uint8_t * dst_data = static_cast<uint8_t *>(windowBuffer.bits);
    int lineSize = windowBuffer.stride * 4; // RGBA 相当于是 每一个像素点 * rgba

    // 下面就是逐行Copy了
    for (int i = 0; i < windowBuffer.height; ++i) {
        // 一行 一行 的 Copy 到 Android屏幕上
        memcpy(dst_data + i * lineSize, src_data + i * src_line_size, lineSize);
    }

    ANativeWindow_unlockAndPost(nativeWindow);
    pthread_mutex_unlock(&mutex);
}

/**
 * 准备工作
 * 此函数是被 MyPlayer调用的  thiz == MyPlayer实例
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_wangxingxing_myffmpegplayer_MyPlayer_prepareNative(JNIEnv *env, jobject thiz,
                                                            jstring data_source_) {
    // 准备工作的话，首先要来解封装，所以以面向对象的思想来考虑的话，新建一个类：MyPlayer
    JNICallback * jniCallback = new JNICallback(::javaVm, env, thiz);

    const char * data_source = env->GetStringUTFChars(data_source_, NULL);
    player = new MyPlayer(data_source, jniCallback);
    player->setRenderCallback(renderFrame); // 设置函数指针-- 的具体函数
    player->prepare(); // 准备

    // 释放操作
    env->ReleaseStringUTFChars(data_source_, data_source);
}

/**
 * 开始播放
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_wangxingxing_myffmpegplayer_MyPlayer_startNative(JNIEnv *env, jobject thiz) {
    if (player) {
        player->start();
    }
}

/**
 * 停止播放
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_wangxingxing_myffmpegplayer_MyPlayer_stopNative(JNIEnv *env, jobject thiz) {
    if (player) {
        player->stop();
    }
}

/**
 * 释放相关的工作
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_wangxingxing_myffmpegplayer_MyPlayer_releaseNative(JNIEnv *env, jobject thiz) {
    pthread_mutex_lock(&mutex);
    if (nativeWindow) {
        //把老的释放
        ANativeWindow_release(nativeWindow);
        nativeWindow = 0;
    }
    pthread_mutex_unlock(&mutex);
    DELETE(player); // 移除中转站
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wangxingxing_myffmpegplayer_MyPlayer_setSurfaceNative(JNIEnv *env, jobject thiz,
                                                               jobject surface) {
    pthread_mutex_lock(&mutex);

    if (nativeWindow) {
        ANativeWindow_release(nativeWindow);
        nativeWindow = 0;
    }

    // 创建新的窗口用于视频显示，关联起来
    nativeWindow = ANativeWindow_fromSurface(env, surface);

    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_wangxingxing_myffmpegplayer_MyPlayer_getFFmpegVersion(JNIEnv *env, jobject thiz) {
    return env->NewStringUTF(av_version_info());
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_wangxingxing_myffmpegplayer_MyPlayer_getDurationNative(JNIEnv *env, jobject thiz) {
    if (player) {
        return player->getDuration();
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wangxingxing_myffmpegplayer_MyPlayer_seekToNative(JNIEnv *env, jobject thiz,
                                                           jint progress) {
    if (player) {
        player->seekTo(progress);
    }
}