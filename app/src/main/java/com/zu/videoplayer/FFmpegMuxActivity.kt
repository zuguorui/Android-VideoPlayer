package com.zu.videoplayer

import android.annotation.SuppressLint
import android.content.ContentValues
import android.content.Intent
import android.hardware.camera2.CameraCharacteristics
import android.hardware.camera2.CameraDevice
import android.hardware.camera2.CaptureRequest
import android.media.ImageReader
import android.media.MediaCodec
import android.media.MediaFormat
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.provider.MediaStore
import android.util.Size
import android.view.Surface
import android.view.SurfaceHolder
import androidx.appcompat.app.AppCompatActivity
import com.zu.videoplayer.audio.AudioInput
import com.zu.videoplayer.camera.BaseCameraLogic
import com.zu.videoplayer.camera.Camera2PreviewView
import com.zu.videoplayer.camera.CameraInfoWrapper
import com.zu.videoplayer.camera.FPS
import com.zu.videoplayer.camera.PreviewViewImplementation
import com.zu.videoplayer.camera.queryCameraInfo
import com.zu.videoplayer.codec.RecorderParams
import com.zu.videoplayer.codec.encoder.AudioEncoder
import com.zu.videoplayer.codec.encoder.EncoderCallback
import com.zu.videoplayer.codec.encoder.EncoderState
import com.zu.videoplayer.codec.encoder.VideoEncoder
import com.zu.videoplayer.databinding.ActivityFfmpegMuxBinding
import com.zu.videoplayer.muxer.FFmpegUrlMuxer
import com.zu.videoplayer.util.createVideoPath
import com.zu.videoplayer.util.createVideoUri
import timber.log.Timber
import java.nio.ByteBuffer
import java.text.SimpleDateFormat
import java.util.Date
import java.util.concurrent.atomic.AtomicLong

class FFmpegMuxActivity : AppCompatActivity() {

    // camera objects start
    private val cameraInfoMap: HashMap<String, CameraInfoWrapper> by lazy {
        queryCameraInfo(this)
    }

    private lateinit var cameraLogic: BaseCameraLogic

    private var currentCameraID: String? = null
    private var currentSize: Size? = null
    private var currentFps: FPS? = null

    private var isFrontCamera = true

    // record components

    private var audioStreamIndex = -1
    private var videoStreamIndex = -1

    private var audioStartPts = -1L
    private var videoStartPts = -1L

    // 第一帧数据的时间戳，用来同步音视频输出，避免错位
    private var firstAudioFrameTS = AtomicLong(-1)
    private var firstVideoFrameTS = AtomicLong(-1)

    private val audioInputCallback = { buffer: ByteArray, offset: Int, size: Int ->
        audioEncoder.feed(buffer, offset, size)
    }

    private val audioEncodeCallback = object : EncoderCallback() {
        override fun onOutputBufferAvailable(buffer: ByteBuffer, info: MediaCodec.BufferInfo) {
            if (firstAudioFrameTS.get() == -1L) {
                firstAudioFrameTS.set(System.currentTimeMillis())
                if (firstVideoFrameTS.get() != -1L) {
                    Timber.w("video before audio ${firstAudioFrameTS.get() - firstVideoFrameTS.get()} ms")
                    // 第一帧音频拿到了，如果视频已经出数据了，就可以开始mux了
                    muxer.start()
                }
            }
            // 视频有数据之后再开始推送音频
            if (firstVideoFrameTS.get() == -1L) {
                return
            }
            if (audioStartPts == -1L) {
                audioStartPts = info.presentationTimeUs
            }
            info.presentationTimeUs -= audioStartPts
            val keyFrame = (info.flags and MediaCodec.BUFFER_FLAG_KEY_FRAME) == MediaCodec.BUFFER_FLAG_KEY_FRAME
            muxer.sendData(buffer, info.offset, info.size, info.presentationTimeUs, keyFrame, audioStreamIndex)
        }

        override fun onOutputFormatChanged(format: MediaFormat) {
            audioStartPts = -1L

            // 编码器启动，通知外部编码器参数。先添加Stream后添加CSD

            if (audioStreamIndex >= 0) {
                var csd0 = format.getByteBuffer("csd-0")
                var csd1 = format.getByteBuffer("csd-1")
                Timber.i("audio outputFormatChanged: %s", format.toString())

                var csdCapacity = (csd0?.remaining() ?: 0) + (csd1?.remaining() ?: 0)
                val csd = ByteBuffer.allocateDirect(csdCapacity)
                if (csd0 != null) {
                    csd.put(csd0)
                }
                if (csd1 != null) {
                    csd.put(csd1)
                }
                csd.rewind()
                muxer.setCSD(csd, 0, csdCapacity, audioStreamIndex)
            } else {
                Timber.e("audioStreamIndex < 0")
            }
        }

        override fun onFinish() {
            Timber.d("audio encoder finish")
        }
    }

    private val videoEncoderCallback = object : EncoderCallback() {
        override fun onOutputBufferAvailable(buffer: ByteBuffer, info: MediaCodec.BufferInfo) {
            if (firstVideoFrameTS.get() == -1L) {
                firstVideoFrameTS.set(System.currentTimeMillis())
                if (firstAudioFrameTS.get() != -1L) {
                    Timber.w("audio before video ${firstVideoFrameTS.get() - firstAudioFrameTS.get()} ms")
                    muxer.start()
                }
            }

            if (firstAudioFrameTS.get() == -1L) {
                return
            }

            val keyFrame = (info.flags and MediaCodec.BUFFER_FLAG_KEY_FRAME) == MediaCodec.BUFFER_FLAG_KEY_FRAME

            if (videoStartPts == -1L) {
                videoStartPts = info.presentationTimeUs
            }
            info.presentationTimeUs -= videoStartPts
            muxer.sendData(buffer, info.offset, info.size, info.presentationTimeUs, keyFrame, videoStreamIndex)
        }

        override fun onOutputFormatChanged(format: MediaFormat) {
            videoStartPts = -1L

            if (videoStreamIndex >= 0) {
                var csd0 = format.getByteBuffer("csd-0")
                var csd1 = format.getByteBuffer("csd-1")
                Timber.i("video outputFormatChanged: %s", format.toString())
                val profile = format.getIntegerSafe(MediaFormat.KEY_PROFILE)
                val level = format.getIntegerSafe(MediaFormat.KEY_LEVEL)
                Timber.i("video profile = %d, level = %d", profile, level)

                var csdCapacity = (csd0?.remaining() ?: 0) + (csd1?.remaining() ?: 0)
                val csd = ByteBuffer.allocateDirect(csdCapacity)
                if (csd0 != null) {
                    csd.put(csd0)
                }
                if (csd1 != null) {
                    csd.put(csd1)
                }
                csd.rewind()
                muxer.setCSD(csd, 0, csdCapacity, videoStreamIndex)
            } else {
                Timber.e("videoStreamIndex < 0")
            }
        }

        override fun onFinish() {
            Timber.d("video encoder finish")
        }
    }

    private val audioInput = AudioInput().apply {
        dataCallback = audioInputCallback
    }
    private var videoEncoder = VideoEncoder()

    private var audioEncoder = AudioEncoder()

    private var muxer = FFmpegUrlMuxer()

    private val surfaceStateListener = object : PreviewViewImplementation.SurfaceStateListener {
        override fun onSurfaceCreated(surface: Surface) {
            Timber.d("surfaceCreated: Thread = ${Thread.currentThread().name}")
            selectCameraConfig(isFrontCamera)
            openCamera()
        }

        override fun onSurfaceSizeChanged(surface: Surface, surfaceWidth: Int, surfaceHeight: Int) {
            val surfaceSize = Size(surfaceWidth, surfaceHeight)
            Timber.d("surfaceChanged: surfaceSize = $surfaceSize, ratio = ${surfaceSize.toRational()}")
        }

        override fun onSurfaceDestroyed(surface: Surface) {
            closeCamera()
        }
    }
    // camera objects end

    private var recording = false
        set(value) {
            field = value
            binding.btnStart.text = if (value) "停止" else "开始"
            binding.btnSetting.isEnabled = !value
            binding.btnSwitchCamera.isEnabled = !value
        }

    private var recordParams: RecorderParams? = null

    private lateinit var binding: ActivityFfmpegMuxBinding
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityFfmpegMuxBinding.inflate(layoutInflater)
        setContentView(binding.root)

        initCameraLogic()
        initViews()
    }

    override fun onStop() {
        super.onStop()
        stopRecord()
    }

    private fun initCameraLogic() {
        cameraLogic = BaseCameraLogic(this)
        cameraLogic.configCallback = object : BaseCameraLogic.ConfigCallback {
            override fun getFps(): FPS {
                return currentFps!!
            }

            override fun getSize(): Size {
                return currentSize!!
            }

            override fun getTemplate(): Int {
                return if (recording) {
                    CameraDevice.TEMPLATE_RECORD
                } else {
                    CameraDevice.TEMPLATE_PREVIEW
                }
            }

            override fun getSessionSurfaceList(): List<Surface> {
                var surfaceList = arrayListOf(binding.preview.surface)
                if (videoEncoder.state == EncoderState.STARTED) {
                    surfaceList.add(videoEncoder.surface!!)
                }
                return surfaceList
            }

            override fun getCaptureSurfaceList(): List<Surface> {
                var surfaceList = arrayListOf(binding.preview.surface)
                if (videoEncoder.state == EncoderState.STARTED) {
                    surfaceList.add(videoEncoder.surface!!)
                }
                return surfaceList
            }

            override fun configBuilder(requestBuilder: CaptureRequest.Builder) {

            }
        }
    }

    private fun initViews() {
        binding.preview.implementationType = Camera2PreviewView.ImplementationType.SURFACE_VIEW
        binding.preview.scaleType = Camera2PreviewView.ScaleType.FIT_CENTER
        binding.preview.surfaceStateListener = surfaceStateListener

        binding.btnStart.setOnClickListener {
            if (recording) {
                stopRecord()
            } else {
                startRecord()
            }
        }

        binding.btnSetting.setOnClickListener {
            val intent = Intent(this, SettingActivity::class.java)
            intent.putExtra(SettingActivity.KEY_SETTING_FLAGS, SettingActivity.SETTING_FLAG_FILE_OUTPUT_LOCATION)
            startActivity(intent)
        }

        binding.btnSwitchCamera.setOnClickListener {
            closeCamera()
            selectCameraConfig(!isFrontCamera)
            openCamera()
        }
    }

    private fun selectCameraConfig(isFront: Boolean) {
        isFrontCamera = isFront
        val facing = if (isFront) CameraCharacteristics.LENS_FACING_FRONT else CameraCharacteristics.LENS_FACING_BACK
        val camera = cameraInfoMap.values.stream().filter {
            it.lensFacing == facing
        }.sorted { o1, o2 ->
            if (o1.cameraID >= o2.cameraID) {
                1
            } else {
                -1
            }
        }.findFirst().get()

        currentCameraID = camera.cameraID

        val sizeList = arrayListOf(
            Size(1920, 1080),
            Size(1280, 720),
            Size(640, 480),
            Size(320, 240)
        )

        for (size in sizeList) {
            val imageReaderSizeList = camera.classSizeMap[ImageReader::class.java]!!
            val previewSizeList = camera.classSizeMap[SurfaceHolder::class.java]!!
            if (imageReaderSizeList.contains(size) && previewSizeList.contains(size)) {
                currentSize = size
                break
            }
        }

        val size = currentSize ?: run {
            Timber.e("no size selected")
            return
        }

        binding.preview.previewSize = size

        currentFps = camera.fpsRanges.filter {
            it.lower == it.upper
        }.maxByOrNull {
            it.lower
        }?.let {
            FPS(it.lower, FPS.Type.NORMAL)
        }
    }
    private fun openCamera() {
        val cameraID = currentCameraID ?: run {
            Timber.e("id is null")
            return
        }
        val size = currentSize ?: run {
            Timber.e("size is null")
            return
        }
        val fps = currentFps ?: run {
            Timber.e("fps is null")
            return
        }
        cameraLogic.openCamera(cameraInfoMap[cameraID]!!)
    }

    private fun closeCamera() {
        cameraLogic.closeCamera()
    }


    @SuppressLint("MissingPermission")
    private fun startRecord() {
        //stopRecord()

        audioEncoder.callback = audioEncodeCallback
        videoEncoder.callback = videoEncoderCallback

        val fmt = "flv"

        val pushStream = Settings.fileOutputType == FILE_OUTPUT_TYPE_STREAM && Settings.pushStreamUrl.isNotBlank()

        firstAudioFrameTS.set(-1)
        firstVideoFrameTS.set(-1)
        audioStartPts = -1L
        videoStartPts = -1L

        val size = currentSize ?: return
        val fps = currentFps ?: return
        val camera = currentCameraID?.let {
            cameraInfoMap[it] ?: return
        } ?: return

        Timber.d("""
            startRecord:
                size = $size
                fps = $fps
                cameraID = ${camera.cameraID}
        """.trimIndent())

        val timeStamp = SimpleDateFormat("yyyy-MM-dd_HH-mm-ss").format(Date(System.currentTimeMillis()))
        val title = "${size.width}x${size.height}@${fps.value}FPS.$timeStamp.$fmt"

        var saveUri: Uri? = null
        var savePath: String? = null

        if (!pushStream) {
            val saveUriPair = if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                createVideoUri(this, title)
            } else {
                null
            }

            saveUriPair?.let {
                saveUri = it.first
                savePath = it.second
            } ?: run {
                savePath = createVideoPath(title)
            }
        } else {
            savePath = Settings.pushStreamUrl
        }

        val params = RecorderParams(
            title = title,
            resolution = size,
            inputFps = fps.value,
            outputFps = fps.value,
            sampleRate = 44100,
            outputPath = savePath,
            outputUri = saveUri,
            viewOrientation = binding.root.display.rotation * 90,
            sensorOrientation = camera.sensorOrientation!!,
            facing = camera.lensFacing
        )

        if (!videoEncoder.prepare(params)) {
            Timber.e("video encoder prepare failed")
            return
        }

        if (!audioEncoder.prepare(params)) {
            Timber.e("audio encoder prepare failed")
            return
        }

        if (!audioInput.prepare(params)) {
            Timber.e("audio input prepare failed")
            return
        }
        muxer.init()
        muxer.setOutputFormat("flv")
        muxer.setUrl(savePath!!)

        audioEncoder.config?.let {
            audioStreamIndex = muxer.addAudioStream(it.mimeType, it.sampleRate, it.channels, it.bitrate)
        } ?: run {
            Timber.e("audio encoder config is null")
            return
        }

        if (audioStreamIndex < 0) {
            Timber.e("add audio stream failed")
            return
        }

        videoEncoder.config?.let {
            videoStreamIndex = muxer.addVideoStream(it.mimeType, it.fps.toDouble(), it.width, it.height, it.colorFormat, it.profile, it.level, it.bitrate)
            Timber.d("video profile = %d, level = %d", it.profile, it.level)
        } ?: run {
            Timber.e("video encoder config is null")
            return
        }

        if (videoStreamIndex < 0) {
            Timber.e("add video stream failed")
            return
        }


        videoEncoder.start()
        if (videoEncoder.state != EncoderState.STARTED) {
            Timber.e("video start failed")
            return
        }

        audioEncoder.start()
        if (audioEncoder.state != EncoderState.STARTED) {
            Timber.e("audio start failed")
            return
        }

        recordParams = params
        cameraLogic.closeSession()
        audioInput.start()
        recording = true
        cameraLogic.createSession()
    }

    private fun stopRecord() {
        if (!recording) {
            return
        }
        val previousParams = recordParams
        recordParams = null
        cameraLogic.closeSession()
        audioInput.stop()
        audioInput.release()
        audioEncoder.stop()
        videoEncoder.stop()
        audioEncoder.release()
        videoEncoder.release()
        Timber.d("muxer finish")
        muxer.stop()
        cameraLogic.createSession()
        previousParams?.let { params ->
            if (params.outputUri == null) {
                return@let
            }
            val contentValues = ContentValues().apply {
                put(MediaStore.Video.Media.IS_PENDING, 0)
            }
            contentResolver.update(params.outputUri!!, contentValues, null, null)
        }
        audioStartPts = -1L
        videoStartPts = -1L
        audioStreamIndex = -1
        videoStreamIndex = -1
        recording = false
    }



    companion object {
        private const val TAG = "FFmpegMuxActivity"
    }
}