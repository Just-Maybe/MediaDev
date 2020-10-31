package com.example.mediadev.mooc;

import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.Button;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.example.mediadev.R;

/**
 * Created by Miracle on 2020/10/19
 * Email: zhaoqirong96@gmail.com
 * Describe:
 */
public class MoocActivity extends AppCompatActivity implements View.OnClickListener {
    static {
        System.loadLibrary("native-lib");
    }

    private Button btnTransform;
    private Button btnCutVideo;
    private String inputPath;
    private String outputPath;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mooc);
        btnTransform = findViewById(R.id.btn_transform);
        btnCutVideo = findViewById(R.id.btn_cut_video);
        btnTransform.setOnClickListener(this);
        btnCutVideo.setOnClickListener(this);
        inputPath = Environment.getExternalStorageDirectory() + "/input.mp4";
        outputPath = Environment.getExternalStorageDirectory() + "/output.mp4";
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_transform:
                mp4ToFlv(inputPath, outputPath);
                break;
            case R.id.btn_cut_video:
                double startTime = 5;
                double endTime = 20;
                cutVideo(inputPath, outputPath, startTime, endTime);
                break;
        }
    }

    public native void mp4ToFlv(String path, String outputPath);

    private native void cutVideo(String inputPath, String outputPath, double startTime, double endTime);

}
