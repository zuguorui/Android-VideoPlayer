package com.zu.videoplayer.codec.encoder

import android.media.MediaCodec
import android.media.MediaFormat
import java.nio.ByteBuffer

/**
 * @author zuguorui
 * @date 2024/6/6
 * @description
 */
abstract class EncoderCallback {
    fun onStart() {}
    fun onStop() {}
    fun onError() {}
    fun onFinish() {}
    abstract fun onOutputBufferAvailable(buffer: ByteBuffer, info: MediaCodec.BufferInfo)
    abstract fun onOutputFormatChanged(format: MediaFormat)
}