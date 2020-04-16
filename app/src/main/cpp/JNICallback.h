//
// Created by WangXingxing on 2020/4/15.
//

#ifndef MYFFMPEGPLAYER_JNICALLBACK_H
#define MYFFMPEGPLAYER_JNICALLBACK_H

#include <jni.h>
#include "macro.h"

class JNICallback {
public:
    // 全部都是，只有声明，没有实现
    JNICallback(JavaVM *javaVm, JNIEnv *env, jobject instance);
    ~JNICallback();

    void onPrepared(int thread_mode);
    void onErrorAction(int thread_mode, int error_code);

private:
    // 成员变量，规范写法，如果是指针 就初始化 = 0
    JavaVM *javaVm = 0;
    JNIEnv *env = 0;
    jobject instance;

    // 上层 成功函数标记
    jmethodID jmd_prepared;
    // 上层 失败函数标记
    jmethodID jmd_error;
};


#endif //MYFFMPEGPLAYER_JNICALLBACK_H