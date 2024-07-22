//
// Created by zu on 2024/7/21.
//

#ifndef ANDROID_VIDEOPLAYER_THREADSAFELIST_H
#define ANDROID_VIDEOPLAYER_THREADSAFELIST_H

#include <list>
#include <stdlib.h>
#include <stdint.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <memory>

#define TAG "ThreadSafeList"

// 安卓对泛型不允许分成h和cpp，所以源码都写在h文件里。
template<typename T>
class ThreadSafeList {
public:
    ThreadSafeList(ThreadSafeList &src) = delete;
    ThreadSafeList(ThreadSafeList && src) = delete;

    ThreadSafeList() {
        init(-1);
    }

    ThreadSafeList(int32_t capacity) {
        init(capacity);
    }

    ~ThreadSafeList() {
        while (!mDeque.empty()) {
            T t = mDeque.front();
            mDeque.pop_front();
            if constexpr (std::is_pointer<T>::value) {
                delete(t);
            }
        }
    }

    int32_t getCapacity() {
        return capacity;
    }

    int32_t getSize() {
        return mDeque.size();
    }

    /**
     * 队列是否已满。
     * */
    bool isFull() {
        std::unique_lock<std::mutex> lock(mu);
        return mDeque.size() >= capacity;
    }

    bool pushBack(const T& t, bool blocking = true) {
        return push(t, blocking, false);
    }

    void forcePushBack(const T& t) {
        forcePush(t, false);
    }

    std::optional<T> popBack(bool blocking = true) {
        return pop(blocking, false);
    }

    bool pushFront(const T& t, bool blocking = true) {
        return push(t, blocking, true);
    }

    void forcePushFront(const T& t) {
        forcePush(t, true);
    }

    std::optional<T> popFront(bool blocking = true) {
        return pop(blocking, true);
    }

    void setBlockPush(bool block) {
        blockPushFlag = block;
        if (!block) {
            notFull.notify_all();
        }
    }

    bool isBlockPush() {
        return blockPushFlag.load();
    }

    void setBlockPop(bool block) {
        blockPopFlag = block;
        if (!block) {
            notEmpty.notify_all();
        }
    }

    bool isBlockPop() {
        return blockPopFlag.load();
    }

    void clear() {
        std::unique_lock<std::mutex> lock(mu);
        if (mDeque.empty()) {
            return;
        }
        while (!mDeque.empty()) {
            T t = mDeque.front();
            mDeque.pop_front();
            if constexpr (std::is_pointer<T>::value) {
                delete(t);
            }
        }
        notFull.notify_all();
    }

private:
    std::list<T> mDeque;
    int32_t capacity;

    std::mutex mu;

    std::condition_variable notFull;

    std::condition_variable notEmpty;

    std::atomic_bool blockPushFlag;
    std::atomic_bool blockPopFlag;

    void init(int32_t capacity) {
        this->capacity = capacity;
        blockPopFlag = capacity > 0;
        blockPushFlag = capacity > 0;
    }

    bool push(const T &t, bool blocking, bool front) {
        std::unique_lock<std::mutex> lock(mu);
        if (blocking && blockPushFlag) {
            if (capacity > 0) {
                // wait可能会在不满足条件的情况下被打断，因此仍然需要while来检测。
                while (mDeque.size() >= capacity && blockPushFlag) {
                    notFull.wait(lock);
                }
            }
        }
        // 外部可能会通知取消阻塞，如果此时仍然不满足入队条件，就返回false
        if (capacity > 0) {
            if (mDeque.size() >= capacity) {
                notEmpty.notify_all();
                return false;
            }
        }
        if (front) {
            mDeque.push_front(t);
        } else {
            mDeque.push_back(t);
        }
        notEmpty.notify_all();
        return true;
    }

    void forcePush(const T& t, bool front) {
        std::unique_lock<std::mutex> lock(mu);
        if (front) {
            mDeque.push_front(t);
        } else {
            mDeque.push_back(t);
        }
        notEmpty.notify_all();
    }

    std::optional<T> pop(bool blocking, bool front) {
        std::unique_lock<std::mutex> lock(mu);
        if (blocking && blockPopFlag) {
            while (mDeque.empty() && blockPopFlag) {
                notEmpty.wait(lock);
            }
        }

        if (mDeque.empty()) {
            notFull.notify_all();
            return std::nullopt;
        }

//        T t;
//        if (front) {
//            t = mDeque.front();
//            mDeque.pop_front();
//        } else {
//            t = mDeque.back();
//            mDeque.pop_back();
//        }
//
//        notFull.notify_all();
//
//        return std::optional<T>(t);

        std::optional<T> t = std::nullopt;
        if (front) {
            t = std::optional<T>(mDeque.front());
            mDeque.pop_front();
        } else {
            t = std::optional<T>(mDeque.back());
            mDeque.pop_back();
        }
        notFull.notify_all();
        return t;
    }
};

#endif //ANDROID_VIDEOPLAYER_THREADSAFELIST_H
