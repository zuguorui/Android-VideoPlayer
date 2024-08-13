package com.zu.videoplayer

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.zu.videoplayer.databinding.FragmentSetStreamLocationBinding

class SetStreamLocationFragment: Fragment(), ISettingFragment {

    private lateinit var binding: FragmentSetStreamLocationBinding

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        binding = FragmentSetStreamLocationBinding.inflate(layoutInflater)
        if (Settings.fileOutputType == FILE_OUTPUT_TYPE_FILE) {
            binding.rbFile.isChecked = true
        } else {
            binding.rbStream.isChecked = true
        }

        binding.etUrl.isEnabled = Settings.fileOutputType == FILE_OUTPUT_TYPE_STREAM
        if (Settings.pushStreamUrl.isNotBlank()) {
            binding.etUrl.setText(Settings.pushStreamUrl)
        }

        binding.rbStream.setOnCheckedChangeListener { _, isChecked ->
            binding.etUrl.isEnabled = isChecked
        }
        return binding.root
    }


    override fun onResult() {
        if (binding.rbFile.isChecked) {
            Settings.fileOutputType = FILE_OUTPUT_TYPE_FILE
        } else {
            Settings.fileOutputType = FILE_OUTPUT_TYPE_STREAM
            var url = binding.etUrl.text.toString().trim()
            if (url.isNotBlank()) {
                Settings.pushStreamUrl = url
            }
        }
    }



}