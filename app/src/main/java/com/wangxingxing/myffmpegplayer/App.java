package com.wangxingxing.myffmpegplayer;

import android.app.Application;
import android.content.Context;

import androidx.multidex.MultiDex;

import com.blankj.utilcode.util.Utils;
import com.didichuxing.doraemonkit.DoraemonKit;

public class App extends Application {

    public static final String DOKIT_PID = "04bbcdda71df1349a293a99b46e1ff87";

    @Override
    public void onCreate() {
        super.onCreate();

        Utils.init(this);
        DoraemonKit.install(this, null, DOKIT_PID);
    }

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        MultiDex.install(this);
    }
}
