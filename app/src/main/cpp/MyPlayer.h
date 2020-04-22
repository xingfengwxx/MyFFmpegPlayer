//
// Created by WangXingxing on 2020/1/17.
//
#include <cstring>
#include "JNICallback.h"
#include "AudioChannel.h"
#include "VideoChannel.h"
#include "macro.h"
#include <pthread.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

#ifndef MYFFMPEGPLAYER_MYPLAYER_H
#define MYFFMPEGPLAYER_MYPLAYER_H

class MyPlayer {
//    friend void *taskStop(void *args);

public:
    MyPlayer();

//    MyPlayer(const char *string);

    MyPlayer(const char *data_source, JNICallback *pCallback);

    virtual ~MyPlayer();

    void prepare();

    void prepare_();

    void start();

    void start_();

    void stop();

    void stop_();

    void setRenderCallback(RenderCallback renderCallback);

    void setDuration(int duration);

    int getDuration() const;

    void seekTo(int progress);

private:
    char * data_source = 0;
    pthread_t pid_prepare = 0;
    // 媒体的总上下文
    AVFormatContext * formatContext = 0;

    AudioChannel *audioChannel = 0;
    VideoChannel *videoChannel = 0;
    JNICallback *pCallback = 0;
    pthread_t  pid_start;
    pthread_t  pid_stop;
    bool isPlaying;
    int duration; //总时长
    pthread_mutex_t seekMutex;

    // native-lib.cpp prepareNative函数执行的时候，会把"具体函数"传递到此处
    RenderCallback renderCallback;
};

#endif //MYFFMPEGPLAYER_MYPLAYER_H