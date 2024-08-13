package com.zu.videoplayer

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.zu.videoplayer.databinding.FragmentSetPullStreamUrlBinding

class SetPullStreamUrlFragment: Fragment(), ISettingFragment {

    private lateinit var binding: FragmentSetPullStreamUrlBinding

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        binding = FragmentSetPullStreamUrlBinding.inflate(layoutInflater)

        if (Settings.pullStreamUrl.isNotBlank()) {
            binding.etUrl.setText(Settings.pullStreamUrl)
        }

        return binding.root
    }

    override fun onResult() {
        var url = binding.etUrl.text.toString().trim()
        if (url.isNotBlank()) {
            Settings.pullStreamUrl = url
        }
    }
}