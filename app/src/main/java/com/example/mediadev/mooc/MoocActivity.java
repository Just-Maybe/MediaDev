package com.example.mediadev.mooc;

import android.os.Bundle;
import android.os.Environment;
import android.widget.TextView;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.example.mediadev.R;

/**
 * Created by Miracle on 2020/10/19
 * Email: zhaoqirong96@gmail.com
 * Describe:
 */
public class MoocActivity extends AppCompatActivity {
    static {
        System.loadLibrary("native-lib");
    }

    private TextView tvMeta;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mooc);
        tvMeta = findViewById(R.id.tv_meta);
        String path = Environment.getExternalStorageDirectory()+"/input.mp4";
        tvMeta.setText(getVideoInfo(path));
    }

    public native String getVideoInfo(String path);
}
