//
// Created by WangXingxing on 2020/4/15.
//

#ifndef MYFFMPEGPLAYER_MACRO_H
#define MYFFMPEGPLAYER_MACRO_H

#include <android/log.h>

//定义释放的宏函数
#define DELETE(object) if (object) { delete object; object = 0;}

//定义日志打印宏函数
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "MYFFMPEG",__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "MYFFMPEG",__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "MYFFMPEG",__VA_ARGS__)

#define THREAD_MAIN 1   // 此宏代表 主线程的意思
#define THREAD_CHILD 2  // 此宏代表 子线程的意思

//打不开媒体数据源
#define FFMPEG_CAN_NOT_OPEN_URL -1

//找不到媒体流信息
#define FFMPEG_CAN_NOT_FIND_STREAMS -2

//找不到解码器
#define FFMPEG_FIND_DECODER_FAIL -3

//无法根据解码器创建上下文
#define FFMPEG_ALLOC_CODEC_CONTEXT_FAIL -4

//根据流信息 配置上下文参数失败
#define FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL -5

//打开解码器失败
#define FFMPEG_OPEN_DECODER_FAIL -6

//没有音视频
#define FFMPEG_NOMEDIA -7

//读取媒体数据包失败
#define FFMPEG_READ_PACKETS_FAIL (ERROR_CODE_FFMPEG_PLAY - 8)

//拖动进度条设置进度异常
#define FFMPEG_SET_PROGRESS_FAIL -9

#endif //MYFFMPEGPLAYER_MACRO_H
