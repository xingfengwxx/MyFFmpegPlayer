# MyFFmpegPlayer
## 功能

1.播放本地视频（可进度条拖动）

2.播放RTMP直播流



## 流程

### 解码流程

![pic_1.png](http://ww1.sinaimg.cn/large/006iQcGSgy1ge2pkgu81rj31g60ljt9a.jpg)

![视频原始数据](http://ww1.sinaimg.cn/large/006iQcGSgy1ge2po7c5v9j30sg0lcaav.jpg)

![解码流程-2](http://ww1.sinaimg.cn/large/006iQcGSgy1ge2pok9gxuj30sg0lcq4s.jpg)

### 调用路径

![调用路径](http://ww1.sinaimg.cn/large/006iQcGSgy1ge2pp047auj30mr1kcgpw.jpg)

### P帧、B帧、I帧

![P帧、B帧、I帧](http://ww1.sinaimg.cn/large/006iQcGSgy1ge2ppcty0aj30sg0lcjsq.jpg)

### 音频重采样

![音频重采样-1](http://ww1.sinaimg.cn/large/006iQcGSgy1ge2ppm6p8ej30sg0lcaaz.jpg)

![](http://ww1.sinaimg.cn/large/006iQcGSgy1ge2pq3x460j30sg0lcmxu.jpg)

![pic_11.png](http://ww1.sinaimg.cn/large/006iQcGSgy1ge2pqhbohjj30sg0lc3ze.jpg)

### 音视频流程对比

![pic_13.png](http://ww1.sinaimg.cn/large/006iQcGSgy1ge2pqzq8p9j31cl1lragz.jpg)

### 动作图

![pic_14.png](http://ww1.sinaimg.cn/large/006iQcGSgy1ge2prc6c1sj30rs28xwlc.jpg)

音视频同步

![pic_15.png](http://ww1.sinaimg.cn/large/006iQcGSgy1ge2prkeerej30sg0lcwf3.jpg)

### 进度条

![pic_16.png](http://ww1.sinaimg.cn/large/006iQcGSgy1ge2prsqs7sj31g60lj0su.jpg)

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

## 第三方库

1. [性能监测工具、开发助手](https://github.com/didi/DoraemonKit)
2. [AndroidUtilCode](https://github.com/Blankj/AndroidUtilCode)