package com.zu.videoplayer.camera

import android.annotation.SuppressLint
import android.content.Context
import android.hardware.camera2.CameraCaptureSession
import android.hardware.camera2.CameraConstrainedHighSpeedCaptureSession
import android.hardware.camera2.CameraDevice
import android.hardware.camera2.CameraManager
import android.hardware.camera2.CaptureFailure
import android.hardware.camera2.CaptureRequest
import android.hardware.camera2.CaptureResult
import android.hardware.camera2.TotalCaptureResult
import android.hardware.camera2.params.OutputConfiguration
import android.hardware.camera2.params.SessionConfiguration
import android.os.Build
import android.os.Handler
import android.os.HandlerThread
import android.util.Range
import android.util.Size
import android.view.Surface
import androidx.annotation.RequiresApi
import com.zu.videoplayer.Settings
import com.zu.videoplayer.codec.FakeMediaRecorderSurfaceProvider
import com.zu.videoplayer.util.waitCallbackResult
import timber.log.Timber
import java.util.concurrent.Executors

/**
 * @author zuguorui
 * @date 2024/1/9
 * @description 相机基础逻辑类，负责相机的启动、关闭和配置逻辑。具体配置通过[ConfigCallback]由客户端传入。
 * 为了避免阻塞用户线程，所有对外开放的相机操作都在额外的单独线程中进行。
 * 为了避免相机回调导致的多线程问题，所有相机回调都被转换为同步操作。
 */

@SuppressLint("MissingPermission")
open class BaseCameraLogic(val context: Context) {

    var configCallback: ConfigCallback? = null
    var cameraStateCallback: CameraDevice.StateCallback? = null
    var sessionStateCallback: CameraCaptureSession.StateCallback? = null
    var captureCallback: CameraCaptureSession.CaptureCallback? = null

    var currentCameraInfo: CameraInfoWrapper? = null
    var currentFps: FPS? = null
    var currentSize: Size? = null
    var currentTemplate: Int = CameraDevice.TEMPLATE_PREVIEW


    protected var internalCaptureCallback = object : CameraCaptureSession.CaptureCallback() {

        override fun onCaptureStarted(
            session: CameraCaptureSession,
            request: CaptureRequest,
            timestamp: Long,
            frameNumber: Long
        ) {
            captureCallback?.onCaptureStarted(session, request, timestamp, frameNumber)
        }

        override fun onCaptureProgressed(
            session: CameraCaptureSession,
            request: CaptureRequest,
            partialResult: CaptureResult
        ) {
            captureCallback?.onCaptureProgressed(session, request, partialResult)
        }

        override fun onCaptureCompleted(
            session: CameraCaptureSession,
            request: CaptureRequest,
            result: TotalCaptureResult
        ) {
            captureCallback?.onCaptureCompleted(session, request, result)
        }

        override fun onCaptureBufferLost(
            session: CameraCaptureSession,
            request: CaptureRequest,
            target: Surface,
            frameNumber: Long
        ) {
            Timber.w("onCaptureBufferLost: target = %s, frameNumber = %s", target.toString(), frameNumber)
            captureCallback?.onCaptureBufferLost(session, request, target, frameNumber)
        }

        override fun onCaptureFailed(
            session: CameraCaptureSession,
            request: CaptureRequest,
            failure: CaptureFailure
        ) {
            captureCallback?.onCaptureFailed(session, request, failure)
        }

        override fun onCaptureSequenceAborted(session: CameraCaptureSession, sequenceId: Int) {
            captureCallback?.onCaptureSequenceAborted(session, sequenceId)
        }

        override fun onCaptureSequenceCompleted(
            session: CameraCaptureSession,
            sequenceId: Int,
            frameNumber: Long
        ) {
            captureCallback?.onCaptureSequenceCompleted(session, sequenceId, frameNumber)
        }

        @RequiresApi(Build.VERSION_CODES.UPSIDE_DOWN_CAKE)
        override fun onReadoutStarted(
            session: CameraCaptureSession,
            request: CaptureRequest,
            timestamp: Long,
            frameNumber: Long
        ) {
            captureCallback?.onReadoutStarted(session, request, timestamp, frameNumber)
        }
    }

    protected val cameraManager: CameraManager by lazy {
        ((context.getSystemService(Context.CAMERA_SERVICE)) as CameraManager)
    }


    var camera: CameraDevice? = null
        protected set
    protected var session: CameraCaptureSession? = null
    protected var highSpeedSession: CameraConstrainedHighSpeedCaptureSession? = null

    protected var captureRequestBuilder: CaptureRequest.Builder? = null

    protected var cameraThread = HandlerThread("CameraThread").apply { start() }

    protected var cameraHandler = Handler(cameraThread.looper)

    protected var cameraExecutor = Executors.newSingleThreadExecutor()

    // 为了避免阻塞主线程，相机操作都放在单独线程
    private var cameraOperationExecutor = Executors.newSingleThreadExecutor()

    // 某些手机高帧率预览必须提供一个MediaCodec或者MediaRecorder的surface
    private val fakeSurfaceProvider = FakeMediaRecorderSurfaceProvider()

    /**
     * 启动相机
     * */
    open fun openCamera(cameraInfo: CameraInfoWrapper) {
        cameraOperationExecutor.execute {
            Timber.d("openCamera ${cameraInfo.cameraID}")
            if (!openCameraInternal(cameraInfo)) {
                Timber.e("openCamera ${cameraInfo.cameraID} failed")
                return@execute
            }
            if (!createSessionInternal()) {
                Timber.e("createSession ${cameraInfo.cameraID} failed")
                return@execute
            }

            startPreviewInternal()
        }
    }

    open fun closeCamera() {
        cameraOperationExecutor.execute {
            closeCameraInternal()
        }
    }

    open fun createSession() {
        cameraOperationExecutor.execute {
            if (!createSessionInternal()) {
                Timber.e("createSession ${camera?.id} failed")
                return@execute
            }
            startPreviewInternal()
        }
    }

    open fun closeSession() {
        cameraOperationExecutor.execute {
            closeSessionInternal()
        }
    }

    open fun startPreview() {
        cameraOperationExecutor.execute {
            startPreviewInternal()
        }
    }

    open fun stopPreview() {
        cameraOperationExecutor.execute {
            stopPreviewInternal()
        }
    }

    open fun startRepeating() {
        cameraOperationExecutor.execute {
            startRepeatingInternal()
        }
    }

    open fun stopRepeating() {
        cameraOperationExecutor.execute {
            stopRepeatingInternal()
        }
    }

    private fun configRequestBuilder() {
        Timber.d("configRequestBuilder")
        val camera = this.camera ?: return
        val configCallback = configCallback ?: return
        val fps = currentFps ?: return
        val captureSurfaceList = configCallback.getCaptureSurfaceList()
        Timber.d("capture surface list size = ${captureSurfaceList.size}")

        val target = ArrayList<Surface>().apply {
            addAll(captureSurfaceList)
        }
        val isHighSpeed = fps.type == FPS.Type.HIGH_SPEED
        if (isHighSpeed && currentTemplate == CameraDevice.TEMPLATE_PREVIEW && Settings.highSpeedPreviewExtraSurface) {
            Timber.w("add fake surface when high speed preview")
            target.add(fakeSurfaceProvider.getSurface()!!)
        }

        captureRequestBuilder = camera.createCaptureRequest(currentTemplate).apply {
            when (currentTemplate) {
                CameraDevice.TEMPLATE_RECORD,
                CameraDevice.TEMPLATE_PREVIEW -> {
                    set(CaptureRequest.CONTROL_AF_MODE,
                        CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_VIDEO)
                }
                CameraDevice.TEMPLATE_STILL_CAPTURE -> {
                    set(CaptureRequest.CONTROL_AF_MODE,
                        CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE)
                }
                else -> {
                    set(CaptureRequest.CONTROL_AF_MODE,
                        CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_VIDEO)
                }
            }
            set(CaptureRequest.CONTROL_CAPTURE_INTENT, CaptureRequest.CONTROL_CAPTURE_INTENT_VIDEO_RECORD)
            target.forEach {
                addTarget(it)
            }
            set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE, Range(fps.value, fps.value))
        }

        // 给客户端有机会做自定义配置
        configCallback?.configBuilder(captureRequestBuilder!!)
    }

    private fun openCameraInternal(cameraInfo: CameraInfoWrapper): Boolean {
        Timber.d("openCameraInternal")
        val configCallback = configCallback ?: throw IllegalStateException("ConfigCallback must be set before openCamera")
        if (camera != null) {
            closeCamera()
        }
        val finalID = if (cameraInfo.isInCameraIdList) {
            cameraInfo.cameraID
        } else {
            if (Settings.openCameraMethod == OpenCameraMethod.IN_CONFIGURATION) {
                cameraInfo.logicalID ?: cameraInfo.cameraID
            } else {
                cameraInfo.cameraID
            }
        }
        Timber.d("openDevice $finalID")

        val cameraResult: Boolean = waitCallbackResult {
            cameraManager.openCamera(finalID, object : CameraDevice.StateCallback() {
                override fun onOpened(pCamera: CameraDevice) {
                    Timber.d("camera ${pCamera.id} opened")
                    cameraStateCallback?.onOpened(pCamera)
                    currentCameraInfo = cameraInfo
                    camera = pCamera
                    it.resume(true)
                }

                override fun onClosed(pCamera: CameraDevice) {
                    Timber.d("camera ${pCamera.id} closed")
                    cameraStateCallback?.onClosed(pCamera)
                }

                override fun onDisconnected(pCamera: CameraDevice) {
                    cameraStateCallback?.onDisconnected(pCamera)
                    Timber.e("camera ${pCamera.id} disconnected")
                    pCamera.close()
                    if (camera?.id == pCamera.id) {
                        highSpeedSession = null
                        session = null
                        camera = null
                    }
                    it.resume(false)
                }

                override fun onError(pCamera: CameraDevice, error: Int) {
                    cameraStateCallback?.onError(pCamera, error)
                    val msg = when (error) {
                        ERROR_CAMERA_DEVICE -> "Fatal (device)"
                        ERROR_CAMERA_DISABLED -> "Device policy"
                        ERROR_CAMERA_IN_USE -> "Camera in use"
                        ERROR_CAMERA_SERVICE -> "Fatal (service)"
                        ERROR_MAX_CAMERAS_IN_USE -> "Maximum cameras in use"
                        else -> "Unknown"
                    }
                    val exc = RuntimeException("Camera ${cameraInfo.cameraID} error: ($error) $msg")
                    Timber.e(exc.message, exc)
                    if (camera?.id == pCamera.id) {
                        highSpeedSession = null
                        session = null
                        camera = null
                    }
                    it.resume(false)
                }
            }, cameraHandler)
        }
        return cameraResult
    }

    private fun closeCameraInternal() {
        Timber.d("closeCameraInternal ${camera?.id}")
        closeSessionInternal()
        camera?.close()
        camera = null
        currentCameraInfo = null
    }

    private fun createSessionInternal(): Boolean {
        Timber.d("createSessionInternal")
        val camera = this.camera ?: return false
        val configCallback = configCallback ?: return false

        val size = configCallback.getSize()
        currentSize = size
        val fps = configCallback.getFps()
        currentFps = fps
        currentTemplate = configCallback.getTemplate()

        val isHighSpeed = fps.type == FPS.Type.HIGH_SPEED

        val createSessionResult: Boolean = waitCallbackResult {
            val createSessionCallback = object : CameraCaptureSession.StateCallback() {
                override fun onConfigured(newSession: CameraCaptureSession) {
                    if (isHighSpeed) {
                        highSpeedSession = newSession as CameraConstrainedHighSpeedCaptureSession
                    } else {
                        session = newSession
                    }
                    sessionStateCallback?.onConfigured(newSession)
                    Timber.w("sessionConfigured")
                    it.resume(true)
                }

                override fun onConfigureFailed(session: CameraCaptureSession) {
                    sessionStateCallback?.onConfigureFailed(session)
                    val exception = RuntimeException("create session failed")
                    Timber.e("onConfigureFailed: ${exception.message}, session: $session")
                    it.resume(false)
                }

                override fun onClosed(pSession: CameraCaptureSession) {
                    Timber.w("session onClosed")
                }
            }

            val sessionSurfaceList = configCallback.getSessionSurfaceList()
            Timber.d("session surface list size = ${sessionSurfaceList.size}")
            val target = ArrayList<Surface>().apply {
                addAll(sessionSurfaceList)
            }
            if (isHighSpeed && currentTemplate == CameraDevice.TEMPLATE_PREVIEW && Settings.highSpeedPreviewExtraSurface) {
                Timber.w("add fake surface when high speed preview")
                if (!fakeSurfaceProvider.isReady) {
                    fakeSurfaceProvider.prepare(size, fps.value)
                }
                target.add(fakeSurfaceProvider.getSurface()!!)
                fakeSurfaceProvider.start()
            }

            Timber.d("total session surface count: ${target.size}")

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                val info = currentCameraInfo!!
                val outputConfigurations = ArrayList<OutputConfiguration>()
                for (surface in target) {
                    val outputConfiguration = OutputConfiguration(surface)
                    if (!isHighSpeed && info.logicalID != null && !info.isInCameraIdList && Settings.openCameraMethod == OpenCameraMethod.IN_CONFIGURATION) {
                        Timber.w("camera${info.cameraID} belong to logical camera${info.logicalID}, set physical camera")
                        outputConfiguration.setPhysicalCameraId(info.cameraID)
                    }
                    outputConfigurations.add(outputConfiguration)
                }
                val sessionType = if (isHighSpeed) {
                    SessionConfiguration.SESSION_HIGH_SPEED
                } else {
                    SessionConfiguration.SESSION_REGULAR
                }

                camera.createCaptureSession(SessionConfiguration(sessionType, outputConfigurations, cameraExecutor, createSessionCallback))

            } else {
                camera.createCaptureSession(target, createSessionCallback, cameraHandler)
            }
        }

        return createSessionResult
    }

    private fun closeSessionInternal() {
        Timber.d("closeSessionInternal ${camera?.id}")
        stopPreviewInternal()
        session?.close()
        session = null
        highSpeedSession?.close()
        highSpeedSession = null
    }

    private fun startPreviewInternal() {
        Timber.d("startPreviewInternal ${camera?.id}")
        configRequestBuilder()
        startRepeatingInternal()
    }

    private fun stopPreviewInternal() {
        Timber.d("stopPreviewInternal ${camera?.id}")
        session?.stopRepeating()
        highSpeedSession?.stopRepeating()
        fakeSurfaceProvider.stop()
        fakeSurfaceProvider.release()
    }

    private fun startRepeatingInternal() {
        val isHighSpeed = currentFps?.type?.let {
            it == FPS.Type.HIGH_SPEED
        } ?: return
        Timber.w("startRepeating, isHighSpeed = $isHighSpeed")
        if (!isHighSpeed) {
            session?.run {
                setRepeatingRequest(captureRequestBuilder!!.build(), internalCaptureCallback, cameraHandler)
            }
        } else {
            if (Build.VERSION.SDK_INT >= 28) {
                highSpeedSession?.run {
                    val highSpeedRequest = createHighSpeedRequestList(captureRequestBuilder!!.build())
                    //setRepeatingBurstRequests(highSpeedRequest, cameraExecutor, internalCaptureCallback)
                    setRepeatingBurst(highSpeedRequest, internalCaptureCallback, cameraHandler)
                }
            } else {
                Timber.e("SDK ${Build.VERSION.SDK_INT} can't create high speed preview")
            }
        }
    }

    private fun stopRepeatingInternal() {
        session?.stopRepeating()
    }


    /**
     * 更新一些参数到session，并且不会重启相机和session。
     * 调用该方法后，会通过[ConfigCallback.configBuilder]将[CaptureRequest.Builder]
     * 传给客户端，由客户端进行自定义配置，然后更新到session。
     * */
    fun updateCaptureRequestParams() {
        Timber.d("updateCaptureRequestParams")
        cameraOperationExecutor.execute {
            val builder = captureRequestBuilder ?: return@execute
            val configCallback = configCallback ?: return@execute
            configCallback.configBuilder(builder)
            startRepeatingInternal()
        }
    }

    /**
     * 更新一些参数到session，不会重启相机和session。
     * @param func 操作builder的函数，在这里对builder进行操作
     * */
    fun updateCaptureRequestParams(func: ((CaptureRequest.Builder) -> Unit)) {
        cameraOperationExecutor.execute {
            captureRequestBuilder?.let {
                func.invoke(it)
                startRepeatingInternal()
            } ?: Timber.e("requestBuilder is null, func = $func")
        }
    }

    fun updateSession(func: ((CameraCaptureSession) -> Unit)) {
        cameraOperationExecutor.execute {
            val session = if (session != null) {
                session!!
            } else if (highSpeedSession != null) {
                highSpeedSession!!
            } else {
                return@execute
            }
            func.invoke(session)
        }
    }


    /**
     * 相机配置回调，在相应时刻，会向客户端请求配置。
     * */
    interface ConfigCallback {
        fun getFps(): FPS
        fun getSize(): Size
        fun getTemplate(): Int
        fun getSessionSurfaceList(): List<Surface>
        fun getCaptureSurfaceList(): List<Surface>
        fun configBuilder(requestBuilder: CaptureRequest.Builder)
    }
}