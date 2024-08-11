package com.zu.videoplayer.codec

import android.view.Surface

/**
 * @author zuguorui
 * @date 2024/1/5
 * @description
 */
interface IRecorder {

    val isReady: Boolean

    val isRecording: Boolean

    fun prepare(params: RecorderParams): Boolean
    fun getSurface(): Surface?
    fun start(): Boolean
    fun stop()
    fun release()
}