package com.zu.videoplayer.codec

import android.media.AudioManager.AudioRecordingCallback
import android.media.EncoderProfiles
import android.media.MediaRecorder
import android.util.Size
import android.view.Surface
import com.zu.videoplayer.App
import java.io.File

class FakeMediaRecorderSurfaceProvider: IRecorder {
    override val isReady: Boolean
        get() = mediaRecorder?.surface != null
    override val isRecording: Boolean = false

    private var mediaRecorder: MediaRecorder? = null

    private val tempFilePath = File(App.context.cacheDir, "temp.mp4").absolutePath

    override fun prepare(params: RecorderParams): Boolean {
        return prepare(params.resolution, params.inputFps)
    }

    fun prepare(size: Size, fps: Int): Boolean {
        if (mediaRecorder != null) {
            return false
        }
        mediaRecorder = MediaRecorder()
        mediaRecorder?.apply {
            setVideoSource(MediaRecorder.VideoSource.SURFACE)
            setOutputFormat(MediaRecorder.OutputFormat.MPEG_4)

            setCaptureRate(fps.toDouble())
            setVideoFrameRate(fps / 5)

            if (fps >= 120) {
                setVideoEncoder(MediaRecorder.VideoEncoder.HEVC)
            } else {
                setVideoEncoder(MediaRecorder.VideoEncoder.H264)
            }

            setOrientationHint(90)

            setVideoSize(size.width, size.height)


            setOutputFile(tempFilePath)
        }
        mediaRecorder?.prepare()
        return true
    }

    override fun getSurface(): Surface? {
        return mediaRecorder?.surface
    }

    override fun start(): Boolean {
        // do nothing
        return false
    }

    override fun stop() {
        // do nothing
    }

    override fun release() {
        mediaRecorder?.release()
        mediaRecorder = null
    }
}