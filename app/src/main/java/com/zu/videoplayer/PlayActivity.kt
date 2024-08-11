package com.zu.videoplayer

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Handler
import android.os.Message
import android.view.*
import android.widget.SeekBar
import androidx.activity.enableEdgeToEdge
import androidx.core.view.WindowCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.WindowInsetsControllerCompat
import com.zu.videoplayer.databinding.ActivityPlayBinding
import timber.log.Timber

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

    private lateinit var binding: ActivityPlayBinding

    private var surfaceViewCallback: SurfaceHolder.Callback = object : SurfaceHolder.Callback {
        override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
            nSetSize(width, height)

        }

        override fun surfaceDestroyed(holder: SurfaceHolder) {
//            nReleaseSurface()
        }

        override fun surfaceCreated(holder: SurfaceHolder) {
            nSetSurface(holder!!.surface)
            nOpenFile(filePath!!)
            runOnUiThread {
                var duration = nGetDuration()
                binding.seekPos.max = (duration / 1000).toInt()
                binding.tvDuration.text = formatDuration(duration)
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
                binding.seekPos.progress = s
                binding.tvPos.text = formatDuration(ms)
            }

            PLAY_STATE_UPDATE -> {
                when (it.arg1) {
                    PLAY_STATE_START -> {
                        Timber.d("PLAY_STATE_START")
                        binding.btnPlay.text = "暂停"
                        isPlaying = true
                    }

                    PLAY_STATE_PAUSE -> {
                        Timber.d("PLAY_STATE_PAUSE")
                        binding.btnPlay.text = "播放"
                        isPlaying = false
                    }

                    PLAY_STATE_COMPLETE -> {
                        Timber.d("PLAY_STATE_COMPLETE")
                        binding.btnPlay.text = "播放"
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
        enableEdgeToEdge()
        binding = ActivityPlayBinding.inflate(layoutInflater)

        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        setContentView(binding.root)

        window.navigationBarColor = 0x60000000.toInt()
        window.statusBarColor = 0x60000000.toInt()


        // 控制system bars隐藏及外观
        WindowCompat.getInsetsController(window, window.decorView).apply {
            hide(WindowInsetsCompat.Type.navigationBars())
            hide(WindowInsetsCompat.Type.statusBars())
            // 控制状态栏色彩。light代表适配浅色界面，那么状态栏文本就是深色的
            // isAppearanceLightStatusBars = false
            // 控制system bars的行为。什么时候隐藏的bars会出现。这里是如果在bar区域滑动，则短暂出现，然后消失。
            systemBarsBehavior = WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
        }



        filePath = intent.getStringExtra("path")

        nInit()
        binding.btnPlay.text = "播放"
        addSurfaceView()

        initViews()

    }

    private fun initViews() {
        binding.btnPlay.setOnClickListener {
            if (isPlaying) {
                nStop()
            } else {
                nStart()
            }
        }

        binding.seekPos.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
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
        binding.rootLayout.addView(surfaceView, 0)
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
