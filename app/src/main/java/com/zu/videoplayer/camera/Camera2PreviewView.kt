package com.zu.videoplayer.camera

import android.content.Context
import android.util.AttributeSet
import android.util.Size
import android.view.Surface
import android.widget.FrameLayout
import timber.log.Timber

/**
 * @author zuguorui
 * @date 2023/11/2
 * @description
 */
class Camera2PreviewView: FrameLayout {

    private var implementation: PreviewViewImplementation

    var implementationType: ImplementationType = ImplementationType.SURFACE_VIEW
        set (value) {
            val diff = value != field
            if (diff) {
                Timber.d("implementationType::setter, value = $value, field = $field")
            }
            field = value
            if (diff) {
                implementation.detachFromParent()
                implementation = createImplementation(value)
            }
        }

    val surface: Surface
        get() = implementation.surface

    val surfaceSize: Size
        get() = implementation.surfaceSize

    var previewSize: Size?
        get() = implementation.previewSize
        set(value) {
            implementation.previewSize = value
        }

    var scaleType: ScaleType
        set(value) {
            implementation.scaleType = value
        }
        get() = implementation.scaleType


    var surfaceStateListener: PreviewViewImplementation.SurfaceStateListener? = null
        set(value) {
            field = value
            implementation.surfaceStateListener = value
        }

    constructor(context: Context): this(context, null)

    constructor(context: Context, attributeSet: AttributeSet?): this(context, attributeSet, 0)

    constructor(context: Context, attributeSet: AttributeSet?, defStyleAttr: Int): this(context, attributeSet, defStyleAttr, 0)

    constructor(context: Context, attributeSet: AttributeSet?, defStyleAttr: Int, defStyle: Int): super(context, attributeSet, defStyleAttr, defStyle) {
        implementation = createImplementation(implementationType)
    }

    private fun createImplementation(type: ImplementationType): PreviewViewImplementation {
        val impl = if (type == ImplementationType.SURFACE_VIEW) {
            SurfaceViewImplementation(context)
        } else {
            TextureViewImplementation(context)
        }
        impl.attachToParent(this)
        impl.surfaceStateListener = surfaceStateListener
        return impl
    }

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        val measuredWidth = MeasureSpec.getSize(widthMeasureSpec)
        val measuredHeight = MeasureSpec.getSize(heightMeasureSpec)
        setMeasuredDimension(measuredWidth, measuredHeight)
        implementation.measure(measuredWidth, measuredHeight)
    }

    override fun onLayout(changed: Boolean, left: Int, top: Int, right: Int, bottom: Int) {
        implementation.layout()
    }


    enum class ScaleType {
        FILL_CENTER,
        FIT_CENTER
    }

    enum class ImplementationType {
        SURFACE_VIEW,
        TEXTURE_VIEW
    }

    companion object {
        private const val TAG = "Camera2PreviewView"
    }


}