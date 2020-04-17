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

    void discardAll(void (*discardCallback)(T));

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

template <class T>
BlockRecyclerQueue<T>::BlockRecyclerQueue(int size) {
    this->size = size;
    queueLock = new unique_lock<mutex>(queueMu);
    queueLock->unlock();

    usedQueueLock = new unique_lock<mutex>(usedQueueMu);
    usedQueueLock->unlock();

}

template <class T>
BlockRecyclerQueue<T>::~BlockRecyclerQueue() {
    if(queueLock != NULL)
    {
        queueLock->unlock();
        delete(queueLock);
    }

    if(usedQueueLock != NULL)
    {
        usedQueueLock->unlock();
        delete(usedQueueLock);
    }
}

template <class T>
T BlockRecyclerQueue<T>::get(bool wait) {
    queueLock->lock();
    if(wait)
    {
        while(queue.size() == 0)
        {
            notEmptySignal.wait(*queueLock);
        }
    }

    T element = NULL;
    if(queue.size() != 0)
    {
        element = queue.front();
        queue.pop_front();
    }
    queueLock->unlock();
    notFullSignal.notify_all();
    return element;
}

template <class T>
void BlockRecyclerQueue<T>::put(T t, bool wait) {
    queueLock->lock();
    if(this->size == -1 || !wait)
    {
        queue.push_back(t);
    } else
    {
        while(queue.size() >= this->size)
        {
            notFullSignal.wait(*queueLock);
        }
        queue.push_back(t);
    }
    queueLock->unlock();
    notEmptySignal.notify_all();
}

template <class T>
int BlockRecyclerQueue<T>::getSize() {
    return this->size;
}

template <class T>
T BlockRecyclerQueue<T>::getUsed() {
    T element = NULL;
    usedQueueLock->lock();
    if(usedQueue.size() != 0)
    {
        element = usedQueue.front();
        usedQueue.pop_front();
    }
    usedQueueLock->unlock();
    return element;
}

template <class T>
void BlockRecyclerQueue<T>::putToUsed(T t) {

    usedQueueLock->lock();
    usedQueue.push_back(t);
    usedQueueLock->unlock();
}

template <class T>
void BlockRecyclerQueue<T>::notifyWaitGet() {
    this->notEmptySignal.notify_all();
}

template <class T>
void BlockRecyclerQueue<T>::notifyWaitPut() {
    this->notFullSignal.notify_all();
}

template <class T>
void BlockRecyclerQueue<T>::discardAll(void (*discardCallback)(T)) {
    queueLock->lock();
    usedQueueLock->lock();
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
    usedQueueLock->unlock();
    queueLock->unlock();

}

#endif //ANDROID_VIDEOPLAYER_BLOCKRECYCLERQUEUE_H
