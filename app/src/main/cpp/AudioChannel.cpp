//
// Created by Miracle on 2020/10/17.
//

#include "include/AudioChannel.h"

AudioChannel::AudioChannel(int stream_index, AVCodecContext *pContext, AVRational avRational,
                           JNICallback *jniCallback)
        : BaseChannel(stream_index, pContext, avRational, jniCallback) {
    //初始化缓冲区 out_buffers
    //动态计算
    // 或者写死 out_buffer_size = 44100 * 2 * 2

    //双通道
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    //采样大小
    out_sample_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    //采样率
    out_sample_rate = AUDIO_SAMPLE_RATE;
    //计算缓冲大小
    out_buffers_size = out_sample_rate * out_sample_size * out_channels;
    //分配缓冲内存
    out_buffers = static_cast<uint8_t *>(av_mallocz(out_buffers_size));
    //根据通道数、采样大小、采样率，返回分配的转换上下文SwrContext 指针
    swr_ctx = swr_alloc_set_opts(0, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, out_sample_rate,
                                 pContext->channel_layout, pContext->sample_fmt,
                                 pContext->sample_rate, 0, 0);
    //初始化上下文
    swr_init(swr_ctx);
    LOGD("AudioChannel 构造函数")
}

void *thread_audio_decode(void *arg) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(arg);
    audioChannel->audio_decode();//真正音频解码的方法
    return 0;
}

void *thread_audio_player(void *arg) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(arg);
    audioChannel->audio_player(); //真正播放的方法
    return 0;
}

/**
 * 开始播放
 */
void AudioChannel::start() {
    //设置正在播放的标志
    isPlaying = true;
    //存放未解码数据的列队开始工作
    packages.setFlat(1);
    //存放解码后数据的列队开始工作
    frames.setFlat(1);

    pthread_create(&pid_audio_decode, 0, thread_audio_decode, this);

    pthread_create(&pid_audio_player, 0, thread_audio_player, this);
}

/**
 * 音频解码
 */
void AudioChannel::audio_decode() {
    AVPacket *avPacket = 0;

    while (isPlaying) {
        if (isStop) {
            continue;
        }
        //这里有一个 bug，如果生产快，消费慢，就会造成队列数据过多容易造成 OOM,
        //解决办法：控制队列大小
        if (isPlaying && frames.queueSize() > 100) {
            av_usleep(10 * 1000);
        }
        int ret = packages.pop(avPacket);
        if (!ret) continue;
        if (!isPlaying) break;
        //@return 0 on success, otherwise negative error code:
        ret = avcodec_send_packet(pContext, avPacket);
        if (ret) break; //发送失败

        releaseAVPacket(&avPacket); //发送成功，释放AVPacket

        //拿到解码后的原始数据
        AVFrame *avFrame = av_frame_alloc();
        //0:success, a frame was returned
        ret = avcodec_receive_frame(pContext, avFrame);
        if (ret == AVERROR(EAGAIN)) {
            continue;//获取失败，继续下次任务
        } else if (ret != 0) { //解码失败
            releaseAVFrame(&avFrame);
            break;
        }
        //将获取到的原始数据放入队列中，也就是解码后的原始数据
        frames.push(avFrame);
    }
    if (avPacket) {
        releaseAVPacket(&avPacket);
    }
}

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(context);
    //获取PCM音频数据
    int pcmSize = audioChannel->getPCM();
    (*bq)->Enqueue(bq, audioChannel->out_buffers, pcmSize);
}

/**
 * 使用OpenSL ES 渲染PCM数据 播放音频
 */
void AudioChannel::audio_player() {
    //TODO  1、创建引擎并获取引擎接口
    // 1.1创建引擎对象：SLObjectItf engineObject
    SLresult result = slCreateEngine(&engineObject, 0, NULL, 0, 0, NULL);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    //1.2 初始化引擎
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 1.3 获取引擎接口SLEngineItf engineInterface
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    //TODO 2 设置混音器
    // 2.1 创建混音器：SLObjectItf outputMixObject
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0, 0, 0);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 2.2初始化 混音器
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    //  不启用混响可以不用获取混音器接口
    //  获得混音器接口
    //  result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
    //                                         &outputMixEnvironmentalReverb);
    //  if (SL_RESULT_SUCCESS == result) {
    //  设置混响 ： 默认。
    //  SL_I3DL2_ENVIRONMENT_PRESET_ROOM: 室内
    //  SL_I3DL2_ENVIRONMENT_PRESET_AUDITORIUM : 礼堂 等
    //  const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
    //  (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
    //       outputMixEnvironmentalReverb, &settings);
    //  }

    //TODO 3.创建播放器
    // 3.1 配置输入声音信息
    // 创建buffer缓冲类型的队列 2 个队列
    SLDataLocator_AndroidBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDBUFFERQUEUE, 2};

    /**
     * pcm数据
     * SL_DATAFORMAT_PCM :数据格式pcm格式
     * 2：双声道
     * SL_SAMPLINGRATE_44_1:采样率为44100
     * SL_PCMSAMPLEFORMAT_FIXED_16 : 采样格式 16bit (16位)(2个字节)
     * SL_PCMSAMPLEFORMAT_FIXED_16 : 采样格式 16bit (16位)(2个字节)
     * SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT :左右声道(双声道)
     * SL_BYTEORDER_LITTLEENDIAN: 字节顺序: 小端模式
     */
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
                                   SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                                   SL_BYTEORDER_LITTLEENDIAN};
    //数据源 将上述配置信息放到数据源中
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // 3.2 配置音轨(输出)
    // 设置混音器
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    //需要的接口 操作队列的接口
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    //3.3创建播放器
    result = (*engineInterface)->CreateAudioPlayer(engineInterface, &bqPlayerObject, &audioSrc,
                                                   &audioSnk, 1,
                                                   ids, req);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    //3.4 初始化播放器：SLObjectItf bqPlayerObject
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    //3.5 获取播放器接口：SlPlayItf bqPlayerPlay
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    //TODO 4. 设置播放器回调函数
    // 4.1 获取播放器队列接口：SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);

    //4.2设置回调 void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq,void* context)
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);

    //TODO 5.设置播放状态
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);

    //TODO 6.手动激活回调函数
    bqPlayerCallback(bqPlayerBufferQueue, this);
}

int AudioChannel::getPCM() {
    return 0;
}

void AudioChannel::stop() {
    isStop = true;
    if (javaCallHelper) {
        javaCallHelper = 0;
    }
}

void AudioChannel::release() {

}

void AudioChannel::restart() {
    isStop = false;
}

AudioChannel::~AudioChannel() {

}
