//
// Created by WangXingxing on 2020/1/17.
//

#include "MyPlayer.h"

// TODO 异步 函数指针 - 准备工作prepare
void * customTaskPrepareThread(void * pVoid) {
    MyPlayer * myPlayer = static_cast<MyPlayer *>(pVoid);
    myPlayer->prepare_();
    return 0; // 必须返回，有坑
}

// TODO 异步 函数指针 - 开始播放工作start
void * customTaskStartThread(void * pVoid) {
    MyPlayer * myPlayer = static_cast<MyPlayer *>(pVoid);
    myPlayer->start_();
    return 0; // 坑：一定要记得return
}

//设置为友元函数
void * task_stop(void * args) {
    MyPlayer *myPlayer = static_cast<MyPlayer *>(args);
    myPlayer->stop_();

    return 0; //一定一定一定要返回0！！！
}

MyPlayer::MyPlayer() {}

MyPlayer::MyPlayer(const char *data_source, JNICallback *pCallback) {
    // 这里有坑，这里赋值之后，不能给其他地方用，因为被释放了，变成了悬空指针
    // this->data_source = data_source;

    // 解决上面的坑，自己Copy才行
    // [strlen(data_source)] 这段代码有坑：因为（hello  而在C++中是 hello\n），所以需要加1
    this->data_source = new char[strlen(data_source) + 1];
    strcpy(this->data_source, data_source);

    this->pCallback = pCallback;
    pthread_mutex_init(&seekMutex, 0);
}

MyPlayer::~MyPlayer() {
    if (this->data_source) {
        delete this->data_source;
        this->data_source = 0;
        pthread_mutex_destroy(&seekMutex);
    }
}

/*MyPlayer::MyPlayer(const char *dataSource) {
    // 有坑， 会报错
    // this->data_source = data_source;

    // 有坑，长度不对   C:aaa     C++ aaa+\0
    this->data_source = new char[strlen(data_source) +1];
    strcpy(this->data_source, data_source);
}*/

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
    // 注意：单位是微妙，如果：你的模拟器很卡，尽量设置大一点,这里是10秒
    av_dict_set(&dictionary, "timeout", "10000000", 0);

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

    duration = formatContext->duration / AV_TIME_BASE; // 最终是要拿到秒

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
             audioChannel = new AudioChannel(stream_index, codecContext, time_base, pCallback);
        } else if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) { // 视频流（目前很多字幕流，都放在视频轨道中）
            /*
             * 获取视频相关的 fps
             * 平均帧率 == 时间基
             * */
            AVRational frame_rate = stream->avg_frame_rate;
            int fpsValue = av_q2d(frame_rate);

            videoChannel = new VideoChannel(stream_index, codecContext, time_base, fpsValue, pCallback);
            videoChannel->setRenderCallback(renderCallback);
        }
    } // end for

    // TODO 第十一步：如果流中 没有音频 也 没有视频
    if (!audioChannel && !videoChannel) {
        // 错误信息回调给Java层
        pCallback->onErrorAction(THREAD_CHILD, FFMPEG_NOMEDIA);
        return;
    }

    // TODO 第十二步：要么有音频，要么有视频，要么音视频都有
    // 准备完毕，通知Android上层开始播放
    if (this->pCallback) {
        pCallback->onPrepared(THREAD_CHILD); // 准备成功
    }
}

/**
 * 开始播放 -- 主线程
 */
void MyPlayer::start() {
    isPlaying = 1;
    if (videoChannel) {
        videoChannel->setAudioChannel(audioChannel);
        videoChannel->start();
    }
    if (audioChannel) {
        audioChannel->start();
    }
    // 子线程  把压缩数据 存入到队列里面去
    pthread_create(&pid_start, 0, customTaskStartThread, this);
}

/**
 * 真正开始播放，属于子线程
 *
 * 此函数（读包（未解码 音频/视频 包） 放入队列）
 *
 * 把 音频 视频 压缩数据包 放入队列
 */
void MyPlayer::start_() {
    // 循环 读视频包
    while (isPlaying) {
        // TODO 由于我们的操作是在异步线程，那就好办了，等待（先让消费者消费掉，然后在生产）
        // 下面解决方案：通俗易懂 让生产慢一点，消费了，在生产
        // 内存泄漏点1，解决方案：控制队列大小
        if (videoChannel && videoChannel->packages.queueSize() > 100) {
            // 休眠 等待队列中的数据被消费
            av_usleep(10 * 1000);
            continue;
        }
        // 内存泄漏点1，解决方案：控制队列大小
        if (audioChannel && audioChannel->packages.queueSize() > 100) {
            // 休眠 等待队列中的数据被消费
            av_usleep(10 * 1000);
            continue;
        }

        // AVPacket 可能是音频 可能是视频, 没有解码的（数据包） 压缩数据AVPacket
        AVPacket * packet = av_packet_alloc();
        pthread_mutex_lock(&seekMutex);
        int ret = av_read_frame(formatContext, packet); // 这行代码一执行完毕，packet就有（音视频数据）
        pthread_mutex_unlock(&seekMutex);
        /*if (ret != 0) {
           // 后续处理
           return;
       }*/

        if (!ret) { // ret == 0
            /*
             * 把已经得到的packet 放入队列中
             * 先判断是视频  还是  音频， 分别对应的放入 音频队列  视频队列
             * packet->stream_index 对应之前的prepare中循环的i
             * */
            if (videoChannel && videoChannel->stream_index == packet->stream_index) {
                /*
                 * 如果他们两 相等 说明是视频  视频包
                 * 未解码的 视频数据包 加入到队列
                 * */
                videoChannel->packages.push(packet);
            } else if (audioChannel && audioChannel->stream_index == packet->stream_index) {
                /*
                 * 如果他们两 相等 明是音频  音频包
                 * 未解码的 音频数据包 加入到队列
                 * */
                audioChannel->packages.push(packet);
            }
        } else if (ret == AVERROR_EOF) { // or   end of file， 文件结尾，读完了 的意思
            // 代表读完了
            // TODO 一定是要 读完了 并且 也播完了，才做事情
        } else { // ret  != 0
            // 代表失败了，有问题
            break;
        }
    }
    // end while

    // 最后做释放的工作
    isPlaying = 0; // 标记清零
    videoChannel->stop();
    audioChannel->stop();
}

void MyPlayer::stop_() {
    /*
 * 要保证_prepare方法（子线程中）执行完再释放（在主线程）
 * pthread_join ：这里调用了后会阻塞主，可能引发ANR
 * */
    isPlaying = 0; // 修改成false，音频和视频，的所有解码 和 播放 等等，全部停止了
    pthread_join(pid_prepare, 0); //解决了：要保证_prepare方法（子线程中）执行完再释放（在主线程）的问题

    if (formatContext) {
        avformat_close_input(&formatContext); // 关闭媒体格式上下文
        avformat_free_context(formatContext); // 回收媒体格式上下文
        formatContext = 0;
    }

    DELETE(videoChannel);
    DELETE(audioChannel);
}

void MyPlayer::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}

void MyPlayer::stop() {
    //    isPlaying = 0;
    pCallback = 0; //prepare阻塞中停止了，还是会回调给java "准备好了"

    //既然在主线程会引发ANR，那么我们到子线程中去释放
    //创建stop子线程
    pthread_create(&this->pid_stop, 0, task_stop, this);
}



int MyPlayer::getDuration() const {
    return duration;
}

/**
 * 异步线程
 * 控制 播放时长的，音频 和 视频 的 快进快退
 * @param progress 进度值
 */
void MyPlayer::seekTo(int progress) {
    LOGD("seekTo progress: %d", progress);
    if (progress < 0 || progress > duration) {
        // 错误信息回调给Java层
        pCallback->onErrorAction(THREAD_CHILD, FFMPEG_SET_PROGRESS_FAIL);
        LOGE("error: %s", FFMPEG_SET_PROGRESS_FAIL);
        return;
    }

    if (!audioChannel && !videoChannel) {
        pCallback->onErrorAction(THREAD_CHILD, FFMPEG_NOMEDIA);
        LOGE("error: %s", FFMPEG_NOMEDIA);
        return;
    }

    if (!formatContext) {
        pCallback->onErrorAction(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
        LOGE("error: %s", FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
        return;
    }

    /*
     * 1,上下文
     * 2，流索引，-1：表示选择的是默认流
     * 3，要seek到的时间戳
     * 4，seek的方式
     *
     * AVSEEK_FLAG_BACKWARD： 表示seek到请求的时间戳之前的最靠近的一个关键帧
     * AVSEEK_FLAG_BYTE：基于字节位置seek
     * AVSEEK_FLAG_ANY：任意帧（可能不是关键帧，会花屏）
     * AVSEEK_FLAG_FRAME：基于帧数seek
     * */

    // 为什么要加互斥锁？ 我们用到了 媒体格式上下文formatContext，（音频通道，视频通道 都用到了formatContext）为了安全，所以加锁
    pthread_mutex_lock(&seekMutex);

    int ret = av_seek_frame(formatContext, -1, progress * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);
    LOGD("seekTo ret=%d", ret);

    if (ret < 0) {
        pCallback->onErrorAction(THREAD_CHILD, ret);
        return;
    }

    /**
     * 用户在拖动的过程中，还没有松手的过程中，（正在解码，正在播放.....）
     * 上层手放开了，拖动值---》时长值 ---> native（重新播放：跳转到用户指定的位置播放）
     */
    if (audioChannel) {
        audioChannel->packages.setFlag(0); // 全部停止 -- 不要在解码了，不要在播放了
        audioChannel->frames.setFlag(0); // 全部停止 -- 不要在解码了，不要在播放了
        audioChannel->packages.clearQueue(); // 停止后 收尾动作
        audioChannel->frames.clearQueue(); // 停止后 收尾动作
        //清除数据后，让队列重新工作
        audioChannel->packages.setFlag(1); // 让队列里面的一切，开始工作....
        audioChannel->frames.setFlag(1); // 让队列里面的一切，开始工作....
    }

    if (videoChannel) {
        videoChannel->packages.setFlag(0); // 让队列里面的一切，开始工作....
        videoChannel->frames.setFlag(0); // 让队列里面的一切，开始工作....
        videoChannel->packages.clearQueue(); // 停止后 收尾动作
        videoChannel->packages.clearQueue(); // 停止后 收尾动作
        //清除数据后，让队列重新工作
        videoChannel->packages.setFlag(1); // 让队列里面的一切，开始工作....
        videoChannel->frames.setFlag(1); // 让队列里面的一切，开始工作....
    }

    pthread_mutex_unlock(&seekMutex);
}
