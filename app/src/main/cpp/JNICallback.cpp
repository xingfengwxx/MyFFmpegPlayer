//
// Created by WangXingxing on 2020/4/15.
//

#include "JNICallback.h"

/**
 * 构造函数
 * @param javaVm  javaVm 能够跨越线程，还能够附加一个线程（创建出全新的env）
 * @param env     env 不能跨越线程（不能在子线程使用） ，能够在主线程使用
 * @param instance 可以理解是Java上层MainActivity的实例对象
 */
JNICallback::JNICallback(JavaVM *javaVm, JNIEnv *env, jobject instance) {
    this->javaVm = javaVm;
    this->env = env;
    this->instance = env->NewGlobalRef(instance); // 坑，需要是全局（jobject一旦涉及到跨函数，跨线程，必须是全局引用）

    // 调用 上层 成功的函数 1
    jclass  mMyPlayerClass = env->GetObjectClass(this->instance);
    const char * sig = "()V";
    jmd_prepared = env->GetMethodID(mMyPlayerClass, "onPrepared", sig);
    // env->CallVoidMethod(instance, onPrepared); 这里就可以调用 Java层的方法

    // 调用 上层 失败的函数 2
    sig = "(I)V";
    jmd_error = env->GetMethodID(mMyPlayerClass, "onError", sig);

    // 调用 上层 播放进度回调的函数 3
    sig = "(I)V";
    jmd_progress = env->GetMethodID(mMyPlayerClass, "onProgress", sig);
}

/**
 * 析构函数：专门完成释放的工作
 */
JNICallback::~JNICallback() {
    this->javaVm = 0;
    env->DeleteGlobalRef(this->instance);
    this->instance = 0;
    env = 0;
}

void JNICallback::onPrepared(int thread_mode) {
    if (thread_mode == THREAD_MAIN) {
        // 主线程
        env->CallVoidMethod(this->instance, jmd_prepared); // 这里就可以调用 Java层的方法
    } else {
        /**
         * 子线程
         * 如果是子线程的情况下，env 就会失效，报错
         *
         * 用附加native线程到JVM的方式，来获得权限的env
         * 是子线程的env
         */
        JNIEnv * jniEnv = nullptr; // 全新的env
        jint ret = javaVm->AttachCurrentThread(/*reinterpret_cast<JNIEnv **>(jniEnv)*/ &jniEnv, 0);
        if (ret != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(this->instance, jmd_prepared); // 调用Java层方法
        javaVm->DetachCurrentThread(); // 解除附加，必须的
    }
}

void JNICallback::onErrorAction(int thread_mode, int error_code) {
    if (thread_mode == THREAD_MAIN) {
        // 主线程
        env->CallVoidMethod(this->instance, jmd_error); // 调用Java层方法
    } else {
        /**
        * 子线程
        * 如果是子线程的情况下，env 就会失效，报错
        *
        * 用附加native线程到JVM的方式，来获得权限的env
        * 是子线程的env
        */
        JNIEnv * jniEnv = nullptr;
        jint ret = javaVm->AttachCurrentThread(/*reinterpret_cast<JNIEnv **>(jniEnv)*/ &jniEnv, 0);
        if (ret != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(this->instance, jmd_error, error_code); // 调用Java层方法
        javaVm->DetachCurrentThread(); // 解除附加，必须的
    }
}

void JNICallback::onProgress(int thread_mode, int progress) {
    if (thread_mode == THREAD_MAIN) {
        //主线程
        env->CallVoidMethod(instance, jmd_progress, progress);
    } else {
        //子线程
        //当前子线程的 JNIEnv
        JNIEnv * env_child;
        javaVm->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(instance, jmd_progress, progress);
        javaVm->DetachCurrentThread();
    }
}
