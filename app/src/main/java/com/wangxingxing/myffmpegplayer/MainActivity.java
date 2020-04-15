package com.wangxingxing.myffmpegplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.SurfaceView;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    private SurfaceView surfaceView;
    public static final String PATH = "rtmp://58.200.131.2:1935/livetv/hunantv";
    MyPlayer myPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        surfaceView = findViewById(R.id.surface_view);

//        myPlayer = new MyPlayer();
//        myPlayer.setDataSource(PATH);
//        myPlayer.setSurfaceView(surfaceView);
    }

    @Override
    protected void onResume() {
        super.onResume();
        myPlayer.prepare();
    }

    @Override
    protected void onPause() {
        super.onPause();
        myPlayer.prepare();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        myPlayer.release();
    }
}
