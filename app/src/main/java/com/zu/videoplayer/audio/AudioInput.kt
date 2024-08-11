package com.zu.videoplayer.audio

import android.Manifest
import android.media.AudioFormat
import android.media.AudioRecord
import android.media.MediaRecorder
import androidx.annotation.RequiresPermission
import com.zu.videoplayer.codec.RecorderParams
import java.util.concurrent.atomic.AtomicBoolean

/**
 * @author zuguorui
 * @date 2024/6/7
 * @description
 */

class AudioInput {

    val isReady: Boolean
        get() = audioRecorder?.state == AudioRecord.STATE_INITIALIZED

    val isRecording: Boolean
        get() = audioRecorder?.recordingState == AudioRecord.RECORDSTATE_RECORDING

    private var audioRecorder: AudioRecord? = null

    var dataCallback: ((data: ByteArray, offset: Int, size: Int) -> Unit?)? = null

    private val stopFlag = AtomicBoolean(false)

    private var recordThread: Thread? = null

    private var bufferSize = 256

    @RequiresPermission(Manifest.permission.RECORD_AUDIO)
    fun prepare(params: RecorderParams): Boolean {
        if (audioRecorder != null) {
            return false
        }
        bufferSize = params.audioBufferSize
        audioRecorder = AudioRecord(MediaRecorder.AudioSource.MIC, params.sampleRate, params.channelConfig, AudioFormat.ENCODING_PCM_16BIT, bufferSize)
        if (audioRecorder!!.state == AudioRecord.STATE_INITIALIZED) {
            return true
        } else {
            audioRecorder = null
            return false
        }
    }

    @Synchronized
    fun start() {
        if (audioRecorder == null || dataCallback == null) {
            return
        }
        if (recordThread != null) {
            if (stopFlag.get()) {
                recordThread!!.join()
                recordThread = null
            } else {
                return
            }
        }
        stopFlag.set(false)
        recordThread = Thread(this::recordLoop, "AudioRecord-thread").apply {
            start()
        }
    }

    @Synchronized
    fun stop() {
        if (audioRecorder == null) {
            return
        }
        stopFlag.set(true)
        recordThread?.join()
        recordThread = null
    }

    @Synchronized
    fun release() {
        stop()
        audioRecorder?.release()
        audioRecorder = null
    }



    private fun recordLoop() {
        stopFlag.set(false)
        val recorder = audioRecorder ?: return
        val callback = dataCallback ?: return
        val buffer = ByteArray(bufferSize)
        recorder.startRecording()
        var readCount = 0
        while (!stopFlag.get()) {
            readCount = recorder.read(buffer, 0, bufferSize)
            callback.invoke(buffer, 0, readCount)
        }
        recorder.stop()
    }
}