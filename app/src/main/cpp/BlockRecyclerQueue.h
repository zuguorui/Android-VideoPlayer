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
    // if size == -1, then we don't limit the size of data queue, and all the put option will not wait.
    BlockRecyclerQueue(int size = -1);
    ~BlockRecyclerQueue();
    int getSize();

    // put a element, if wait = true, put option will wait until the length of data queue is less than specified size.
    void put(T t, bool wait = true);

    // get a element, if wait = true, it will wait until the data queue is not empty. If wait = false, it will return NULL if the data queue is empty.
    // It will still return NULL even wait = true, in this case, it must be someone call notifyWaitGet() but the data queue is still empty.
    T get(bool wait = true);

    void putToUsed(T t);

    T getUsed();

    // notify all the put option to not wait. This will cause put option succeed immediately
    void notifyWaitPut();

    // notify all the get option to return immediately. if data queue is still empty, get option will return a NULL.
    void notifyWaitGet();





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
