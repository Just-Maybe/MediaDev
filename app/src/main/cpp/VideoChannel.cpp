//
// Created by Miracle on 2020/10/17.
//

#include "include/VideoChannel.h"

/**
 * 丢弃 原始数据(未压缩)
 * @param qq
 */
void dropAVFrame(queue<AVFrame *> &qq) {
    if (!qq.empty()) {
        AVFrame *avFrame = qq.front();
        BaseChannel::releaseAVFrame(&avFrame);//释放掉
        qq.pop();
    }
}

/**
 * 丢弃 压缩数据
 * @param qq
 */
void dropAVPacket(queue<AVPacket *> &qq) {
    if (!qq.empty()) {
        AVPacket *avPacket = qq.front();
        //这里需要判断当前删除的是否是关键帧，不能删除关键帧不然不能解码了
        if (avPacket->flags != AV_PKT_FLAG_KEY) {
            BaseChannel::releaseAVPacket(&avPacket);//释放掉
        }
        qq.pop();
    }
}

VideoChannel::VideoChannel(int stream_index, AVCodecContext *pContext, AVRational avRational,
                           int fpsValue,
                           JNICallback *jniCallback)
        : BaseChannel(stream_index, pContext, avRational, jniCallback) {
    this->fpsValue = fpsValue;
    this->frames.setDeleteVideoFrameCallback(dropAVFrame);
    this->packages.setDeleteVideoFrameCallback(dropAVPacket);
}

VideoChannel::~VideoChannel() {

}

void *taskVideoDecodeThread(void *arg) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(arg);
    videoChannel->video_decode();
    return 0;
}

void *taskVideoPlayerThread(void *arg) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(arg);
    videoChannel->video_player();
    return 0;
}

void VideoChannel::start() {
    isPlaying = 1;

    //存放未解码的队列开始工作
    packages.setFlat(1);
    //存放解码后的队列开始工作
    frames.setFlat(1);


    //1.创建解码线程
    pthread_create(&pid_video_decode, 0, taskVideoDecodeThread, this);
    //2.创建播放线程
    pthread_create(&pid_video_player, 0, taskVideoPlayerThread, this);
}

void VideoChannel::stop() {
    isStop = true;
    if (javaCallHelper) {
        javaCallHelper = 0;
    }
}

/**
 * 视频解码
 */
void VideoChannel::video_decode() {
    AVPacket *avPacket = 0;
    while (isPlaying) {

        if (isStop) {
            continue;
        }
        if (isPlaying && frames.queueSize() > 100) {

            av_usleep(10 * 1000);
            continue;
        }

        int ret = packages.pop(avPacket);

        //如果停止播放，跳出循环，出了循环，就要释放
        if (!isPlaying) {
            LOGD("停止播放");
            break;
        }
        if (!ret) continue;

        ret = avcodec_send_packet(pContext, avPacket);
        if (ret) {
            LOGD("未解码包发送到解码器失败 ，ret %d", ret);
            break;
        }
        //释放AVPacket
        releaseAVPacket(&avPacket);

        AVFrame *avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(pContext, avFrame);
        if (ret == AVERROR(EAGAIN)) {
            //重新取
            continue;
        } else if (ret != 0) {
            LOGD("获取解码后的数据失败 ret : %d", ret);
            releaseAVFrame(&avFrame);
            break;
        }
        frames.push(avFrame);
    }
}

/**
 * 视频播放
 */
void VideoChannel::video_player() {
    // 1. 原始数据YUV ---> rgba
    SwsContext *swsContext = sws_getContext(pContext->width, pContext->height, pContext->pix_fmt,
                                            pContext->width, pContext->height, AV_PIX_FMT_RGBA,
                                            SWS_BILINEAR, NULL, NULL, NULL);
    //2.给dst_data 申请内存
    uint8_t *dst_data[4];
    int dst_linesize[4];
    AVFrame *frame = 0;

    /**
     * pointers[4]：保存图像通道的地址。如果是RGB，则前三个指针分别指向R,G,B的内存地址。第四个指针保留不用
     *
     * linesizes[4]：保存图像每个通道的内存对齐的步长，即一行的对齐内存的宽度，此值大小等于图像宽度。
     *
     *  w: 要申请内存的图像宽度。
     *
     *  h:  要申请内存的图像高度。
     *
     *  pix_fmt: 要申请内存的图像的像素格式。
     *
     *  align: 用于内存对齐的值。
     *
     *  返回值：所申请的内存空间的总大小。如果是负值，表示申请失败。
     */
    int ret = av_image_alloc(dst_data, dst_linesize, pContext->width, pContext->height,
                             AV_PIX_FMT_RGBA, 1);

    if (ret < 0) {
        LOGE("Could not allocate source image \n");
        return;
    }
    //3. YUV -> rgba 格式转换  一帧一帧转换
    while (isPlaying) {

        if (isStop) {
            continue;
        }
        int ret = frames.pop(frame);

        //如果停止播放，跳出循环，需要释放
        if (!isPlaying) {
            break;
        }

        if (!ret) {
            continue;
        }
        //真正转换的函数，dst_data是rgba 格式数据
        sws_scale(swsContext, frame->data, frame->linesize, 0, pContext->height, dst_data,
                  dst_linesize);

        //视频向音频时间戳对齐 --->控制视频播放速度
        //在视频渲染之前，根据fps 来控制视频帧
        //frame->repeat_pict = 当解码时，这张图片需要延迟多久显示
        double extra_delay = frame->repeat_pict;
        //根据fps得到延迟时间
        double base_delay = 1 / this->fpsValue;
        //得到当前帧的延迟时间
        double result_delay = extra_delay + base_delay;

        //拿到视频播放的时间基
        this->video_time = frame->best_effort_timestamp * av_q2d(this->base_time);

        //拿到音频播放的时间基
        double audioTime = audioChannel->audio_time;

        //计算音和视频的差值
        double av_time_diff = video_time - audioTime;

        LOGE("av_time_diff init audioTime :%f, video :%f", audioTime, video_time);
        //说明:
        //video_time > audioTime 说明视频快，音频慢，等待音频
        //video_time < audioTime 说明视频慢，音屏快，需要追赶音频，丢弃掉冗余的视频包也就是丢帧
        if (av_time_diff > 0) {
            if (av_time_diff > 1) {
                av_usleep(result_delay * 2 * 1000000);
                LOGE("av_time_diff >  1 睡眠:%f", (result_delay * 2) * 1000000);
            } else {
                av_usleep((av_time_diff + result_delay) * 1000000);
                LOGE("av_time_diff < 1 睡眠:%d", (av_time_diff + result_delay) * 1000000);
            }
        } else if (av_time_diff < 0) {
            frames.deleteVideoFrame();
            LOGE("av_time_diff <0  睡眠:%s，丢包:%f ，剩余包: %d", "不睡眠", av_time_diff, frames.queueSize());
            continue;
        } else {

        }

        if (javaCallHelper && !audioChannel) {
            javaCallHelper->onProgress(THREAD_CHILD, video_time);
        }

        //开始渲染，显示屏幕上
        //渲染一帧图像
        if (renderCallback && pContext) {
            renderCallback(dst_data[0], pContext->width, pContext->height, dst_linesize[0]);
        }
        releaseAVFrame(&frame);
    }
    releaseAVFrame(&frame);
    isPlaying = 0;
    av_freep(&dst_data[0]);
    sws_freeContext(swsContext);
}

void VideoChannel::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}

void VideoChannel::setAudioChannel(AudioChannel *audioChannel) {

}

void VideoChannel::release() {
    LOGE("av_time_diff release 睡眠 size :%d", frames.queueSize());
    isPlaying = false;
    stop();
    if (frames.queueSize() > 0) {
        frames.clearQueue();
    }
    if (packages.queueSize() > 0) {
        packages.clearQueue();
    }
    if (renderCallback) {
        renderCallback = 0;
    }
    LOGE("av_time_diff release 睡眠 size :%d", frames.queueSize());
}

void VideoChannel::restart() {
    isStop = false;
}





