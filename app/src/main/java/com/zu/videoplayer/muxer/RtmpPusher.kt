package com.zu.videoplayer.muxer

import timber.log.Timber
import java.nio.ByteBuffer

class RtmpPusher {

    private var nativeObjectID: Long = 0

    val isInit: Boolean
        get() = nativeObjectID != 0L

    fun init() {
        if (isInit) {
            return
        }
        nInit()
        Timber.d("init, id = %d", nativeObjectID)
    }

    fun release() {
        nRelease()
    }

    fun setUrl(url: String) {
        if (url.isBlank()) {
            Timber.e("url is blank")
            return
        }
        checkInit()
        nSetUrl(url)
    }

    fun addAudioStream(mimeType: String, sampleRate: Int, channels: Int, bitrate: Int): Int {
        checkInit()
        return nAddAudioStream(mimeType, sampleRate, channels, bitrate)
    }

    fun addVideoStream(mimeType: String, fps: Double, width: Int, height: Int, pixelFmt: Int, profile: Int, level: Int, bitrate: Int): Int {
        checkInit()
        return nAddVideoStream(mimeType, fps, width, height, pixelFmt, profile, level, bitrate)
    }

    /**
     * 设置Codec Specific Data，例如H264的SPS/PPS
     * 需要在[sendData]前设置一次。否则可能导致无法播放。
     * */
    fun setCSD(buffer: ByteBuffer, offset: Int, size: Int, streamIndex: Int) {
        checkInit()
        nSetCSD(buffer, offset, size, streamIndex)
    }

    /**
     * 设置输出格式，例如flv、mp4
     * */
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


    fun sendData(buffer: ByteBuffer, offset: Int, size: Int, pts: Long, keyFrame: Boolean, streamIndex: Int) {
        checkInit()
        nSendData(buffer, offset, size, pts, keyFrame, streamIndex)
    }


    private fun checkInit() {
        if (nativeObjectID == 0L) {
            throw IllegalStateException("not init")
        }
    }

    private external fun nInit()
    private external fun nRelease()
    private external fun nSetUrl(url: String)
    private external fun nAddAudioStream(mimeType: String, sampleRate: Int, channels: Int, bitrate: Int): Int
    private external fun nAddVideoStream(mimeType: String, fps: Double, width: Int, height: Int, pixelFmt: Int, profile: Int, level: Int, bitrate: Int): Int
    private external fun nSetCSD(buffer: ByteBuffer, offset: Int, size: Int, streamIndex: Int)
    private external fun nSetOutputFormat(fmt: String)
    private external fun nStart(): Boolean
    private external fun nStop()
    private external fun nSendData(buffer: ByteBuffer, offset: Int, count: Int, pts: Long, keyFrame: Boolean, streamIndex: Int)


    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }
}