package com.zu.videoplayer.camera

import android.content.Context
import android.graphics.ImageFormat
import android.hardware.camera2.CameraCharacteristics
import android.hardware.camera2.CameraManager
import android.media.ImageReader
import android.util.Rational
import android.util.Size
import android.view.Surface
import com.zu.videoplayer.area
import com.zu.videoplayer.toRational
import timber.log.Timber
import java.util.ArrayDeque
import kotlin.math.abs

fun sortCamera(cameras: MutableList<CameraInfoWrapper>) {
    cameras.sortWith { o1, o2 ->
        if (o1.lensFacing == CameraCharacteristics.LENS_FACING_FRONT && o2.lensFacing != CameraCharacteristics.LENS_FACING_FRONT) {
            -1
        } else if (o1.lensFacing != CameraCharacteristics.LENS_FACING_FRONT && o2.lensFacing == CameraCharacteristics.LENS_FACING_FRONT) {
            1
        } else {
            if (o1.isLogical && !o2.isLogical) {
                -1
            } else if (!o1.isLogical && o2.isLogical) {
                1
            } else {
                if (o1.focalArray[0] > o2.focalArray[0]) {
                    1
                } else if (o1.focalArray[0] < o2.focalArray[0]) {
                    -1
                } else {
                    0
                }
            }
        }
    }
}

fun queryCameraInfo(context: Context): HashMap<String, CameraInfoWrapper> {
    val cameraManager = context.getSystemService(Context.CAMERA_SERVICE) as CameraManager
    val presentIdQueue = ArrayDeque<String>().apply {
        addAll(cameraManager.cameraIdList)
    }

    val cameraInfoMap = HashMap<String, CameraInfoWrapper>()

    val logicalIdQueue = ArrayDeque<String>()

    // 先处理通过CameraManager能查询到的
    while (presentIdQueue.isNotEmpty()) {
        val id = presentIdQueue.poll()
        if (cameraInfoMap.containsKey(id)) {
            continue
        }
        val characteristics = cameraManager.getCameraCharacteristics(id)
        val infoWrapper = CameraInfoWrapper(id, characteristics).apply {
            isInCameraIdList = true
        }
        cameraInfoMap.put(id, infoWrapper)
        if (infoWrapper.isLogical) {
            logicalIdQueue.add(id)
        }
    }

    // 然后处理隐藏的物理镜头。如果一个摄像头既能被CameraManager独立查询到，又属于逻辑镜头。
    // 那最终将它视作属于逻辑镜头，要打开它，就通过逻辑镜头打开
    while (logicalIdQueue.isNotEmpty()) {
        val logicalID = logicalIdQueue.poll()
        val logicalInfo = cameraInfoMap[logicalID] ?: continue
        for (physicalID in logicalInfo.logicalPhysicalIDs) {
            val characteristics = cameraManager.getCameraCharacteristics(physicalID)
            cameraInfoMap[physicalID]?.let {
                it.logicalID = logicalID
            } ?: kotlin.run {
                val infoWrapper = CameraInfoWrapper(physicalID, characteristics).apply {
                    isInCameraIdList = false
                    this.logicalID = logicalID
                }
                cameraInfoMap[physicalID] = infoWrapper
            }
        }
    }

    val infoList = ArrayList<CameraInfoWrapper>().apply {
        addAll(cameraInfoMap.values)
    }


    sortCamera(infoList)

    infoList.forEach {
        Timber.d(it.toString())
    }

    return cameraInfoMap
}

fun selectCameraID(cameraInfoMap: HashMap<String, CameraInfoWrapper>, facing: Int, logical: Boolean): String {
    if (cameraInfoMap.isEmpty()) {
        throw IllegalArgumentException("cameraInfoMap is empty")
    }
    var result: String? = null
    cameraInfoMap.values.forEach {
        if (it.lensFacing != facing) {
            return@forEach
        }
        if (it.isLogical != logical) {
            return@forEach
        }

        // Query the available capabilities and output formats
        val capabilities = it.characteristics.get(
            CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES)!!
        val outputFormats = it.characteristics.get(
            CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP)!!.outputFormats

        // Return cameras that support RAW capability
        if (capabilities.contains(
                CameraCharacteristics.REQUEST_AVAILABLE_CAPABILITIES_RAW) &&
            outputFormats.contains(ImageFormat.RAW_SENSOR)) {
            result = it.cameraID
        }

    }

    if (result == null) {
        result = cameraInfoMap.values.stream().findFirst()?.get()?.cameraID
    }

    return result!!
}

// 几种标准宽高比
private val standardRatios = arrayListOf(
    Rational(21, 9),
    Rational(2, 1),
    Rational(16, 9),
    Rational(4, 3),
    Rational(1, 1)
)

fun translateViewSizeToSensorSize(viewSize: Size, rotation: Int): Size {
    return when (rotation) {
        Surface.ROTATION_90, Surface.ROTATION_270 -> {
            Size(viewSize.width, viewSize.height)
        }
        else -> {
            Size(viewSize.height, viewSize.width)
        }
    }
}

fun <T> computePreviewSize(
    characteristics: CameraCharacteristics,
    targetPreviewSize: Size,
    rotation: Int,
    targetClass: Class<T>
): Size {

    var viewSize = translateViewSizeToSensorSize(targetPreviewSize, rotation)

    var viewRational = viewSize.toRational()

    val config = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP)!!

    val supportedPreviewSizes = ArrayList<Size>().apply {
        addAll(config.getOutputSizes(targetClass))
    }

    // 将分辨率按照宽高比分组，只取标准宽高比的
    val supportedRationalSizeMap = HashMap<Rational, ArrayList<Size>>()
    groupSizeByRatio(supportedPreviewSizes).entries.forEach {
        if (standardRatios.contains(it.key)) {
            supportedRationalSizeMap[it.key] = it.value
        }
    }

    // 按宽高比与view最接近来排序，这样可以避免无用的大尺寸图像
    val supportedRationals = ArrayList<Rational>().apply {
        addAll(supportedRationalSizeMap.keys)
    }
    supportedRationals.sortBy {
        abs(it.toFloat() - viewRational.toFloat())
    }

    var previewSize: Size? = null

    for (rational in supportedRationals) {
        val previewSizes = supportedRationalSizeMap[rational] ?: continue
        var pixelDiff = Int.MAX_VALUE
        var finalPreviewSize: Size? = null
        for (size in previewSizes) {
            if (abs(size.area() - viewSize.area()) < pixelDiff) {
                pixelDiff = abs(size.area() - viewSize.area())
                finalPreviewSize = size
            }
        }
        previewSize = finalPreviewSize!!
        break
    }
    return previewSize ?: throw RuntimeException("can't find preview size")
}

/**
 * 计算ImageReader的尺寸
 * @param characteristics 当前相机信息
 * @param previewSize 目标大小，这个size是相机预览大小，例如1920 * 1080。注意宽高
 * @param keepRational 是否保持与previewSize一致的宽高比。该函数始终会取与previewSize最接近的宽高比
 * @param preferResolutionType 偏爱的分辨率，>0时取最高，=0时取最接近previewSize的，<0时取最低
 * */
fun computeImageReaderSize(
    characteristics: CameraCharacteristics,
    previewSize: Size,
    format: Int = ImageFormat.YUV_420_888,
    keepRational: Boolean = true,
    preferResolutionType: Int = 0
): Size? {

    val readerSize = previewSize
    val readerRational = readerSize.toRational()

    val config = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP)!!

    val supportedReaderSizes = config.getOutputSizes(ImageReader::class.java).let {
        if (it == null || it.isEmpty()) {
            throw IllegalStateException("No analysis sizes")
        }
        val formatSizes = HashSet<Size>().apply {
            addAll(config.getOutputSizes(format))
        }
        val result = ArrayList<Size>()
        for (size in it) {
            if (formatSizes.contains(size)) {
                result.add(size)
            }
        }
        result
    }

    // 将分辨率按照宽高比分组，只取标准宽高比的
    val supportedRationalSizeMap = HashMap<Rational, ArrayList<Size>>()
    groupSizeByRatio(supportedReaderSizes).entries.forEach {
        if (standardRatios.contains(it.key)) {
            supportedRationalSizeMap[it.key] = it.value
        }
    }

    // 按宽高比与target最接近来排序，这样可以避免无用的大尺寸图像
    val supportedRationals = ArrayList<Rational>().apply {
        addAll(supportedRationalSizeMap.keys)
    }
    supportedRationals.sortBy {
        abs(it.toFloat() - readerRational.toFloat())
    }

    var result: Size? = null
    if (keepRational) {
        val sizeList = supportedRationalSizeMap[readerRational] ?: return null
        if (preferResolutionType == 0) {
            var pixelDiff = Int.MAX_VALUE
            var finalSize: Size? = null
            for (size in sizeList) {
                if (abs(size.area() - readerSize.area()) < pixelDiff) {
                    pixelDiff = abs(size.area() - readerSize.area())
                    finalSize = size
                }
            }
            result = finalSize
        } else {
            sizeList.sortBy {
                it.area()
            }
            result = if (preferResolutionType > 0) {
                sizeList.last()
            } else {
                sizeList.first()
            }
        }

    } else {
        // 直接取最接近目标宽高比的分辨率
        val sizeList = supportedRationalSizeMap[supportedRationals[0]] ?: return null
        sizeList.sortBy {
            it.area()
        }
        if (preferResolutionType == 0) {
            var pixelDiff = Int.MAX_VALUE
            var finalSize: Size? = null
            for (size in sizeList) {
                if (abs(size.area() - readerSize.area()) < pixelDiff) {
                    pixelDiff = abs(size.area() - readerSize.area())
                    finalSize = size
                }
            }
            result = finalSize
        } else {
            result = if (preferResolutionType > 0) {
                sizeList.last()
            } else {
                sizeList.first()
            }
        }
    }

    return result
}


private fun groupSizeByRatio(sizes: ArrayList<Size>): Map<Rational, ArrayList<Size>> {
    var result = HashMap<Rational, ArrayList<Size>>()
    for (size in sizes) {
        val rational = size.toRational()
        if (result[rational] == null) {
            result[rational] = ArrayList()
        }

        result[rational]?.add(size)
    }
    return result
}

fun computeRotation(sensorOrientation: Int, viewOrientation: Int, cameraFacing: Int): Int {
    val isFront = cameraFacing == CameraCharacteristics.LENS_FACING_FRONT
    val rotationSign = if (isFront) -1 else 1
    val rotation = (sensorOrientation - viewOrientation * rotationSign + 360) % 360
    return rotation
}
