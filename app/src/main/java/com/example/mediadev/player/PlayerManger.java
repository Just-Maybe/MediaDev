package com.example.mediadev.player;

import android.view.Surface;

import com.example.mediadev.Constants;
import com.example.mediadev.callback.OnPreparedListener;
import com.example.mediadev.callback.OnProgressListener;

/**
 * Created by Miracle on 2020/10/14
 * Email: zhaoqirong96@gmail.com
 * Describe:
 */
public class PlayerManger {

    private static PlayerManger mPlayerManger = null;

    private OnPreparedListener mOnPreparedListener;
    private OnProgressListener mOnProgressListener;

    static {
        System.loadLibrary("avfilter");
        System.loadLibrary("postproc");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("swscale");
        System.loadLibrary("swresample");
        System.loadLibrary("avcodec");
        System.loadLibrary("native-lib");
    }

    private PlayerManger() {

    }

    public static PlayerManger getInstance() {
        if (mPlayerManger == null) {
            synchronized (PlayerManger.class) {
                if (mPlayerManger == null) {
                    mPlayerManger = new PlayerManger();
                }
            }
        }
        return mPlayerManger;
    }

    public native String getFFmpegVersion();

    public native void setSurfaceNative(Surface surface);

    public native void prepareNative(String mDataSource);

    public native void startNative();

    public native void restartNative();

    public native void stopNative();

    public native void releaseNative();

    public native boolean isPlayerNative();

    /**
     * JNI 反射调用的方法
     */
    public void onPrepared() {
        if (null != mOnPreparedListener) {
            mOnPreparedListener.onPrepared();
        }
    }

    /**
     * JNI 回调 进度给java
     *
     * @param progress
     */
    public void onProgress(int progress) {
        if (null != mOnProgressListener) {
            mOnProgressListener.onProgress(progress);
        }
    }

    public void onError(int errorCode) {
        if (null == mOnPreparedListener) return;
        String errorText = null;
        switch (errorCode) {
            case Constants.IMessageType.FFMPEG_ALLOC_CODEC_CONTEXT_FAIL:
                errorText = "无法根据解码器创建上下文";
                break;
            case Constants.IMessageType.FFMPEG_CAN_NOT_FIND_STREAMS:
                errorText = "找不到媒体流信息";
                break;
            case Constants.IMessageType.FFMPEG_CAN_NOT_OPEN_URL:
                errorText = "打不开媒体数据源";
                break;
            case Constants.IMessageType.FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL:
                errorText = "根据流信息 配置上下文参数失败";
                break;
            case Constants.IMessageType.FFMPEG_FIND_DECODER_FAIL:
                errorText = "找不到解码器";
                break;
            case Constants.IMessageType.FFMPEG_NOMEDIA:
                errorText = "没有音视频";
                break;
            case Constants.IMessageType.FFMPEG_READ_PACKETS_FAIL:
                errorText = "读取媒体数据包失败";
                break;
            default:
                errorText = "未知错误，自己去检测你的垃圾代码...";
                break;
        }
        this.mOnPreparedListener.onError(errorText);
    }
}
