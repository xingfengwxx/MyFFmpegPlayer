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

/**
 * 准备-其实就是解封装
 */
void MyPlayer::prepare() {
    // 解析媒体流，通过ffmpeg API 来解析 dataSource

    // 思考：可以直接解析吗？
    // 答：1.可能是文件，考虑到io流
    //     2.可能是直播，考虑到网络
    // 所以需要异步

    // 是由MainActivity中方法调用下来的，所以是属于在main线程，不能再main线程操作，所以需要异步
    // 创建异步线程
    pthread_create(&this->pid_prepare, 0, customTaskPrepareThread, this);
}

void MyPlayer::prepare_() {
    // TODO 【大流程是：解封装】

    // TODO 第一步：打开媒体地址（文件路径 、 直播地址）

    // 可以初始为NULL，如果初始为NULL，当执行avformat_open_input函数时，内部会自动申请avformat_alloc_context，这里干脆手动申请
    // 封装了媒体流的格式信息
    formatContext = avformat_alloc_context(); // 媒体上下文（总上下文）（打开包裹）

    // 此字典 能够决定我们打开的需求
    AVDictionary * dictionary = 0;
    // 注意：单位是微妙，如果：你的模拟器很卡，尽量设置大一点
    av_dict_set(&dictionary, "timeout", "5000000", 0);

    int ret = avformat_open_input(&formatContext, data_source, 0, &dictionary); // 打开文件需要字典

    // 注意：字典使用过后，一定要去释放，回收字典
    av_dict_free(&dictionary);

    if (ret) {
        /**
         * ret 不等于 0    ffmpeg【0==success】
         * 你的文件路径，或，你的文件损坏了，需要告诉用户
         * 把错误信息，告诉给Java层去（回调给Java）
         */
        if (pCallback) {
            this->pCallback->onErrorAction(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);
        }
        return;
    }

    // TODO 第二步：1.查找媒体中的音视频流的信息  2.给媒体上下文初始化
    ret = avformat_find_stream_info(this->formatContext, 0);
    if (ret < 0) {
        // 错误信息回调给Java层
        if (pCallback) {
            pCallback->onErrorAction(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
        }
        return;
    }

    // TODO 第三步：根据流信息，流的个数，循环查找， 音频流 视频流
    // nb_streams == 流的个数
    // 循环遍历，媒体格式里面的 流1==音频 流2==字幕 流0==视频
    for (int stream_index = 0; stream_index < this->formatContext->nb_streams; ++stream_index) {
        // TODO 第四步：获取媒体流（视频、音频）
        AVStream * stream = formatContext->streams[stream_index];

        // TODO 第五步：从stream流中获取解码这段流的参数信息，区分到底是 音频 还是 视频
        AVCodecParameters * codecParameters = stream->codecpar;

        // TODO 第六步：通过流的编解码参数中编解码ID，来获取当前流的解码器
        AVCodec * codec = avcodec_find_decoder(codecParameters->codec_id);
        if (!codec) {
            /**
             * 如果为空，就代表：解码器不支持
             * 错误信息回调给Java层
             */
             pCallback->onErrorAction(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
             return;
        }

        // TODO 第七步：通过 拿到的解码器，获取解码器上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            // 错误信息回调给Java层
            pCallback->onErrorAction(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            return;
        }

        // TODO 第八步：给解码器上下文 设置参数 (内部会让编解码器上下文初始化)
        ret = avcodec_parameters_to_context(codecContext, codecParameters);
        if (ret < 0) {
            // 错误信息回调给Java层
            pCallback->onErrorAction(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            return;
        }

        // TODO 第九步：打开解码器
        ret = avcodec_open2(codecContext, codec, 0);
        if (ret) { // 不等于0
            // 错误信息回调给Java层
            pCallback->onErrorAction(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            return;
        }

        // AVStream媒体流中就可以拿到时间基 (音视频同步)
        AVRational time_base = stream->time_base;

        // TODO 第十步：从编码器参数中获取流类型codec_type
        if (codecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            // 音频流
             audioChannel = new AudioChannel(stream_index, codecContext, time_base);
        } else if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) { // 视频流（目前很多字幕流，都放在视频轨道中）
            /*
             * 获取视频相关的 fps
             * 平均帧率 == 时间基
             * */
            AVRational frame_rate = stream->avg_frame_rate;
            int fpsValue = av_q2d(frame_rate);
        }
    }
}
