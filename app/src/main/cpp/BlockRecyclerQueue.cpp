//
// Created by 祖国瑞 on 2020-04-12.
//

#include "BlockRecyclerQueue.h"

using namespace std;

BlockRecyclerQueue::BlockRecyclerQueue(int size) {
    this->size = size;
    queueLock = new unique_lock(queueMu);
    queueLock->unlock();

    usedQueueLock = new unique_lock(usedQueueMu);
    usedQueueLock->unlock();

}

BlockRecyclerQueue::~BlockRecyclerQueue() {
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
T* BlockRecyclerQueue::get() {
    queueLock->lock();
    while(queue.size() == 0)
    {
        notEmptySignal.wait(*queueLock);
    }

    T *element = queue.front();
    queue.pop_front();
    queueLock->unlock();
    return element;
}

template <class T>
void BlockRecyclerQueue::put(T *t) {
    queueLock->lock();
    while(queue.size() >= this->size)
    {
        notFullSignal.wait(*queueLock);
    }
    queue.push_back(t);
    queueLock->unlock();
}

int BlockRecyclerQueue::getSize() {
    return this->size;
}

template <class T>
T* BlockRecyclerQueue::getUsed() {
    T* element = NULL;
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
void BlockRecyclerQueue::putToUsed(T* t) {
    usedQueueLock->lock();
    usedQueue.push_back(t);
    usedQueueLock->unlock();
}