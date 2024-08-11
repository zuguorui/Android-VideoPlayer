package com.zu.videoplayer.camera

import android.content.Context
import android.graphics.Rect
import android.util.Size
import android.view.Surface
import android.view.ViewGroup
import com.zu.videoplayer.toRational
import timber.log.Timber

/**
 * @author zuguorui
 * @date 2023/12/12
 * @description
 */
abstract class PreviewViewImplementation(val context: Context) {

    var surfaceStateListener: SurfaceStateListener? = null

    abstract val surface: Surface

    protected var parent: ViewGroup? = null
    var scaleType: Camera2PreviewView.ScaleType = Camera2PreviewView.ScaleType.FIT_CENTER
        set(value) {
            val same = field == value
            field = value
            if (!same) {
                parent?.requestLayout()
            }
        }

    abstract var previewSize: Size?

    abstract val surfaceSize: Size

    abstract val viewSize: Size

    private var surfaceRect = Rect()

    open fun measure(parentMeasuredWidth: Int, parentMeasuredHeight: Int) {
        val parent = parent ?: return

        var resolution = previewSize ?: kotlin.run {
            val centerX = parentMeasuredWidth / 2
            val centerY = parentMeasuredHeight / 2
            val width = parentMeasuredWidth
            val height = parentMeasuredHeight
            surfaceRect = Rect(centerX - width / 2, centerY - height / 2, centerX + width / 2, centerY + height / 2)
            //surfaceRect = Rect(0, 0, parentMeasuredWidth, parentMeasuredHeight)
            onMeasure(surfaceRect)
            return
        }

        var rotate = parent.display.rotation

        // Surface坐标系固定为自然方向，这里根据手机旋转来将其转换为view坐标方向
        var sourceRatio = when (rotate) {
            Surface.ROTATION_90, Surface.ROTATION_270 -> resolution.run { width.toFloat() / height }
            else -> resolution.run { height.toFloat() / width }
        }

        var viewRatio = parentMeasuredWidth.toFloat() / parentMeasuredHeight

        var centerX = parentMeasuredWidth / 2
        var centerY = parentMeasuredHeight / 2

        var surfaceWidth: Int
        var surfaceHeight: Int

        if (viewRatio >= sourceRatio) {
            // 图片比view窄
            when (scaleType) {
                Camera2PreviewView.ScaleType.FIT_CENTER -> {
                    surfaceHeight = parentMeasuredHeight
                    surfaceWidth = (surfaceHeight * sourceRatio).toInt()
                }
                else -> {
                    surfaceWidth = parentMeasuredWidth
                    surfaceHeight = (surfaceWidth / sourceRatio).toInt()
                }
            }
        } else {
            // 图片比view宽
            when (scaleType) {
                Camera2PreviewView.ScaleType.FIT_CENTER -> {
                    surfaceWidth = parentMeasuredWidth
                    surfaceHeight = (surfaceWidth / sourceRatio).toInt()
                }
                else -> {
                    surfaceHeight = parentMeasuredHeight
                    surfaceWidth = (surfaceHeight * sourceRatio).toInt()
                }
            }
        }

        surfaceRect.apply {
            left = centerX - surfaceWidth / 2
            right = left + surfaceWidth
            top = centerY - surfaceHeight / 2
            bottom = top + surfaceHeight
        }
        val measureSize = Size(parentMeasuredWidth, parentMeasuredHeight)
        Timber.d("measure:\n" +
                "viewSize = ${measureSize}, ratio = ${measureSize.toRational()}\n" +
                "previewSize = ${previewSize}, ratio = ${previewSize?.toRational()}\n" +
                "surfaceRect = $surfaceRect, ratio = ${surfaceRect.toRational()}")

        onMeasure(surfaceRect)
    }

    abstract fun onMeasure(bound: Rect)

    fun layout() {
        onLayout(surfaceRect)
    }

    abstract fun onLayout(bound: Rect)

    fun attachToParent(viewGroup: ViewGroup) {
        parent = viewGroup
        requestAttachToParent(viewGroup)
    }

    abstract fun requestAttachToParent(viewGroup: ViewGroup)

    fun detachFromParent() {
        surfaceStateListener = null
        val parent = parent ?: return
        requestDetachFromParent(parent)
        this.parent = null
    }

    abstract fun requestDetachFromParent(viewGroup: ViewGroup)

    interface SurfaceStateListener {
        fun onSurfaceCreated(surface: Surface)
        fun onSurfaceSizeChanged(surface: Surface, surfaceWidth: Int, surfaceHeight: Int)
        fun onSurfaceDestroyed(surface: Surface)
    }
}