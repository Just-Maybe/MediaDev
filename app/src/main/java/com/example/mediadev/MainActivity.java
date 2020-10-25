package com.example.mediadev;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.example.mediadev.callback.OnPreparedListener;
import com.example.mediadev.player.PlayerManger;

import java.util.function.LongUnaryOperator;

public class MainActivity extends AppCompatActivity {
    public static final String TAG = MainActivity.class.getSimpleName();

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private PlayerManger playerManger;
    private SurfaceView surfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        surfaceView = findViewById(R.id.surface_view);
        playerManger = PlayerManger.getInstance();
        playerManger.setSurfaceView(surfaceView);
        tv.setText(stringFromJNI());
        tv.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                initPlayer();
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


}
