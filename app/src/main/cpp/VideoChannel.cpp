//
// Created by WangXingxing on 2020/4/16.
//

#include "VideoChannel.h"

// 丢包的  函数指针 具体实现 frame
void dropAVFrame(queue<AVFrame *> & qq) {
    if (!qq.empty()) {
        AVFrame * frame = qq.front();
        BaseChannel::releaseAVFrame(&frame);  // 释放掉
        qq.pop();
    }
}

// 丢包的  函数指针 具体实现 packet
void dropAVPacket(queue<AVPacket *> & qq) {
    if (!qq.empty()) {
        AVPacket * packet = qq.front();
        // BaseChannel::releaseAVPacket(&packet); // 关键帧没有了
        if (packet->flags != AV_PKT_FLAG_KEY) { // AV_PKT_FLAG_KEY == 关键帧
            BaseChannel::releaseAVPacket(&packet);
        }
        qq.pop();
    }
}

VideoChannel::VideoChannel(int stream_index, AVCodecContext *pContext, AVRational rational,
                           int fpsValue) : BaseChannel(stream_index, pContext, rational) {
    this->fpsValue = fpsValue;
    this->frames.setSyncCallback(dropAVFrame);
    this->packages.setSyncCallback(dropAVPacket);
}

VideoChannel::~VideoChannel() {}

// 解码的函数指针  run
void * task_video_decode(void * pVoid) { // pVoid == this == VideoChannel
    VideoChannel * videoChannel = static_cast<VideoChannel *>(pVoid);
    videoChannel->video_decode();
    return 0;
}

// 播放的函数指针 run
void * task_video_player(void * pVoid) {
    VideoChannel * videoChannel = static_cast<VideoChannel *>(pVoid);
    videoChannel->video_player();
    return 0;
}

/**
 * 主线程
 * 真正的解码，并且，播放
 * 1.解码（解码只有的是原始数据）
 * 2.播放
 */
void VideoChannel::start() {
    isPlaying = 1;

    // 存放未解码的队列开始工作了
    packages.setFlag(1);

    // 存放解码后的队列开始工作了
    frames.setFlag(1);

    // 1.解码的线程
    pthread_create(&pid_video_decode, 0, task_video_decode, this);

    // 2.播放的线程
    pthread_create(&pid_video_player, 0, task_video_player, this);
}

void VideoChannel::stop() {

}

/**
 * 子线程
 * 运行在异步线程（视频真正解码函数）
 */
 void VideoChannel::video_decode() {
     AVPacket * packet = 0; // AVPacket 专门存放 压缩数据（H264）
     while (isPlaying) {
         /*
          * 生产快  消费慢
          * 消费速度比生成速度慢（生成100，只消费10个，这样队列会爆）
          * 内存泄漏点2，解决方案：控制队列大小
          * */
         if (isPlaying && frames.queueSize() > 100) {
             // 休眠 等待队列中的数据被消费
             av_usleep(10 * 1000); // 微秒
             continue;
         }

         // 压缩数据
         int ret = packages.pop(packet);

         // 如果停止播放，跳出循环, 出了循环，就要释放
         if (!isPlaying) {
             break;
         }

         if (!ret) {
             continue;
         }

         // 第一步：AVPacket 从队列取出一个饭盒

         // 走到这里，就证明，取到了待解码的视频数据包
         ret = avcodec_send_packet(pContext, packet); // 第二步：把饭盒给食堂阿姨，食堂阿姨（内部维持一个队列）打菜
         if (ret) {
             // 失败了（给”解码器上下文“发送Packet失败）
             break;
         }

         // 走到这里，就证明，packet，用完毕了，成功了（给“解码器上下文”发送Packet成功），那么就可以释放packet了
         releaseAVPacket(&packet);

         AVFrame * frame = av_frame_alloc(); // AVFrame 拿到解码后的原始数据包 （原始数据包，内部还是空的）
         ret = avcodec_receive_frame(pContext, frame); //第三步： AVFrame 就有了值了 原始数据==YUV，最终打好的饭菜（原始数据）
         if (ret == AVERROR(EAGAIN)) {
             // 重来，重新取，没有取到关键帧，重试一遍
             releaseAVFrame(&frame); // 内存释放点
             continue;
         } else if (ret != 0) {
             // 释放规则：必须是后面不再使用了，才符合释放的要求
             releaseAVFrame(&frame); // 内存释放点
             break;
         }

         // TODO AVPacket H264压缩数据 ----》 AVFrame YUV原始数据

         // 终于取到了，解码后的视频数据（原始数据）
         frames.push(frame); // 加入队列，为什么没有使用，无法满足"释放规则"
     }

     /*
      * 必须考虑的
      * 出了循环，就要释放
      * 释放规则：必须是后面不再使用了，才符合释放的要求
      * */
     releaseAVPacket(&packet);
 }

/**
* 运行在异步线程（视频真正播放函数）
*/
void VideoChannel::video_player() {
    // Sws视频   Swr音频
    // 原始图形宽和高，可以通过解码器拿到
    // 1.TODO 把元素的 yuv  --->  rgba
}
