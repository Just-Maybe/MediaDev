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

void VideoChannel::start() {

}

void VideoChannel::stop() {

}

void VideoChannel::video_decode() {

}

void VideoChannel::video_player() {

}

void VideoChannel::setRenderCallback(RenderCallback renderCallback) {

}

void VideoChannel::setAudioChannel(AudioChannel *audioChannel) {

}

void VideoChannel::release() {

}

void VideoChannel::restart() {

}





