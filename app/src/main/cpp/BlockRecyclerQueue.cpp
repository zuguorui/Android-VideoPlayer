//
// Created by 祖国瑞 on 2020-04-12.
//

#include "BlockRecyclerQueue.h"

using namespace std;

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

    T *element = NULL;
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



