package com.zu.videoplayer

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.*
import kotlinx.android.synthetic.main.activity_play.*

class PlayActivity : AppCompatActivity() {

    private lateinit var surfaceView: SurfaceView
    private var surfaceViewCallback: SurfaceHolder.Callback = object : SurfaceHolder.Callback{
        override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {
            nSetSize(width, height)
        }

        override fun surfaceDestroyed(holder: SurfaceHolder?) {
            nReleaseSurface()
        }

        override fun surfaceCreated(holder: SurfaceHolder?) {
            nSetSurface(holder!!.surface)
            nStart()
        }
    }

    private var filePath: String? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        requestWindowFeature(Window.FEATURE_NO_TITLE)
        window.setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN)
        setContentView(R.layout.activity_play)
        filePath = intent.getStringExtra("path")
        nInit()
        nOpenFile(filePath!!)
        addSurfaceView()


    }

    override fun onDestroy() {
        nStop()
        nCloseFile()
        nDestroy()
        super.onDestroy()
    }

    private fun addSurfaceView()
    {
        surfaceView = SurfaceView(this)
        var holder = surfaceView.holder
        holder.addCallback(surfaceViewCallback)
        holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS)
        var layoutParams = ViewGroup.LayoutParams(1920, 1080)
        layoutParams.width = ViewGroup.LayoutParams.MATCH_PARENT
        layoutParams.height = ViewGroup.LayoutParams.MATCH_PARENT
        surfaceView.layoutParams = layoutParams
        root_layout.addView(surfaceView)
    }

    external fun nInit()
    external fun nDestroy()
    external fun nStart()
    external fun nStop()
    external fun nOpenFile(filePath: String): Boolean
    external fun nCloseFile()
    external fun nSetSurface(surfaceView: Any)
    external fun nReleaseSurface()
    external fun nSetSize(width: Int, height: Int)
    external fun nSeek(position: Long)


    companion object{
        init {
            System.loadLibrary("native-lib")
        }
    }
}
