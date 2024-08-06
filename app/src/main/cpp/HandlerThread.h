//
// Created by zu on 2024/8/3.
//

#include <iostream>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <list>
#include <optional>

typedef typename std::function<void(void)> Runnable;

class HandlerThread {
public:
    HandlerThread();
    ~HandlerThread();

    void post(Runnable &&runnable);

    template<class Rep, class Period>
    void postDelayed(Runnable &&runnable, std::chrono::duration<Rep, Period> &&delay);

    template<class Clock, class Duration>
    void postAtTime(Runnable &&runnable, std::chrono::time_point<Clock, Duration> &&timePoint);

    void removeAll(Runnable &&runnable);

    void clear();

private:
    struct Message {
        int64_t runAtTime = -1;
        Runnable runnable;
    };

    void startMessageThread();

    void stopMessageThread();

    std::mutex startStopMu;

    std::mutex listMu;
    std::condition_variable newMessageCond;
    std::condition_variable notEmptyCond;

    std::atomic_bool stopFlag = false;

    std::list<Message> messageQueue = std::list<Message>();

    std::thread *messageThread = nullptr;

    static void runCallback(void *context);

    void messageLoop();

    std::optional<Message> popMessage();
    void insertMessage(Message &message);
};


