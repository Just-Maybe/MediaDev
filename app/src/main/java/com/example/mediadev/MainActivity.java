package com.example.mediadev;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.example.mediadev.callback.OnPreparedListener;
import com.example.mediadev.callback.OnProgressListener;
import com.example.mediadev.player.PlayerManger;

import java.util.ArrayList;
import java.util.function.LongUnaryOperator;

public class MainActivity extends AppCompatActivity implements View.OnClickListener, OnProgressListener {
    public static final String TAG = MainActivity.class.getSimpleName();

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private PlayerManger playerManger;
    private SurfaceView surfaceView;
    private TextView tvStart, tvPause, tvRestart, tvRelease;
    private SeekBar seekBar;

    private boolean isTouch;
    private boolean isSeek;
    private int progress;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        tvStart = findViewById(R.id.btn_start);
        tvPause = findViewById(R.id.btn_pause);
        tvRestart = findViewById(R.id.btn_restart);
        tvRelease = findViewById(R.id.btn_release);
        seekBar = findViewById(R.id.seek_bar);
        surfaceView = findViewById(R.id.surface_view);
        playerManger = PlayerManger.getInstance();
        playerManger.setSurfaceView(surfaceView);
        tvStart.setOnClickListener(this);
        tvPause.setOnClickListener(this);
        tvRestart.setOnClickListener(this);
        tvRelease.setOnClickListener(this);
        playerManger.setmOnProgressListener(this);
        initListener();
    }

    private void initListener() {
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                isTouch = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                isTouch = false;
                isSeek = true;
                progress = (int) (playerManger.getDurationNative() * seekBar.getProgress() / 100);
                playerManger.seekNative(progress);
            }
        });
    }

    private void initPlayer() {
        String inputPath = Environment.getExternalStorageDirectory() + "/input.mp4";
        playerManger.prepareNative(inputPath);
        playerManger.setmOnPreparedListener(new OnPreparedListener() {
            @Override
            public void onPrepared() {
                Log.d(TAG, "onPrepared: ");
                playerManger.startNative();
            }

            @Override
            public void onError(String error) {
            }
        });
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    @Override
    protected void onPause() {
        super.onPause();
        playerManger.stopNative();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_start:
                initPlayer();
                break;
            case R.id.btn_pause:
                playerManger.stopNative();
                break;
            case R.id.btn_restart:
                playerManger.restartNative();
                break;
            case R.id.btn_release:
                playerManger.releaseNative();
                break;
        }
    }

    @Override
    public void onProgress(final int progress) {
        if (!isTouch) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if (playerManger == null) return;
                    int duration = (int) playerManger.getDurationNative();
                    if (duration != 0) {
                        if (isSeek) {
                            isSeek = false;
                            return;
                        }
                        //更新进度 计算比例
                        seekBar.setProgress(progress * 100 / duration);
                    }
                }
            });
        }
    }
}
