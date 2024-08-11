package com.zu.videoplayer.codec.encoder

import android.media.MediaCodec
import android.media.MediaCodecInfo
import android.media.MediaCodecList
import android.media.MediaFormat
import android.os.Bundle
import android.view.Surface
import com.zu.videoplayer.codec.RecorderParams
import com.zu.videoplayer.getFloatSafe
import com.zu.videoplayer.getIntegerSafe
import com.zu.videoplayer.getStringSafe
import timber.log.Timber
import java.nio.ByteBuffer
import java.util.concurrent.atomic.AtomicBoolean

/**
 * @author zuguorui
 * @date 2024/6/5
 * @description
 */
class VideoEncoder: BaseEncoder("VideoEncoder") {
    var surface: Surface? = null
        private set

    var config: Config? = null
        private set

    override fun prepare(params: RecorderParams): Boolean = synchronized(lockObj) {
        val callback = this.callback ?: return false
        if (state != EncoderState.IDLE) {
            return false
        }
        val useHEVC = supportHEVC() && params.inputFps >= 120
        val mimeType = if (useHEVC) MediaFormat.MIMETYPE_VIDEO_HEVC else MediaFormat.MIMETYPE_VIDEO_AVC

        val width = params.resolution.width
        val height = params.resolution.height

        val format = MediaFormat.createVideoFormat(mimeType, width, height)
        format.setInteger(MediaFormat.KEY_COLOR_STANDARD, MediaFormat.COLOR_STANDARD_BT709)
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface)
        val bitrate = computeVideoBitRate(width, height, params.outputFps, 8)
        Timber.d("bitrate: $bitrate bps")
        format.setInteger(MediaFormat.KEY_BIT_RATE, bitrate)
        format.setInteger(MediaFormat.KEY_FRAME_RATE, params.outputFps)
        format.setFloat(MediaFormat.KEY_CAPTURE_RATE, params.inputFps.toFloat())
        format.setInteger(MediaFormat.KEY_BITRATE_MODE, MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_VBR)
        format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1)
        try {
            encoder = MediaCodec.createEncoderByType(mimeType)
            encoder?.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE)
            surface = encoder?.createInputSurface()

            var b = Bundle()
            b.putInt(MediaCodec.PARAMETER_KEY_REQUEST_SYNC_FRAME, 0)
            encoder?.setParameters(b)
        } catch (e: Exception) {
            e.printStackTrace()
            callback.onError()
            return false
        }

        config = Config(format)

        state = EncoderState.PREPARED
        return true
    }

    override fun stop() {
        super.stop()
        config = null
    }

    override fun signalEndOfStream() {
        encoder?.signalEndOfInputStream()
    }

    private fun supportHEVC(): Boolean {
        val allCodec = MediaCodecList(MediaCodecList.ALL_CODECS)
        allCodec.codecInfos.forEach {
            if (it.isEncoder && it.supportedTypes.contains(MediaFormat.MIMETYPE_VIDEO_HEVC)) {
                return true
            }
        }
        return false
    }

    // 计算视频比特率
    // https://zidivo.com/blog/video-bitrate-guide/
    private val videoQuality = 1
    private fun computeVideoBitRate(width: Int, height: Int, frameRate: Int, pixelSize: Int): Int {
        return (0.07 * width * height * pixelSize * frameRate * videoQuality).toInt()
    }

    class Config {
        var mimeType = ""
        var width = 0
        var height = 0
        var fps = 0
        var bitrate = 0
        var bitrateMode = 0
        var profile = 0
        var level = 0
        var colorStandard = 0
        var colorFormat = 0
        var colorRange = 0

        constructor()

        constructor(format: MediaFormat) {
            mimeType = format.getStringSafe(MediaFormat.KEY_MIME)
            width = format.getIntegerSafe(MediaFormat.KEY_WIDTH)
            height = format.getIntegerSafe(MediaFormat.KEY_HEIGHT)
            bitrateMode = format.getIntegerSafe(MediaFormat.KEY_BITRATE_MODE)
            bitrate = format.getIntegerSafe(MediaFormat.KEY_BIT_RATE)
            colorFormat = format.getIntegerSafe(MediaFormat.KEY_COLOR_FORMAT)
            colorStandard = format.getIntegerSafe(MediaFormat.KEY_COLOR_STANDARD)
            colorRange = format.getIntegerSafe(MediaFormat.KEY_COLOR_RANGE)
            fps = format.getIntegerSafe(MediaFormat.KEY_FRAME_RATE)
            profile = format.getIntegerSafe(MediaFormat.KEY_PROFILE)
            level = format.getIntegerSafe(MediaFormat.KEY_LEVEL)
        }
    }

}