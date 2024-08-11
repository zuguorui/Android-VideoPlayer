package com.zu.videoplayer.camera

import android.content.Context
import android.graphics.Rect
import android.graphics.SurfaceTexture
import android.util.Size
import android.view.Surface
import android.view.TextureView
import android.view.TextureView.SurfaceTextureListener
import android.view.View
import android.view.ViewGroup
import android.widget.FrameLayout
import com.zu.videoplayer.toRational
import timber.log.Timber

class TextureViewImplementation: PreviewViewImplementation {

    override var previewSize: Size? = null
        set(value) {
            field = value
            value?.let {
                innerSurfaceSize = Size(it.width, it.height)
                if (this::surfaceTexture.isInitialized) {
                    surfaceTexture.setDefaultBufferSize(it.width, it.height)
                }
            }
            Timber.d("previewSize: $value, ${value?.toRational()}")
            parent?.requestLayout()
        }

    override val surfaceSize: Size
        get() = innerSurfaceSize

    override val viewSize: Size
        get() = Size(textureView.width, textureView.height)

    override val surface: Surface
        get() = innerSurface

    private var textureView: TextureView
    private lateinit var surfaceTexture: SurfaceTexture
    private lateinit var innerSurface: Surface
    private lateinit var innerSurfaceSize: Size


    constructor(context: Context): super(context) {
        textureView = TextureView(context)
        textureView.layoutParams = FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,
            FrameLayout.LayoutParams.MATCH_PARENT
        )
        textureView.surfaceTextureListener = object : SurfaceTextureListener {
            override fun onSurfaceTextureAvailable(
                pSurfaceTexture: SurfaceTexture,
                width: Int,
                height: Int
            ) {
                val paramSize = Size(width, height)
                Timber.d("onSurfaceTextureAvailable: paramSize = $paramSize, viewSize = $viewSize , previewSize = $previewSize")
                surfaceTexture = pSurfaceTexture
                innerSurfaceSize = previewSize ?: viewSize
                surfaceTexture.setDefaultBufferSize(innerSurfaceSize.width, innerSurfaceSize.height)
                innerSurface = Surface(surfaceTexture)
                surfaceStateListener?.onSurfaceCreated(innerSurface)
            }

            override fun onSurfaceTextureSizeChanged(
                surface: SurfaceTexture,
                width: Int,
                height: Int
            ) {
                val paramSize = Size(width, height)
                Timber.d("onSurfaceTextureSizeChanged, paramSize = $paramSize, viewSize = $viewSize, previewSize = $previewSize}")
                innerSurfaceSize = previewSize ?: viewSize
                surfaceTexture.setDefaultBufferSize(innerSurfaceSize.width, innerSurfaceSize.height)
                surfaceStateListener?.onSurfaceSizeChanged(innerSurface, innerSurfaceSize.width, innerSurfaceSize.height)
            }

            override fun onSurfaceTextureDestroyed(surface: SurfaceTexture): Boolean {
                Timber.d("onSurfaceTextureDestroyed")
                surfaceStateListener?.onSurfaceDestroyed(this@TextureViewImplementation.surface)
                return true
            }

            override fun onSurfaceTextureUpdated(surface: SurfaceTexture) {

            }
        }
    }

    override fun onMeasure(bound: Rect) {
        Timber.d("onMeasure, bound = $bound")
        var widthSpec = View.MeasureSpec.makeMeasureSpec(bound.width(), View.MeasureSpec.EXACTLY)
        var heightSpec = View.MeasureSpec.makeMeasureSpec(bound.height(), View.MeasureSpec.EXACTLY)
        textureView.measure(widthSpec, heightSpec)
    }

    override fun onLayout(bound: Rect) {
        Timber.d("onLayout, bound = $bound")
        textureView.layout(bound.left, bound.top, bound.right, bound.bottom)
    }

    override fun requestAttachToParent(viewGroup: ViewGroup) {
        viewGroup.addView(textureView)
        viewGroup.postInvalidate()
    }

    override fun requestDetachFromParent(viewGroup: ViewGroup) {
        viewGroup.removeView(textureView)
        viewGroup.postInvalidate()
    }


}