package com.zu.videoplayer.codec

import android.media.AudioFormat
import android.media.AudioRecord
import android.net.Uri
import android.util.Size
import java.io.File

/**
 * @author zuguorui
 * @date 2024/1/5
 * @description
 */
data class RecorderParams(
    val title: String,
    val resolution: Size,
    val outputFps: Int,
    val inputFps: Int,
    val sampleRate: Int,
    val channelConfig: Int = AudioFormat.CHANNEL_IN_STEREO,
    val outputPath: String?,
    val outputUri: Uri?,
    val viewOrientation: Int,
    val sensorOrientation: Int,
    val facing: Int,
) {
    val audioBufferSize = AudioRecord.getMinBufferSize(sampleRate, AudioFormat.CHANNEL_IN_STEREO, AudioFormat.ENCODING_PCM_16BIT)

    override fun toString(): String {
        return """
            RecordParams {
                resolution: $resolution
                outputFps: $outputFps
                inputFps: $inputFps
            }
        """.trimIndent()
    }
}
