package com.wangxingxing.myffmpegplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;

public class DoKitActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_do_kit);


    }

    public void toMain(View view) {
        startActivity(new Intent(this, MainActivity.class));
        finish();
    }
}
