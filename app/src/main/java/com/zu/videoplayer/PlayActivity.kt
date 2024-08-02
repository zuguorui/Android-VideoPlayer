package com.zu.videoplayer

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Handler
import android.os.Message
import android.view.*
import android.widget.SeekBar
import kotlinx.android.synthetic.main.activity_play.*
import timber.log.Timber
import java.util.concurrent.atomic.AtomicBoolean

fun formatDuration(duration: Long): String {
    val totalSeconds: Int = (duration / 1000).toInt()
    val totalMinutes = totalSeconds / 60
    val seconds: Int = totalSeconds % 60
    val minutes: Int = totalMinutes % 60
    val hours: Int = totalMinutes / 60

    var result = "${if (hours == 0) "" else String.format("%02d:", hours)}${
        String.format(
            "%02d",
            minutes
        )
    }:${String.format("%02d", seconds)}"

    return result
}

class PlayActivity : AppCompatActivity() {

    private lateinit var surfaceView: SurfaceView
    private var surfaceViewCallback: SurfaceHolder.Callback = object : SurfaceHolder.Callback {
        override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {
            nSetSize(width, height)

        }

        override fun surfaceDestroyed(holder: SurfaceHolder?) {
//            nReleaseSurface()
        }

        override fun surfaceCreated(holder: SurfaceHolder?) {
            nSetSurface(holder!!.surface)
            nOpenFile(filePath!!)
            runOnUiThread {
                var duration = nGetDuration()
                seek_pos.max = (duration / 1000).toInt()
                tv_duration.text = formatDuration(duration)
            }
        }
    }

    private var handler: Handler = Handler {
        when (it.what) {
            PROGRESS_UPDATE -> {
                if (isSeeking) {
                    return@Handler true
                }
                val ms: Long = it.obj as Long
                val s: Int = (ms / 1000).toInt()
                seek_pos.progress = s
                tv_pos.text = formatDuration(ms)
            }

            PLAY_STATE_UPDATE -> {
                when (it.arg1) {
                    PLAY_STATE_START -> {
                        Timber.d("PLAY_STATE_START")
                        btn_play.text = "暂停"
                        isPlaying = true
                    }

                    PLAY_STATE_PAUSE -> {
                        Timber.d("PLAY_STATE_PAUSE")
                        btn_play.text = "播放"
                        isPlaying = false
                    }

                    PLAY_STATE_COMPLETE -> {
                        Timber.d("PLAY_STATE_COMPLETE")
                        btn_play.text = "播放"
                        isPlaying = false
                        nSeek(0)
                    }

                    PLAY_STATE_SEEK_START -> {
                        Timber.d("PLAY_STATE_SEEK_START")
                    }

                    PLAY_STATE_SEEK_COMPLETE -> {
                        Timber.d("PLAY_STATE_SEEK_COMPLETE")
                        isSeeking = false
                    }
                }
            }

        }
        return@Handler true
    }

    private val playListener = object : PlayListener {
        override fun onProgressChanged(positionMS: Long) {
            handler.sendMessage(Message().apply {
                what = PROGRESS_UPDATE
                obj = positionMS
            })
        }

        override fun onPlayStateChanged(playState: Int) {
            handler.sendMessage(Message().apply {
                what = PLAY_STATE_UPDATE
                arg1 = playState
            })
        }
    }

    private var filePath: String? = null

    private var isPlaying = false

    private var isSeeking = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        requestWindowFeature(Window.FEATURE_NO_TITLE)

        window.setFlags(
            WindowManager.LayoutParams.FLAG_FULLSCREEN,
            WindowManager.LayoutParams.FLAG_FULLSCREEN
        )
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        setContentView(R.layout.activity_play)
        filePath = intent.getStringExtra("path")

        nInit()
        btn_play.text = "播放"
        addSurfaceView()

        initViews()

    }

    private fun initViews() {
        btn_play.setOnClickListener {
            if (isPlaying) {
                nStop()
            } else {
                nStart()
            }
        }

        seek_pos.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {

            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
                isSeeking = true
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                //seekBar max is second, need translate to ms.
                var progress = seekBar!!.progress
                nSeek((progress * 1000).toLong())
            }
        })
    }

    override fun onResume() {
        super.onResume()
        nSetPlayStateListener(playListener)

    }

    override fun onPause() {
        super.onPause()
        nRemovePlayStateListener()
    }

    override fun onDestroy() {
        nStop()

        nDestroy()
        super.onDestroy()
    }

    private fun addSurfaceView() {
        surfaceView = SurfaceView(this)
        var holder = surfaceView.holder
        holder.addCallback(surfaceViewCallback)
        holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS)
        var layoutParams = ViewGroup.LayoutParams(1920, 1080)
        layoutParams.width = ViewGroup.LayoutParams.MATCH_PARENT
        layoutParams.height = ViewGroup.LayoutParams.MATCH_PARENT
        surfaceView.layoutParams = layoutParams
        root_layout.addView(surfaceView, 0)
    }


    external fun nInit()
    external fun nDestroy()
    external fun nStart()
    external fun nStop()
    external fun nOpenFile(filePath: String): Boolean
    external fun nSetSurface(surfaceView: Any)
    external fun nSetSize(width: Int, height: Int)
    external fun nSeek(position: Long)
    external fun nGetDuration(): Long
    external fun nSetPlayStateListener(listener: Any)
    external fun nRemovePlayStateListener()
    external fun nIsPlaying(): Boolean


    companion object {
        private val TAG = "PlayActivity"
        const val PROGRESS_UPDATE: Int = 1
        const val PLAY_STATE_UPDATE: Int = 2

        init {
            System.loadLibrary("native-lib")
        }
    }
}
