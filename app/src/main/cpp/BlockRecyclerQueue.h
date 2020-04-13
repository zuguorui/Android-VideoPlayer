//
// Created by 祖国瑞 on 2020-04-12.
//

#ifndef ANDROID_VIDEOPLAYER_BLOCKRECYCLERQUEUE_H
#define ANDROID_VIDEOPLAYER_BLOCKRECYCLERQUEUE_H

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <thread>
#include <list>

using namespace std;
// TODO: add notify lockers to avoid dead waiting. Add get element immediately interfaces.
template <class T>
class BlockRecyclerQueue {
public:
    BlockRecyclerQueue(int size = -1);
    ~BlockRecyclerQueue();
    int getSize();


    void put(T t);

    T get();

    void putToUsed(T t);

    T getUsed();

private:
    int size = 0;
    mutex queueMu;
    mutex usedQueueMu;
    condition_variable notFullSignal;
    condition_variable notEmptySignal;
    list<T> queue;
    list<T> usedQueue;

    unique_lock<mutex> *queueLock = NULL;
    unique_lock<mutex> *usedQueueLock = NULL;



};



#endif //ANDROID_VIDEOPLAYER_BLOCKRECYCLERQUEUE_H
