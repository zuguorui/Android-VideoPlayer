package com.zu.videoplayer.util

import java.util.concurrent.Future
import java.util.concurrent.Semaphore
import java.util.concurrent.TimeUnit
import java.util.concurrent.locks.Condition
import java.util.concurrent.locks.ReentrantLock

/**
 * @author zuguorui
 * @date 2024/1/11
 * @description
 */

fun <T> waitCallbackResult(block: (receiver: CallbackResultReceiver<T>) -> Any): T {
    val receiver = CallbackResultReceiver<T>()
    block(receiver)
    return receiver.get()
}

class CallbackResultReceiver<T> {
    private var result: T? = null

    private val semaphore = Semaphore(1, true).apply {
        acquire()
    }

    fun resume(t: T) {
        result = t
        semaphore.release()
    }

    fun get(): T {
        semaphore.acquire()
        val ret = result
        semaphore.release()
        return ret!!
    }

    fun get(timeout: Long): T? {
        semaphore.tryAcquire(timeout, TimeUnit.MILLISECONDS)
        val ret = result
        semaphore.release()
        return ret
    }
}