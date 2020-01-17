package com.wangxingxing.myffmpegplayer;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class MyPlayer implements SurfaceHolder.Callback {

    // rtmp网络直播流，本地流
    private String dataSource;
    private SurfaceHolder surfaceHolder;

    static {
        System.loadLibrary("my-player");
    }

    public MyPlayer() {

    }

    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }

    public void setSurfaceView(SurfaceView surfaceView) {
        // 每次设置SurfaceView的时候，判断Holder是否被清除
        if (surfaceHolder != null) {
            surfaceHolder.removeCallback(this);
        }
        surfaceHolder = surfaceView.getHolder();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        setSurfaceNative(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    /**
     * 上层区域
     */
    public void prepare() {
        prepareNative(this.dataSource);
    }

    public void start() {
        startNative();
    }

    public void stop() {
        stopNative();
    }

    public void release() {
        releaseNative();
    }


    /**
     * native区域
     */
    public native void prepareNative(String dataSource);
    public native void startNative();
    public native void stopNative();
    public native void releaseNative();
    // 告诉底层，上层可以怎么去渲染，其实底层是操控 Surface对象
    public native void setSurfaceNative(Surface surface);
    public native String getFFmpegVersion();
}
