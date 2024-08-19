package com.zu.videoplayer

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.zu.videoplayer.databinding.FragmentSetPushStreamUrlBinding

class SetPushStreamUrlFragment: Fragment(), ISettingFragment {

    private lateinit var binding: FragmentSetPushStreamUrlBinding

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        binding = FragmentSetPushStreamUrlBinding.inflate(layoutInflater)
        if (Settings.pushStreamUrl.isNotBlank()) {
            binding.etUrl.setText(Settings.pushStreamUrl)
        }
        return binding.root
    }
    override fun onResult() {
        var url = binding.etUrl.text.toString().trim()
        if (url.isNotBlank()) {
            Settings.pushStreamUrl = url
        }
    }
}