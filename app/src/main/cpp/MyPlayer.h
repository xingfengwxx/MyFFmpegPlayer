//
// Created by WangXingxing on 2020/1/17.
//

#ifndef MYFFMPEGPLAYER_MYPLAYER_H
#define MYFFMPEGPLAYER_MYPLAYER_H

#include <cstring>
#include <pthread.h>
#include <libavformat/avformat.h>

class MyPlayer {

public:
    MyPlayer();

    MyPlayer(const char *string);

    MyPlayer(char *dataSource);

    virtual ~MyPlayer();

    void prepare();

    void prepare_();

private:
    char * data_source = 0;
    pthread_t pid_prepare = 0;
    // 媒体的总上下文
    AVFormatContext * avFormatContext = 0;
};

#endif //MYFFMPEGPLAYER_MYPLAYER_H