//
// Created by 祖国瑞 on 2022/8/30.
//

#ifndef _LINKEDBLOCKINGQUEUE_H_
#define _LINKEDBLOCKINGQUEUE_H_

#include <stdlib.h>
#include <stdint.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <memory>

/**
 * @brief This class is a thread-safe blocking queue.
 *
 * @tparam T
 */


template <class T>
class LinkedBlockingQueue {
public:

    LinkedBlockingQueue(LinkedBlockingQueue &a) = delete;
    LinkedBlockingQueue(LinkedBlockingQueue &&a) = delete;

    LinkedBlockingQueue();
    /**
     * @brief Construct a new Linked Blocking Queue object
     *
     * @param capacity If capacity <= 0, it means this queue has infinite capacity and will not block pushBack option.
     */
    LinkedBlockingQueue(int32_t capacity);
    ~LinkedBlockingQueue();
    int32_t getCapacity();
    int32_t getSize();

    /**
     * @brief Push an element into queue.
     *
     * @param t The element to be bushed
     * @param blocking Whether block pushing when the queue is full(size >= capacity). If
     * blocking == false, false will be returned immediately when queue
     * is full.
     *
     * @attention The description above is the normal behavior of pushing. For more
     * conditions, see {setBlockPush(bool)}.
     *
     * @return Whether pushBack succeed. There are two conditions in which pushing will fail.
     * 1. blocking == false and the queue is full (size >= capacity)
     * 2. A blocking pushing is waked by {setBlockPush(false)} and the queue is still full.
     */
    bool pushBack(const T &t, bool blocking = true);

    /**
     * @brief Force pushBack an object in to the queue ignoring capacity.
     *
     * @param t
     */
    void forcePushBack(T &t);

    /**
     * @brief Get and remove the earliest pushed element.
     *
     * @param blocking Whether block popping when the queue is empty. If blocking == true,
     * poping will be blocked until the queue is not empty. If blocking == false, poping will
     * return immediately, if the queue is empty, std::nullopt will be returned.
     * @return std::optional<T> The element which was removed.
     */
    std::optional<T> popFront(bool blocking = true);

    //std::optional<const T&> peekFront();

    /**
     * @brief Set whether block pushing when queue is full.
     * This is the top level trigger. If blockPush == false, {pushBack(T& t, bool blocking)}
     * will never block no matter what value the blocking is. And already blocked threads will
     * be waked.
     * If blockPush == true, which is default value, {pushBack(T& t, bool blocking)} behaves as its
     * note descriped.
     *
     * @details The intent of this function is to solve the problem that std::thread can't be
     * waked or interrupted when it is waiting except the waited signal notifies it. If you want
     * to stop a blocking thread, you can use {setBlockPush(false)} to notify all threads
     * blocking in pushing, which will have them a opportunity to check their stop flag.
     * Here is a simple code example:
     * @code
     * volatile atomic_bool stopFlag = false;
     * LinkedBlockingQueue<Element> queue(capacity);
     * static void taskLoop() // this is a task running in a thread
     * {
     *      bool pushSucceed = false;
     *      while (!stopFlag)
     *      {
     *          // some work to prepare a element
     *          pushSucceed = queue.pushBack(element); // this step may blocking.
     *      }
     *      // now you are out of the loop and can finish this thread task
     *      // if you don't want to waste the element, you can use {forcePushBack(T& t)} to
     *      // pushBack the element into queue.
     *      if (!pushSucceed) {
     *          queue.forcePushBack(element);
     *      }
     * }
     *
     * int main()
     * {
     *      thread *t = new thread(taskLoop);
     *      // do something
     *      ...
     *      // now you need to finish the task, but it may be blocking. The following step
     *      // shows how you can jump out of blocking.
     *      // 1. set stop flag to indicate the thread to stop.
     *      stopFlag = true;
     *      // 2. wake possiable blocking threads
     *      queue.setBlockPush(false);
     *      // 3. waiting the thread to finish its task.
     *      t->join();
     *
     *      // 4. if you want to reuse the queue as blocking one, just set block flag as true.
     *      queue.setBlockPush(true);
     *
     *      ...
     *      return 0;
     * }
     * @endcode
     *
     *
     * @param blockPush
     */
    void setBlockPush(bool blockPush);

    /**
     * @brief Whether this queue will block pushing when it is full.
     *
     * @return
     */
    bool isBlockPush();

    /**
     * @brief see {setBlockPush(bool blockPush)}
     *
     * @param blockPop
     */
    void setBlockPop(bool blockPop);

    /**
     * @brief Whether this queue will block popping when it is empty.
     *
     * @return
     */
    bool isBlockPop();

    /**
     * 队列是否已满。
     * */
    bool isFull();

    /**
     * Clear this queue. If the T is pointer type, it will free the memory where the pointer point to
     * */
    void clear();

    /**
     * 使用自己的析构器来清理元素。
     * */
    void clear(void (*deleter)(T));


private:
    const char* TAG = "LinkedBlockingQueue";


    int32_t capacity;
    std::atomic_int32_t size;

    std::mutex popMu;
    std::condition_variable notFull;

    std::mutex pushMu;
    std::condition_variable notEmpty;

    struct Node {
        std::unique_ptr<T> pointer;
        Node *next;

        Node() {
            next = nullptr;
        }

        ~Node() {
            pointer.reset();
        }
    };

    Node *head;
    Node *tail;

    std::atomic_bool blockPushFlag;
    std::atomic_bool blockPopFlag;

    void enqueue(const T& t);
    std::unique_ptr<T> dequeue();
};

template <class T>
LinkedBlockingQueue<T>::LinkedBlockingQueue(): LinkedBlockingQueue(INT32_MAX) {

}

template <class T>
LinkedBlockingQueue<T>::LinkedBlockingQueue(int32_t capacity) {
    //LOGD(TAG, "LinkedBlockingQueue: T.name = %s, isPointer = %d", typeid(T).name(), std::is_pointer<T>::value);
    this->capacity = capacity;
    head = new Node();
    tail = head;
    size = 0;
    blockPushFlag = true;
    blockPopFlag = true;
}

template <class T>
LinkedBlockingQueue<T>::~LinkedBlockingQueue() {
    Node *n = nullptr;
    while (head != tail) {
        n = head;
        head = head->next;
        delete(n);
        // 编译时就确定确定条件值。if constexpr不支持短路规则
        // 例如if constexpr (condA && condB)，即使condA为false，
        // condB仍然会被检测，这和一般的if不一样。
        if constexpr (std::is_pointer<T>::value) {
            delete(*(head->pointer.get()));
            head->pointer.reset();
        } else {
            head->pointer.reset();
        }
        size--;
    }
}

template <class T>
void LinkedBlockingQueue<T>::enqueue(const T& t) {
    Node *node = new Node();
    node->pointer = std::make_unique<T>(t);
    tail->next = node;
    tail = node;
    size++;
}

template <class T>
std::unique_ptr<T> LinkedBlockingQueue<T>::dequeue() {
    Node *p = head->next;
    delete head;
    head = p;
    size--;
    return std::move(head->pointer);
}



template <class T>
int32_t LinkedBlockingQueue<T>::getCapacity() {
    return capacity;
}

template <class T>
int32_t LinkedBlockingQueue<T>::getSize() {
    return size;
}

template <class T>
bool LinkedBlockingQueue<T>::isFull() {
    std::unique_lock<std::mutex> popLock(popMu);
    std::unique_lock<std::mutex> pushLock(pushMu);
    return size >= capacity;
}

template <class T>
bool LinkedBlockingQueue<T>::pushBack(const T& t, bool blocking) {
    std::unique_lock<std::mutex> lock(pushMu);
    if (blocking && blockPushFlag) {
        if (capacity > 0) {
            while (size >= capacity && blockPushFlag) {
                notFull.wait(lock);
            }
        }
    }

    if (capacity > 0) {
        if (size >= capacity) {
            notEmpty.notify_all();
            return false;
        }
    }

    enqueue(t);
    notEmpty.notify_all();
    return true;
}



template <class T>
void LinkedBlockingQueue<T>::forcePushBack(T &t) {
    std::unique_lock<std::mutex> lock(pushMu);
    enqueue(t);
    notEmpty.notify_all();
}

template <class T>
std::optional<T> LinkedBlockingQueue<T>::popFront(bool blocking) {
    std::unique_lock<std::mutex> lock(popMu);
    if (blocking && blockPopFlag) {
        while (size == 0 && blockPopFlag) {
            notEmpty.wait(lock);
        }
    }

    if (size == 0) {
        notFull.notify_all();
        return std::nullopt;
    }
    std::unique_ptr<T> t = dequeue();
    notFull.notify_all();
    return std::optional<T>(*t);
}

template <class T>
void LinkedBlockingQueue<T>::setBlockPush(bool blockPush) {
    this->blockPushFlag = blockPush;
    if (!blockPush) {
        notFull.notify_all();
    }
}

template <class T>
bool LinkedBlockingQueue<T>::isBlockPush() {
    return this->blockPushFlag.load();
}

template <class T>
void LinkedBlockingQueue<T>::setBlockPop(bool blockPop) {
    this->blockPopFlag = blockPop;
    if (!blockPop) {
        notEmpty.notify_all();
    }
}

template <class T>
bool LinkedBlockingQueue<T>::isBlockPop() {
    return this->blockPopFlag.load();
}

template <class T>
void LinkedBlockingQueue<T>::clear() {
    if (size == 0) {
        return;
    }
    std::unique_lock<std::mutex> popLock(popMu);
    std::unique_lock<std::mutex> pushLock(pushMu);
    Node *n = nullptr;
    while (head != tail) {
        n = head;
        head = head->next;
        delete(n);

        if constexpr (std::is_pointer<T>::value) {
            delete(*(head->pointer.get()));
            head->pointer.reset();
        } else {
            head->pointer.reset();
        }
        size--;
    }

    notFull.notify_all();
}

template<class T>
void LinkedBlockingQueue<T>::clear(void (*deleter)(T)) {
    if (size == 0) {
        return;
    }
    std::unique_lock<std::mutex> popLock(popMu);
    std::unique_lock<std::mutex> pushLock(pushMu);
    Node *n = nullptr;
    while (head != tail) {
        n = head;
        head = head->next;
        delete(n);
        deleter(*(head->pointer.release()));
        size--;
    }
    notFull.notify_all();
}


#endif //_LINKEDBLOCKINGQUEUE_H_
