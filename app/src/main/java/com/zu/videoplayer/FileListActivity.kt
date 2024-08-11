package com.zu.videoplayer

import android.Manifest
import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.permissionx.guolindev.PermissionX
import com.zu.videoplayer.adapter.VideoFileAdapter
import com.zu.videoplayer.bean.VideoBean
import com.zu.videoplayer.databinding.ActivityFileListBinding
import com.zu.videoplayer.util.loadVideoFiles
import com.zu.videoplayer.util.loadVideoFromMovieFolder
import io.reactivex.Observable
import io.reactivex.android.schedulers.AndroidSchedulers
import io.reactivex.schedulers.Schedulers

class FileListActivity : AppCompatActivity() {

    private lateinit var binding: ActivityFileListBinding

    private var videoList: ArrayList<VideoBean>? = null
        set(value){
            field = value
            adapter.data = value
        }

    private var adapter = VideoFileAdapter()
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityFileListBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.rvFiles.layoutManager = LinearLayoutManager(this, RecyclerView.VERTICAL, false)
        binding.rvFiles.adapter = adapter
        adapter.itemClickListener = {
            val path = videoList!![it].path
            var intent = Intent(this, PlayActivity::class.java)
            intent.putExtra("path", path)
            startActivity(intent)
        }

        Observable.fromCallable {
            loadVideoFiles(this)
        }.subscribeOn(Schedulers.io())
            .observeOn(AndroidSchedulers.mainThread())
            .subscribe {
                videoList = it
            }
    }
}