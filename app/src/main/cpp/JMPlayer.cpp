//
// Created by Miracle on 2020/10/14.
//

#include <jni.h>
#include "include/JMPlayer.h"

void *customTaskStartThread(void *arg) {
    LOGD("customTaskStartThread");
    JMPlayer *player = static_cast<JMPlayer *>(arg);
    player->start_();
    return 0;
}

void *customTaskPrepareThread(void *arg) {
    LOGD("customTaskPrepareThread");
    JMPlayer *player = static_cast<JMPlayer *>(arg);
    player->prepare_();
    return 0;//这里是坑一定要记得 return;
}

JMPlayer::JMPlayer(const char *data_source, JNICallback *callback) {
    // 这里有坑，这里赋值之后，不能给其他地方用，因为被释放了，变成了悬空指针
    // this->data_source = data_source;
    //解决上面的坑，自己 copy 才行 +1 在 C++ 中有一个 \n
    this->data_source = new char[strlen(data_source) + 1];
    strcpy(this->data_source, data_source);
    this->pCallback = callback;
    duration = 0;
    pthread_mutex_init(&seekMutex, 0);
}

void JMPlayer::prepare() {
    //解析流媒体，通过 ffmpeg  API 来解析 dataSource
    //这里需要异步，由于这个函数从 Java 主线程调用的，所以这里需要创建一个异步线程
    pthread_create(&pid_thread, 0, customTaskPrepareThread, this);
}


void JMPlayer::prepare_() {
    LOGD("第一步：打开流媒体地址");
    formatContext = avformat_alloc_context();

    AVDictionary *dictinonary = 0;
    av_dict_set(&dictinonary, "timeout", "5000000", 0);

    /**
     * @param AVFormatContext: 传入一个 format 上下文是一个二级指针
     * @param const char *url: 播放源
     * @param ff_const59 AVInputFormat *fmt: 输入的封住格式，一般让 ffmpeg 自己去检测，所以给了一个 0
     * @param AVDictionary **options: 字典参数
     */
    int result = avformat_open_input(&formatContext, data_source, 0, &dictinonary);

    LOGD("avformat_open_input--> %d,%s", result, data_source);
    av_dict_free(&dictinonary);

    if (result) {  //0 on success
        if (pCallback) {
            pCallback->onErrorAction(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);
        }
        return;
    }
    LOGD("第二步 查找媒体中的音视频流信息");
    int ret = avformat_find_stream_info(formatContext, NULL);
    if (ret < 0) {
        if (pCallback) {
            pCallback->onErrorAction(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
            return;
        }
    }
    LOGD("第三步 遍历流信息，查找音频流，视频流");
    for (int stream_index = 0; stream_index < formatContext->nb_streams; ++stream_index) {
        LOGD("第四步 获取流信息")
        AVStream *stream = formatContext->streams[stream_index];

        LOGD("从 stream 流中获取解码这段流的参数信息，区分音频，视频");
        AVCodecParameters *codecpar = stream->codecpar;

        LOGD("第六步 通过流的编解码参数中的编码ID，获取当前流的解码器")
        AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
        if (!codec) {
            pCallback->onErrorAction(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            return;
        }
        LOGD("第七步 通过拿到解码器，获取解码器上下文");
        codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            pCallback->onErrorAction(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            return;
        }
        LOGD("第八步 给解码器上下文设置参数")
        result = avcodec_parameters_to_context(codecContext, codecpar);
        if (result < 0) {
            pCallback->onErrorAction(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            return;
        }
        LOGD("第九步 打开解码器")
        result = avcodec_open2(codecContext, codec, NULL);
        if (result < 0) {
            pCallback->onErrorAction(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            return;
        }

        //媒体流 获取时间基
        AVRational baseTime = codecContext->time_base;

        LOGD("第十步 从编码器参数中获取流类型 codec_type");
        if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioChannel = new AudioChannel(stream_index, codecContext, baseTime, pCallback);
        } else if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            //获取视频帧 fps
            AVRational frame_rate = stream->avg_frame_rate;
            int fps_value = av_q2d(frame_rate);
            videoChannel = new VideoChannel(stream_index, codecContext, baseTime, fps_value,
                                            pCallback);
            videoChannel->setRenderCallback(renderCallback);
        }
    }

    LOGD("第十一步 如果流中没有音视频数据");
    if (!audioChannel && !videoChannel) {
        pCallback->onErrorAction(THREAD_CHILD, FFMPEG_NOMEDIA);
        return;
    }
    LOGD("第十二步 有视频 或者 有音频")
    if (pCallback) {
        pCallback->onPrepared(THREAD_CHILD);
    }
}

void JMPlayer::start() {
    //声明是否播放的标记
    isPlaying = 1;

    if (videoChannel) {
        videoChannel->setAudioChannel(audioChannel);
        videoChannel->start();
    }
    if (audioChannel) {
        audioChannel->start();
    }


    pthread_create(&pid_thread, 0, customTaskStartThread, this);
}

/**
 * 读包、未解码的音视频数据放到队列中
 */
void JMPlayer::start_() {
    //循环 读音视频包
    while (isPlaying) {
        if (!formatContext) {
            LOGE("formatContext : 已经被释放了");
            return;
        }
        if (isStop) {
            continue;
        }
        LOGD("start_");
        //内存泄漏点 1，解决方法 : 控制队列大小
        if (videoChannel && videoChannel->packages.queueSize() > 100) {
            //休眠 等待队列中的数据被消费
            av_usleep(10 * 1000);
            continue;
        }

        //内存泄漏点 2，解决方法 : 控制队列大小
        if (audioChannel && audioChannel->packages.queueSize() > 100) {
            av_usleep(10 * 1000);
            continue;
        }
        //未解码数据，可能是音频或视频数据
        AVPacket *packet = av_packet_alloc();

        int ret = av_read_frame(formatContext, packet);

        if (!ret) {
            if (videoChannel && videoChannel->stream_index == packet->stream_index) { //视频包
                LOGD("steam_index 视频 %s ", "push");
                videoChannel->packages.push(packet);
            } else if (audioChannel && audioChannel->stream_index == packet->stream_index) { //音频包
                LOGD("steam_index 音频 %s", "push");
                audioChannel->packages.push(packet);
            }
        } else if (ret == AVERROR_EOF) { //读取完毕
            LOGE("steam_index 拆包完成 读取完成了");
            isPlaying = 0;
            stop();
            release();
            break;
        } else {
            LOGE("steam_index 拆包 读取失败");
            break;
        }
    }//end while
    //释放工作
    isPlaying = 0;
    isStop = false;
    videoChannel->stop();
    audioChannel->stop();
}

void JMPlayer::stop() {
    LOGD("Player 执行停止")
    isStop = true;

    if (videoChannel) {
        videoChannel->stop();
    }
    if (audioChannel) {
        audioChannel->stop();
    }
}

void JMPlayer::release() {
    LOGD("Player 执行销毁");
    isPlaying = false;
    stop();
    if (videoChannel) {
        videoChannel->release();
    }
    if (audioChannel) {
        audioChannel->release();
    }
    if (codecContext) {
        avcodec_free_context(&codecContext);
        codecContext = 0;
    }
    if (formatContext) {
        avformat_free_context(formatContext);
        formatContext = 0;
    }
    duration = 0;
}

void JMPlayer::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}


