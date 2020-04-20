package com.wangxingxing.myffmpegplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.os.Bundle;
import android.os.Environment;
import android.view.SurfaceView;
import android.widget.TextView;

import com.blankj.utilcode.util.LogUtils;
import com.blankj.utilcode.util.PermissionUtils;
import com.blankj.utilcode.util.ToastUtils;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    private SurfaceView surfaceView;
//    public static final String PATH = "rtmp://58.200.131.2:1935/livetv/hunantv";
    private final static String PATH = Environment.getExternalStorageDirectory() + File.separator + "demo.mp4";
    MyPlayer myPlayer;

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

        initPlayer();
    }

    private void initPlayer() {
        myPlayer = new MyPlayer();
        myPlayer.setSurfaceView(surfaceView);
        myPlayer.setDataSource(PATH);

        myPlayer.setOnPreparedListener(new MyPlayer.OnPreparedListener() {
            @Override
            public void onPrepared() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        ToastUtils.showShort("准备好了，开始播放...");
                    }
                });
                LogUtils.i("准备好了，开始播放...");

                // 准备成功之后，开始播放 视频 音频
                myPlayer.start();
            }

            @Override
            public void onError(final String errorText) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        ToastUtils.showShort("发生错误，请查阅：" + errorText);
                    }
                });
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

    private void checkPermission() {
        if (!PermissionUtils.isGranted(permissions)) {
            PermissionUtils.permission(permissions).request();
        }
    }
}
