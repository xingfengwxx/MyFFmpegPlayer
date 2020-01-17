//
// Created by WangXingxing on 2020/1/17.
//

#include "MyPlayer.h"

// 函数指针
// void* (*__start_routine)(void*)
void * customTaskPrepareThread(void * pVoid) {
    MyPlayer * myPlayer = static_cast<MyPlayer *>(pVoid);
    myPlayer->prepare_();
    return 0; // 必须返回，有坑
}

MyPlayer::MyPlayer() {}

MyPlayer::~MyPlayer() {
    if (this->data_source) {
        delete this->data_source;
        this->data_source = 0;
    }
}

MyPlayer::MyPlayer(const char *dataSource) {
    // 有坑， 会报错
    // this->data_source = data_source;

    // 有坑，长度不对   C:aaa     C++ aaa+\0
    this->data_source = new char[strlen(data_source) +1];
    strcpy(this->data_source, data_source);
}

// 准备工作是去，”解码“， 才包裹（音频流，视频流，字幕流 ....）
// av_register_all()  FFmpeg 2.用的
void MyPlayer::prepare() {
    // 创建异步线程
    pthread_create(&this->pid_prepare, 0, customTaskPrepareThread, this);
}

void MyPlayer::prepare_() {
    // 拆包裹
    this->avFormatContext = avformat_alloc_context();

    // 此字典 能够决定我们打开的需求
    AVDictionary * dictionary = 0;
    // 注意：单位是微妙，如果：你的模拟器很卡，尽量设置大一点
    av_dict_set(&dictionary, "timeout", "5000000", 0);

    // 回收字典
    av_dict_free(&dictionary);

    // 是不是包裹，如果对方寄来的是，石头（被损坏的数据），那就没法玩了
    int ret = avformat_open_input(&this->avFormatContext, this->data_source, 0, &dictionary);
    if (ret) {
        // .... 写JNI回调，告诉Java层，通知用户，你的播放流损坏的
        return;
    }

    // 寻找，媒体格式中的，（音频，视频，字幕）
    ret = avformat_find_stream_info(this->avFormatContext, 0); // 为什么不给字典，是因为，不需要设置而外配置
    if (ret < 0) {
        // .... 写JNI回调，告诉Java层，通知用户，你的播放流损坏的
        return;
    }

    // 媒体上下文，就有了丰富的值了 == avFormatContext

    // 循环遍历，媒体格式里面的 流1==音频 流2==字幕 流0==视频
    for (int i = 0; i < this->avFormatContext->nb_streams; ++i) {
        // TOOD 同学们 下节课，我们来  获取 音视频流
    }
}
