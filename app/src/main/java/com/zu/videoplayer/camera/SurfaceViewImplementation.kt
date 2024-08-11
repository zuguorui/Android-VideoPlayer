package com.zu.videoplayer.camera

import android.content.Context
import android.graphics.Rect
import android.util.Size
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import android.view.ViewGroup
import android.widget.FrameLayout
import timber.log.Timber

/**
 * @author zuguorui
 * @date 2023/12/13
 * @description
 */
class SurfaceViewImplementation: PreviewViewImplementation {

    private var surfaceView: SurfaceView

    private val holder: SurfaceHolder
        get() = surfaceView.holder

    override val surface: Surface
        get() = holder.surface

    override val surfaceSize: Size
        get() = with(holder.surfaceFrame) {
            Size(this.width(), this.height())
        }

    override val viewSize: Size
        get() = Size(surfaceView.width, surfaceView.height)

    override var previewSize: Size? = null
        set(value) {
            field = value
            value?.let {
                holder.setFixedSize(it.width, it.height)
            }
            parent?.requestLayout()
        }

    private val surfaceCallback = object : SurfaceHolder.Callback {
        override fun surfaceCreated(holder: SurfaceHolder) {
            Timber.d("surfaceCreated")
            surfaceStateListener?.onSurfaceCreated(surface)
        }

        override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
            Timber.d("surfaceChanged: surfaceSize = $surfaceSize, paramSize = ${Size(width, height)}")
            val size = surfaceSize
            surfaceStateListener?.onSurfaceSizeChanged(surface, size.width, size.height)
        }

        override fun surfaceDestroyed(holder: SurfaceHolder) {
            Timber.d("surfaceDestroyed")
            surfaceStateListener?.onSurfaceDestroyed(surface)
        }
    }

    constructor(context: Context): super(context) {
        surfaceView = SurfaceView(context)
        surfaceView.layoutParams = FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.MATCH_PARENT
        )
        surfaceView.holder.addCallback(surfaceCallback)
    }

    override fun onMeasure(bound: Rect) {
        Timber.d("onMeasure: bound = $bound")
        var surfaceWidthSpec = View.MeasureSpec.makeMeasureSpec(bound.width(), View.MeasureSpec.EXACTLY)
        var surfaceHeightSpec = View.MeasureSpec.makeMeasureSpec(bound.height(), View.MeasureSpec.EXACTLY)
        surfaceView.measure(surfaceWidthSpec, surfaceHeightSpec)
    }

    override fun onLayout(bound: Rect) {
        Timber.d("onLayout: bound = $bound")
        surfaceView.layout(bound.left, bound.top, bound.right, bound.bottom)
    }

    override fun requestAttachToParent(viewGroup: ViewGroup) {
        Timber.d("requestAttachToParent")
        viewGroup.addView(surfaceView)
        viewGroup.postInvalidate()
    }

    override fun requestDetachFromParent(viewGroup: ViewGroup) {
        Timber.d("requestDetachFromParent")
        viewGroup.removeView(surfaceView)
        viewGroup.postInvalidate()
    }
}