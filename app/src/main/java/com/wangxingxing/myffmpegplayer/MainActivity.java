package com.wangxingxing.myffmpegplayer;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.SurfaceView;
import android.widget.TextView;

import com.blankj.utilcode.util.LogUtils;
import com.blankj.utilcode.util.PermissionUtils;
import com.blankj.utilcode.util.ToastUtils;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    //    public static final String PATH = "rtmp://58.200.131.2:1935/livetv/hunantv";
    private final static String PATH = Environment.getExternalStorageDirectory() + File.separator + "demo.mp4";

    public static final int MSG_UPDATE_TEXT = 1;

    private SurfaceView surfaceView;
    private TextView tvInfo;

    private MyPlayer myPlayer;

    private long mFirstPressTime = 0;

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

        initPlayer();
    }

    private void initPlayer() {
        myPlayer = new MyPlayer();
        myPlayer.setSurfaceView(surfaceView);
        myPlayer.setDataSource(PATH);

        sendMsg("FFmpeg version:" + myPlayer.getFFmpegVersion());

        myPlayer.setOnPreparedListener(new MyPlayer.OnPreparedListener() {
            @Override
            public void onPrepared() {
                LogUtils.i("准备好了，开始播放...");
                sendMsg("准备好了，开始播放...");

                // 准备成功之后，开始播放 视频 音频
                myPlayer.start();
            }

            @Override
            public void onError(final String errorText) {
                sendMsg("发生错误，请查阅：" + errorText);
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        myPlayer.prepare();
    }

    @Override
    protected void onPause() {
        super.onPause();
        myPlayer.stop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        myPlayer.release();
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

    private void sendMsg(String str) {
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

    private Handler mHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(@NonNull Message msg) {
            switch (msg.what) {
                case MSG_UPDATE_TEXT:
                    updateInfo(msg.obj.toString());
                    break;
            }
            super.handleMessage(msg);
        }
    };
}
