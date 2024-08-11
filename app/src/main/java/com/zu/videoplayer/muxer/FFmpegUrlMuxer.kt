package com.zu.videoplayer.muxer

import timber.log.Timber
import java.nio.ByteBuffer

class FFmpegUrlMuxer {

    private var nativeObjectID: Long = 0

    val isInit: Boolean
        get() = nativeObjectID != 0L

    fun init() {
        nInit()
        Timber.d("init, id = %d", nativeObjectID)
    }

    fun release() {
        nRelease()
    }

    fun setUrl(url: String) {
        checkInit()
        nSetUrl(url)
    }

    fun addAudioStream(mimeType: String, sampleRate: Int, channels: Int): Int {
        checkInit()
        return nAddAudioStream(mimeType, sampleRate, channels)
    }

    fun addVideoStream(mimeType: String, fps: Double, width: Int, height: Int, pixelFmt: Int, profile: Int, level: Int): Int {
        checkInit()
        return nAddVideoStream(mimeType, fps, width, height, pixelFmt, profile, level)
    }

    fun setOutputFormat(fmt: String) {
        checkInit()
        nSetOutputFormat(fmt)
    }

    fun start(): Boolean {
        checkInit()
        return nStart()
    }

    fun stop() {
        checkInit()
        nStop()
    }


    fun sendData(buffer: ByteBuffer, offset: Int, size: Int, pts: Long, streamIndex: Int) {
        checkInit()
        nSendData(buffer, offset, size, pts, streamIndex)
    }


    private fun checkInit() {
        if (nativeObjectID == 0L) {
            throw IllegalStateException("not init")
        }
    }
    private external fun nInit()
    private external fun nRelease()
    private external fun nSetUrl(url: String)
    private external fun nAddAudioStream(mimeType: String, sampleRate: Int, channels: Int): Int
    private external fun nAddVideoStream(mimeType: String, fps: Double, width: Int, height: Int, pixelFmt: Int, profile: Int, level: Int): Int
    private external fun nSetOutputFormat(fmt: String)
    private external fun nStart(): Boolean
    private external fun nStop()
    private external fun nSendData(buffer: ByteBuffer, offset: Int, count: Int, pts: Long, streamIndex: Int)


    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }
}