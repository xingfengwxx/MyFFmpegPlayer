//
// Created by WangXingxing on 2020/4/15.
// 视频解码 和 音频解码 有公用的东西，所以需要抽取父类

#ifndef MYFFMPEGPLAYER_BASECHANNEL_H
#define MYFFMPEGPLAYER_BASECHANNEL_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
};

#include "safe_queue.h"
class BaseChannel {

};

#endif //MYFFMPEGPLAYER_BASECHANNEL_H
