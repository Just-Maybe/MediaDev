package com.example.mediadev.player;

/**
 * Created by Miracle on 2020/10/14
 * Email: zhaoqirong96@gmail.com
 * Describe:
 */
public class PlayerManger {
    private static PlayerManger mPlayerManger=null;

    static {
        System.loadLibrary("avfilter");
        System.loadLibrary("postproc");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("swscale");
        System.loadLibrary("swresample");
        System.loadLibrary("avcodec");
    }

    private PlayerManger(){

    }
    public static PlayerManger getInstance(){
        if(mPlayerManger==null){
            synchronized (PlayerManger.class){
                if(mPlayerManger==null){
                    mPlayerManger = new PlayerManger();
                }
            }
        }
        return mPlayerManger;
    }

}
