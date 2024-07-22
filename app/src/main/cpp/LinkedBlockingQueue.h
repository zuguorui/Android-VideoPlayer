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
 * @brief 线程安全的FIFO阻塞队列。队列空时，会阻塞pop，队列满时，会阻塞push。
 * 原理与list相同，元素都包装在node中，写与读不会相互阻塞。
 *
 * @tparam T
 */


template <typename T>
class LinkedBlockingQueue {
public:

    LinkedBlockingQueue(LinkedBlockingQueue &a) = delete;
    LinkedBlockingQueue(LinkedBlockingQueue &&a) = delete;

    LinkedBlockingQueue() {
        init(-1);
    }
    /**
     * @brief Construct a new Linked Blocking Queue object
     *
     * @param capacity If capacity <= 0, it means this queue has infinite capacity and will not block pushBack option.
     */
    LinkedBlockingQueue(int32_t capacity) {
        init(capacity);
    }



    ~LinkedBlockingQueue() {
        std::unique_lock<std::mutex> pushLock(pushMu);
        std::unique_lock<std::mutex> popLock(popMu);
        clearInner();
    }

    int32_t getCapacity() {
        return capacity;
    }
    int32_t getSize() {
        return size;
    }

    /**
     * 队列是否已满。
     * */
    bool isFull() {
        std::unique_lock<std::mutex> popLock(popMu);
        std::unique_lock<std::mutex> pushLock(pushMu);
        return size >= capacity;
    }

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
    bool pushBack(const T &t, bool blocking = true) {
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

    /**
     * @brief Force pushBack an object in to the queue ignoring capacity.
     *
     * @param t
     */
    void forcePushBack(T &t) {
        std::unique_lock<std::mutex> lock(pushMu);
        enqueue(t);
        notEmpty.notify_all();
    }

    /**
     * @brief Get and remove the earliest pushed element.
     *
     * @param blocking Whether block popping when the queue is empty. If blocking == true,
     * poping will be blocked until the queue is not empty. If blocking == false, poping will
     * return immediately, if the queue is empty, std::nullopt will be returned.
     * @return std::optional<T> The element which was removed.
     */
    std::optional<T> popFront(bool blocking = true) {
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
    void setBlockPush(bool blockPush) {
        this->blockPushFlag = blockPush;
        if (!blockPush) {
            notFull.notify_all();
        }
    }

    /**
     * @brief Whether this queue will block pushing when it is full.
     *
     * @return
     */
    bool isBlockPush() {
        return this->blockPushFlag.load();
    }

    /**
     * @brief see {setBlockPush(bool blockPush)}
     *
     * @param blockPop
     */
    void setBlockPop(bool blockPop) {
        this->blockPopFlag = blockPop;
        if (!blockPop) {
            notEmpty.notify_all();
        }
    }

    /**
     * @brief Whether this queue will block popping when it is empty.
     *
     * @return
     */
    bool isBlockPop() {
        return this->blockPopFlag.load();
    }



    /**
     * Clear this queue. If the T is pointer type, it will free the memory where the pointer point to
     * */
    void clear() {
        std::unique_lock<std::mutex> popLock(popMu);
        std::unique_lock<std::mutex> pushLock(pushMu);
        clearInner();
        notFull.notify_all();
    }

    /**
     * 使用自己的析构器来清理元素。
     * */
    void clear(void (*deleter)(const T&)) {
        std::unique_lock<std::mutex> popLock(popMu);
        std::unique_lock<std::mutex> pushLock(pushMu);
        clearInner(deleter);
        notFull.notify_all();
    }


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

    void enqueue(const T& t) {
        Node *node = new Node();
        node->pointer = std::make_unique<T>(t);
        tail->next = node;
        tail = node;
        size++;
    }
    std::unique_ptr<T> dequeue() {
        Node *p = head->next;
        delete head;
        head = p;
        size--;
        return std::move(head->pointer);
    }

    void init(int capacity) {
        //LOGD(TAG, "LinkedBlockingQueue: T.name = %s, isPointer = %d", typeid(T).name(), std::is_pointer<T>::value);
        this->capacity = capacity;
        head = new Node();
        tail = head;
        size = 0;
        blockPushFlag = true;
        blockPopFlag = true;
    }

    void clearInner() {
        Node *n = nullptr;
        while (head != tail) {
            n = head;
            head = head->next;
            delete(n);
            if constexpr (std::is_pointer<T>::value) {
                // 智能指针会申请一块内存保存数据，它自己持有该内存的指针。head->pointer相当于T*。
                // 例如T是class*，unique_ptr申请了一块内存存放class*，指向这块内存的指针相当于
                // class**。因此这里先要释放class*指向的内存，然后让unique_ptr自己释放存放class*
                // 的内存
                delete(*(head->pointer.get()));
                head->pointer.reset();
            } else {
                head->pointer.reset();
            }
            size--;
        }
    }

    void clearInner(void (*deleter)(const T&)) {
        Node *n = nullptr;
        while (head != tail) {
            n = head;
            head = head->next;
            delete(n);
            deleter(*(head->pointer.get()));
            head->pointer.reset();
            size--;
        }
    }
};

#endif //_LINKEDBLOCKINGQUEUE_H_
