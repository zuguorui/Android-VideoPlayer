//
// Created by zu on 2024/8/3.
//

#include "HandlerThread.h"
#include "utils.h"
#include "Log.h"

#define TAG "HandlerThread"

using namespace std;

HandlerThread::HandlerThread() {
    startMessageThread();
}

HandlerThread::~HandlerThread() {
    stopMessageThread();
}


void HandlerThread::startMessageThread() {
    unique_lock<mutex> lock(startStopMu);
    if (messageThread != nullptr) {
        return;
    }
    stopFlag = false;
    messageThread = new thread(runCallback, this);
}

void HandlerThread::stopMessageThread() {
    unique_lock<mutex> lock(startStopMu);
    if (messageThread == nullptr) {
        return;
    }
    stopFlag = true;
    newMessageCond.notify_all();
    notEmptyCond.notify_all();
    if (messageThread->joinable()) {
        messageThread->join();
    }
    delete(messageThread);
    messageThread = nullptr;
}

void HandlerThread::runCallback(void *context) {
    ((HandlerThread *) context)->messageLoop();
}

void HandlerThread::messageLoop() {
    while (!stopFlag) {

        int64_t now = getSystemClockCurrentMicroseconds();

        optional<Message> opt = nullopt;
        {
            unique_lock<mutex> listLock(listMu);
            opt = popMessage();
        }

        if (opt.has_value()) {
            Message message = opt.value();
            int64_t waitTime = message.runAtTime - now;
            if (waitTime > 0) {
                unique_lock<mutex> listLock(listMu);
                insertMessage(message);
                newMessageCond.wait_for(listLock, chrono::microseconds(waitTime));
            } else {
                LOGD(TAG, "run message, name = %s", message.runnable.target_type().name());
                message.runnable();
            }
        } else {
            unique_lock<mutex> listLock(listMu);
            notEmptyCond.wait(listLock);
        }
    }
}


template<class Clock, class Duration>
void
HandlerThread::postAtTime(Runnable &&runnable, std::chrono::time_point<Clock, Duration> &&timePoint) {
    Message message {
        chrono::time_point_cast<chrono::microseconds>(timePoint).time_since_epoch().count(),
        runnable
    };

    unique_lock<mutex> listLock(listMu);
    insertMessage(message);
    newMessageCond.notify_all();
    notEmptyCond.notify_all();
}

void HandlerThread::post(Runnable &&runnable) {
    postAtTime(std::forward<Runnable>(runnable), chrono::system_clock::now());
}

template<class Rep, class Period>
void HandlerThread::postDelayed(Runnable &&runnable, std::chrono::duration<Rep, Period> &&delay) {
    postAtTime(std::forward<Runnable>(runnable), chrono::system_clock::now() + delay);
}

std::optional<HandlerThread::Message> HandlerThread::popMessage() {
    optional<Message> opt = nullopt;
    if (!messageQueue.empty()) {
        opt = optional(messageQueue.front());
        messageQueue.pop_front();
    }
    return opt;
}

void HandlerThread::insertMessage(HandlerThread::Message &message) {
    LOGD(TAG, "insert message, name = %s", message.runnable.target_type().name());
    if (messageQueue.empty()) {
        messageQueue.push_back(message);
        return;
    }
    for (auto it = messageQueue.begin(); it != messageQueue.end();) {
        Message &a = *it;
        if (a.runAtTime >= message.runAtTime) {
            messageQueue.insert(it, std::forward<Message>(message));
            break;
        }
        it++;
    }
}

void HandlerThread::removeAll(Runnable &&runnable) {
    unique_lock<mutex> listLock(listMu);
    for (auto it = messageQueue.begin(); it != messageQueue.end();) {
        if ((*it).runnable.target_type() == runnable.target_type()) {
            messageQueue.erase(it);
        } else {
            it++;
        }
    }
}

void HandlerThread::clear() {
    unique_lock<mutex> listLock(listMu);
    messageQueue.clear();
}


