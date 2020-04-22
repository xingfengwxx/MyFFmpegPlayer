# MyFFmpegPlayer
## 功能

1.播放本地视频（可进度条拖动）

2.播放RTMP直播流



## 流程

### 解码流程

![流程图-1](res\pic_1.png)

![视频原始数据](res\pic_6.png)

![解码流程-2](res\pic_9.png)

### 调用路径

![调用路径](res\pic_10.png)

### P帧、B帧、I帧

![P帧、B帧、I帧](res\pic_7.png)

### 音频重采样

![音频重采样-1](res\pic_8.png)

![](res\pic_12.png)

![](res\pic_11.png)

### 音视频流程对比

![](res\pic_13.png)

### 动作图

![动作图](res\pic_14.png)

音视频同步

![](res\pic_15.png)

### 进度条

![](res\pic_16.png)

## 笔记

```
湖南卫视 rtmp/rtsp 直播流  不需要进度条   --  duration是0
本地视频文件      .mp4         进度条     --  duration是有值

一：媒体文件总时长
1.得到native-->audio时长值（优先是获取音频时间，如果音频没有的话，在去获取视频时间）
2.把audio时长值，通过JNICallback方式回调上去
3.Java MainActivity 拖动条设置，就可以动起来了

二：用户去拖动 拖动条
1.Java MainActivity 用户去拖动，拖动监听事件（当用户手 离开拖动条的时候，就在这一刻：把拖动条的值（转成时长） 告诉给底层）
2.把时长值，传递给了底层
3.（native） av_seek_frame
4.（native）重新播放，让所以的队列，全部停止，全部清空，再开始运行

三：stop
1.关闭媒体格式上下文（防止内存泄漏）
2.回收媒体格式上下文
3.回收不用的 对象

四：release
1.释放 window 重量级的，渲染画面的
2.释放 中转站 涉及的代码太多
```