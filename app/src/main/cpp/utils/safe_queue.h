//
// Created by Miracle on 2020/10/17.
//

#ifndef MEDIADEV_SAFEQUEUE_H
#define MEDIADEV_SAFEQUEUE_H

#endif //MEDIADEV_SAFEQUEUE_H

#include <queue>
#include <pthread.h>

using namespace std;

template<typename T>

class SafeQueue {
    //  C语言的函数指针
    typedef void (*ReleaseCallback)(T *);

    //删除视频帧的回调
    typedef void (*DeleteFrameCallback)(queue<T> &);

private:
    queue<T> q;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int flag; //标记队列释放工作[true=工作状态，false=非工作状态]
    ReleaseCallback releaseCallback;
    DeleteFrameCallback deleteFrameCallback;

public:
    SafeQueue() {
        pthread_mutex_init(&mutex, 0);
        pthread_cond_init(&cond, 0);
    }

    ~SafeQueue() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    /**
     * 入队
     */
    void push(T value) {
        pthread_mutex_lock(&mutex);  //线程安全，上锁
        if (flag) {
            q.push(value);
            pthread_cond_signal(&cond);
        } else {
            // 释放操作（不知道value是什么类型，int， 对象 ，等等，怎么办？）
            // 我们不知道，交给用户来处理
            if (releaseCallback) {
                releaseCallback(&value);
            }
        }

        pthread_mutex_unlock(&mutex); //解锁 ，让其他线程访问
    }

    int pop(T &t) {
        int ret = 0;
        pthread_mutex_lock(&mutex);//线程安全，上锁

        while (flag && q.empty()) {
            //如果工作状态 而且队列中没有数据，就等待
            pthread_cond_wait(&cond, &mutex);
        }
        if (!q.empty()) {
            t = q.front();
            q.pop();
            ret = 1;
        }

        pthread_mutex_unlock(&mutex);//解锁 ，让其他线程访问
        return ret;
    }

    void setFlat(int flag) {
        pthread_mutex_lock(&mutex);

        this->flag = flag;
        pthread_cond_signal(&cond);//通知

        pthread_mutex_unlock(&mutex);
    }

    int isEmpty() {
        return q.empty();
    }

    void clearQueue() {
        pthread_mutex_lock(&mutex);

        unsigned int size = q.size();
        for (int i = 0; i < size; ++i) {
            T value = q.front();
            if (releaseCallback) {
                releaseCallback(&value);
            }
            q.pop();
        }

        pthread_mutex_unlock(&mutex);
    }

    void setReleaseCallback(ReleaseCallback callback) {
        this->releaseCallback = callback;
    }

    //设置删除视频帧的回调
    void setDeleteVideoFrameCallback(DeleteFrameCallback deleteFrameCallback) {
        this->delFrameCallback = deleteFrameCallback;
    }

    void deleteVideoFrame() {
        pthread_mutex_lock(&mutex);

        //不做任何实现，需要的才去实现
        deleteFrameCallback(q);

        pthread_mutex_unlock(&mutex);
    }
};