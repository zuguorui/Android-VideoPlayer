package com.zu.videoplayer.codec.encoder

import android.media.MediaCodec
import com.zu.videoplayer.codec.RecorderParams
import timber.log.Timber
import java.nio.ByteBuffer
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.locks.ReentrantLock

/**
 * @author zuguorui
 * @date 2024/6/6
 * @description 基本编码器。
 */
abstract class BaseEncoder(val name: String) {

    var state = EncoderState.IDLE
        protected set

    protected var encoder: MediaCodec? = null

    var callback: EncoderCallback? = null

    protected var encodeThread: Thread? = null

    protected var stopFlag = AtomicBoolean(false)

    protected var lockObj = Any()

    abstract fun prepare(params: RecorderParams): Boolean

    /**
     * 开始编码。
     * 必须在state为[EncoderState.PREPARED]时调用才会成功。
     * 调用成功后，state切换为[EncoderState.STARTED]
     * */
    open fun start() = synchronized(lockObj) {
        if (state != EncoderState.PREPARED) {
            Timber.e("current state is $state, start() only can be called when state is ${EncoderState.PREPARED}")
            return
        }
        startEncodeThread()
        state = EncoderState.STARTED
    }

    /**
     * 结束编码。
     * 必须在编码器已经运行时才会起作用。结束之后，该编码器不再可用，
     * state切换为[EncoderState.IDLE]，必须重新prepare。
     * */
    open fun stop() = synchronized(lockObj) {
        if (state != EncoderState.STARTED) {
            Timber.e("current state is $state, stop() only can be called when state is ${EncoderState.STARTED}")
            return
        }
        signalEndOfStream()
        encodeThread?.join()
        encodeThread = null
        encoder?.release()
        encoder = null
        state = EncoderState.IDLE
    }

    /**
     * 暂停编码
     * */
    open fun pause() = synchronized(lockObj) {
        if (state != EncoderState.STARTED) {
            Timber.e("current state is $state, pause() only can be called when state is ${EncoderState.STARTED}")
            return
        }
        stopEncodeThread()
        state = EncoderState.PAUSED
    }


    open fun resume() = synchronized(lockObj) {
        if (state != EncoderState.PAUSED) {
            Timber.e("current state is $state, resume() only can be called when state is ${EncoderState.PAUSED}")
            return
        }
        startEncodeThread()
        state = EncoderState.STARTED
    }

    /**
     * 释放编码器。
     * 如果有正在进行的编码，将会结束编码。
     * 释放之后，state切换为[EncoderState.IDLE]
     * */
    open fun release() = synchronized(lockObj) {
        // 如果编码器不为空，那就通知其结束录制，并等待线程退出
        if (encoder != null) {
            signalEndOfStream()
            startEncodeThread()
        }
        encodeThread?.join()
        encodeThread = null
        encoder?.release()
        encoder = null
        state = EncoderState.IDLE
    }

    protected open fun startEncodeThread() {
        if (stopFlag.get()) {
            encodeThread?.join()
            encodeThread = null
        }
        if (encodeThread != null) {
            return
        }
        stopFlag.set(false)
        encodeThread = Thread(this::encodeLoop, "$name-thread").apply {
            start()
        }
    }

    protected open fun stopEncodeThread() {
        stopFlag.set(true)
        encodeThread?.join()
        encodeThread = null
    }

    protected open fun encodeLoop() {
        val encoder = encoder ?: return
        val callback = callback ?: return
        var outputBufferId: Int = 0
        var outputBufferInfo = MediaCodec.BufferInfo()
        var outputBuffer: ByteBuffer? = null
        var isEof = false
        encoder.start()
        callback?.onStart()
        while (!stopFlag.get()) {
            outputBufferId = encoder.dequeueOutputBuffer(outputBufferInfo, 5000)
            if (outputBufferId >= 0) {
                if (outputBufferInfo.flags and MediaCodec.BUFFER_FLAG_END_OF_STREAM != 0) {
                    isEof = true
                    stopFlag.set(true)
                }
                outputBuffer = encoder.getOutputBuffer(outputBufferId)
                callback.onOutputBufferAvailable(outputBuffer!!, outputBufferInfo)
                encoder.releaseOutputBuffer(outputBufferId, false)
                outputBuffer = null
            } else if (outputBufferId == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                callback?.onOutputFormatChanged(encoder.outputFormat)
            }
        }
        if (isEof) {
            Timber.d("$name meets eof")
            encoder.release()
            this.encoder = null
            state = EncoderState.IDLE
            callback?.onFinish()
        } else {
            encoder.stop()
            callback?.onStop()
        }
    }

    // 不同的编码器实现有不同的结束方式。
    // 对于音频编码器，没有surface，只能用传统的传入空的buffer并且flag为[MediaCodec.BUFFER_FLAG_END_OF_STREAM]。
    // 对于视频编码器，有surface，只能用encoder.signalEndOfStream
    protected abstract fun signalEndOfStream()
}