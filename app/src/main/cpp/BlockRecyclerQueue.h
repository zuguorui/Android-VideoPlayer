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

template <class T>
class BlockRecyclerQueue {
public:
    // if size == -1, then we don't limit the size of data queue, and all the put option will not wait.
    BlockRecyclerQueue(int capacity = -1);
    ~BlockRecyclerQueue();
    int getCapacity();

    int getSize();

    // put a element, if wait = true, put option will wait until the length of data queue is less than specified size.
    void put(T t, bool wait = true);

    // get a element, if wait = true, it will wait until the data queue is not empty. If wait = false, it will return NULL if the data queue is empty.
    // It will still return NULL even wait = true, in this case, it must be someone call notifyWaitGet() but the data queue is still empty.
    T get(bool wait = true);

    void putToUsed(T t);

    T getUsed();

    void discardAll(void (*discardCallback)(T));

    // notify all the put option to not wait. This will cause put option succeed immediately
    void notifyWaitPut();

    // notify all the get option to return immediately. if data queue is still empty, get option will return a NULL.
    void notifyWaitGet();





private:
    int capacity = 0;
    mutex queueMu;
    mutex usedQueueMu;
    condition_variable notFullSignal;
    condition_variable notEmptySignal;
    list<T> queue;
    list<T> usedQueue;

    bool allowNotifyPut = false;
    bool allowNotifyGet = false;


};

template <class T>
BlockRecyclerQueue<T>::BlockRecyclerQueue(int capacity) {
    this->capacity = capacity;
    allowNotifyPut = false;
    allowNotifyGet = false;
}

template <class T>
BlockRecyclerQueue<T>::~BlockRecyclerQueue() {

}

template <class T>
T BlockRecyclerQueue<T>::get(bool wait) {
    unique_lock<mutex> queueLock(queueMu);
    if(wait)
    {
        while(queue.size() == 0)
        {
            notEmptySignal.wait(queueLock);
            if(allowNotifyGet)
            {
                allowNotifyGet = false;
                break;
            }
        }
    }

    T element = NULL;
    if(queue.size() != 0)
    {
        element = queue.front();
        queue.pop_front();
    }
    notFullSignal.notify_all();
    queueLock.unlock();
    return element;
}

template <class T>
void BlockRecyclerQueue<T>::put(T t, bool wait) {
    unique_lock<mutex> queueLock(queueMu);
    if(this->capacity == -1 || !wait)
    {
        queue.push_back(t);
    } else
    {
        while(queue.size() >= this->capacity)
        {
            notFullSignal.wait(queueLock);
            if(allowNotifyPut)
            {
                allowNotifyPut = false;
                break;
            }
        }
        queue.push_back(t);
    }
    queueLock.unlock();
    notEmptySignal.notify_all();
}

template <class T>
int BlockRecyclerQueue<T>::getCapacity() {
    return this->capacity;
}

template <class T>
int BlockRecyclerQueue<T>::getSize() {
    unique_lock<mutex> queueLock(queueMu);
    int size = queue.size();
    queueLock.unlock();
    return size;
}

template <class T>
T BlockRecyclerQueue<T>::getUsed() {
    T element = NULL;
    unique_lock<mutex> usedQueueLock(usedQueueMu);
    if(usedQueue.size() != 0)
    {
        element = usedQueue.front();
        usedQueue.pop_front();
    }
    usedQueueLock.unlock();
    return element;
}

template <class T>
void BlockRecyclerQueue<T>::putToUsed(T t) {

    unique_lock<mutex> usedQueueLock(usedQueueMu);
    usedQueue.push_back(t);
    usedQueueLock.unlock();
}

template <class T>
void BlockRecyclerQueue<T>::notifyWaitGet() {
    allowNotifyGet = true;
    this->notEmptySignal.notify_all();
}

template <class T>
void BlockRecyclerQueue<T>::notifyWaitPut() {
    allowNotifyPut = true;
    this->notFullSignal.notify_all();
}

template <class T>
void BlockRecyclerQueue<T>::discardAll(void (*discardCallback)(T)) {
    unique_lock<mutex> queueLock(queueMu);
    unique_lock<mutex> usedQueueLock(usedQueueMu);
    while(queue.size() != 0)
    {
        T t = queue.front();
        queue.pop_front();
        if(discardCallback != NULL)
        {
            (*discardCallback)(t);
        }
        usedQueue.push_back(t);
    }
    notFullSignal.notify_all();
    usedQueueLock.unlock();
    queueLock.unlock();

}

#endif //ANDROID_VIDEOPLAYER_BLOCKRECYCLERQUEUE_H
