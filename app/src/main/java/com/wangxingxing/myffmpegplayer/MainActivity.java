package com.wangxingxing.myffmpegplayer;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import com.blankj.utilcode.util.AppUtils;
import com.blankj.utilcode.util.LogUtils;
import com.blankj.utilcode.util.PermissionUtils;
import com.blankj.utilcode.util.SPUtils;
import com.blankj.utilcode.util.ToastUtils;

import java.io.File;

public class MainActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener {

    private static final String TAG = "MainActivity";

    public static final String PATH_HUNANTV_RTMP = "rtmp://58.200.131.2:1935/livetv/hunantv";
    private final static String PATH_LOCAL = Environment.getExternalStorageDirectory() + File.separator + "demo.mp4";

    public static final int MSG_UPDATE_TEXT = 1;
    public static final int MSG_UPDATE_PROGRESS = 2;
    public static final int MSG_IS_SHOW_SEEKBAR = 3;

    // 0:MP4; 1:RTMP
    public static final String SP_KEY_TYPE = "type";

    private SurfaceView surfaceView;
    private TextView tvInfo;
    private SeekBar seekBar;

    private MyPlayer myPlayer;

    private long mFirstPressTime = 0;
    private boolean isTouch;
    private boolean isSeek;

    private int duration = 0;
    private String videoPath = "";

    private String[] permissions = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        checkPermission();

        surfaceView = findViewById(R.id.surface_view);
        tvInfo = findViewById(R.id.tv_info);
        seekBar = findViewById(R.id.seek_bar);
        seekBar.setOnSeekBarChangeListener(this);


        initPlayer();
    }

    private void initPlayer() {
        myPlayer = new MyPlayer();
        myPlayer.setSurfaceView(surfaceView);
        int playType = SPUtils.getInstance().getInt(SP_KEY_TYPE, 0);
        if (playType == 0) {
            videoPath = PATH_LOCAL;
        } else {
            videoPath = PATH_HUNANTV_RTMP;
        }
        myPlayer.setDataSource(videoPath);

        sendInfoMsg("ffmpeg version:" + myPlayer.getFFmpegVersion());
        sendInfoMsg("ffmpeg player init, play url: " + videoPath);

        myPlayer.setOnPreparedListener(new MyPlayer.OnPreparedListener() {
            @Override
            public void onPrepared() {
                duration = myPlayer.getDuration();
                LogUtils.i("准备好了，开始播放...");
                LogUtils.i("video duration: " + duration);
                sendShowSeekBarMsg();
                sendInfoMsg("准备好了，开始播放...");

                // 准备成功之后，开始播放 视频 音频
                myPlayer.start();
            }

            @Override
            public void onError(final String errorText) {
                sendInfoMsg("发生错误，请查阅：" + errorText);
            }
        });

        myPlayer.setOnProgressListener(new MyPlayer.OnProgressListener() {
            @Override
            public void onProgress(int progress) {
//                Log.d(TAG, "onProgress: " + progress);

                //非人为干预进度条，让进度条自然的正常播放
                if (!isTouch) {
                    if (duration != 0) {
                        if (isSeek) {
                            isSeek = false;
                            return;
                        }
                        sendProgressMsg(progress * 100 / duration);
                    }
                }
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (myPlayer != null) {
            myPlayer.prepare();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (myPlayer != null) {
            myPlayer.stop();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (myPlayer != null) {
            myPlayer.release();
        }
    }

    @Override
    public void onBackPressed() {
        if (System.currentTimeMillis() - mFirstPressTime < 2000) {
            super.onBackPressed();
        } else {
            mFirstPressTime = System.currentTimeMillis();
            ToastUtils.showShort("再按一次返回键退出");
        }
    }

    private void checkPermission() {
        if (!PermissionUtils.isGranted(permissions)) {
            PermissionUtils.permission(permissions).request();
        }
    }

    private void sendInfoMsg(String str) {
        Message msg = new Message();
        msg.what = MSG_UPDATE_TEXT;
        msg.obj = str;
        mHandler.sendMessage(msg);
    }

    private void updateInfo(String str) {
        StringBuffer sb = new StringBuffer();
        sb.append(tvInfo.getText());
        sb.append("\n");
        sb.append(str);
        tvInfo.setText(sb.toString());
    }

    private void sendProgressMsg(int progress) {
        Message msg = new Message();
        msg.what = MSG_UPDATE_PROGRESS;
        msg.arg1 = progress;
        mHandler.sendMessage(msg);
    }

    private void sendShowSeekBarMsg() {
        Message msg = mHandler.obtainMessage(MSG_IS_SHOW_SEEKBAR);
        //如果是直播，duration是0
        //不为0，可以显示seekbar
        if (duration != 0) {
            msg.obj = true;
        } else {
            msg.obj = false;
        }
        mHandler.sendMessage(msg);
    }

    private void updateProgress(int progress) {
        seekBar.setProgress(progress);
    }

    public void playMP4(View view) {
        SPUtils.getInstance().put(SP_KEY_TYPE, 0);
        AppUtils.relaunchApp(true);
    }

    public void playRTMP(View view) {
        SPUtils.getInstance().put(SP_KEY_TYPE, 1);
        AppUtils.relaunchApp(true);
    }

    public void btnPause(View view) {
        if (myPlayer != null) {
            myPlayer.stop();
        }
    }

    private Handler mHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(@NonNull Message msg) {
            switch (msg.what) {
                case MSG_UPDATE_TEXT:
                    updateInfo(msg.obj.toString());
                    break;
                case MSG_UPDATE_PROGRESS:
                    updateProgress(msg.arg1);
                    break;
                case MSG_IS_SHOW_SEEKBAR:
                    if ((boolean) msg.obj) {
                        seekBar.setVisibility(View.VISIBLE);
                    } else {
                        seekBar.setVisibility(View.GONE);
                    }
                    break;
            }
            super.handleMessage(msg);
        }
    };

    /**
     * SeekBar callback start
     * seek 的核心思路1
     * 跟随播放进度自动刷新进度：拿到每个时间点相对总播放时长的百分比进度 progress
     * 1，总时间 getDurationNative
     * 2，当前播放时间: 随播放进度动态变化的
     * ===================================================================================
     */
    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        isTouch = true; // 用户 手触摸到了 拖动条
    }

    /**
     * 当用户手 离开拖动条的时候，就在这一刻：把拖动条的值 告诉给底层
     *
     * @param seekBar
     */
    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        isSeek = true;
        isTouch = false;

        //获取seekbar的当前进度（百分比）
        int seekBarProgress = seekBar.getProgress();
        //将seekbar的进度转换成真实的播放进度
        int playProgress = seekBarProgress * duration / 100; // 把拖动条的值，变成播放的进度时长

        /*
         * 将播放进度传给底层 ffmpeg
         * playProgress == 时长 native只认识时长，不认识拖动值
         *
         * seek 的核心思路2
         * 手动拖动进度条，要能跳到指定的播放进度  av_seek_frame
         * */
        LogUtils.i("playProgress: " + playProgress);
        myPlayer.seekTo(playProgress);
    }

    /** SeekBar callback end =============================================================*/
}
