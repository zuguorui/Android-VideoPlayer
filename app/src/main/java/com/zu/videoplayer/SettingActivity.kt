package com.zu.videoplayer

import android.content.Intent
import android.os.Bundle
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import com.zu.videoplayer.databinding.ActivitySettingBinding

class SettingActivity : AppCompatActivity() {

    private lateinit var binding: ActivitySettingBinding
    private var fragments = ArrayList<ISettingFragment>()
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivitySettingBinding.inflate(layoutInflater)
        setContentView(binding.root)
        binding.btnConfirm.setOnClickListener {
            for (fragment in fragments) {
                fragment.onResult()
            }
            when (App.task) {
                Task.PLAY_STREAM -> {
                    val intent = Intent(this, PlayActivity::class.java)
                    intent.putExtra("url", Settings.pullStreamUrl)
                    startActivity(intent)
                }
                Task.NATIVE_ENCODE_FFMPEG_MUX -> {
                    finish()
                }
                else -> {

                }
            }
        }
        initFragments()
    }

    private fun initFragments() {
        val flag = intent.getIntExtra(KEY_SETTING_FLAGS, 0)
        val transaction = supportFragmentManager.beginTransaction()
        if ((flag and SETTING_FLAG_FILE_OUTPUT_LOCATION) != 0) {
            val fragment = SetStreamLocationFragment()
            fragments.add(fragment)
            transaction.add(R.id.fragment_container, fragment)
        }
        if ((flag and SETTING_FLAG_SET_PULL_STREAM_URL) != 0) {
            val fragment = SetPullStreamUrlFragment()
            fragments.add(fragment)
            transaction.add(R.id.fragment_container, fragment)
        }
        transaction.commitAllowingStateLoss()
    }

    companion object {
        const val SETTING_FLAG_FILE_OUTPUT_LOCATION = 1
        const val SETTING_FLAG_SET_PULL_STREAM_URL = 1 shl 1

        const val KEY_SETTING_FLAGS = "key_setting_flags"
    }
}