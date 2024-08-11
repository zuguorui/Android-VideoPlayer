package com.zu.videoplayer

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import androidx.core.app.ActivityCompat
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.permissionx.guolindev.PermissionX
import com.zu.videoplayer.adapter.VideoFileAdapter
import com.zu.videoplayer.bean.VideoBean
import com.zu.videoplayer.databinding.ActivityMainBinding
import com.zu.videoplayer.util.loadVideoFiles
import io.reactivex.Observable
import io.reactivex.android.schedulers.AndroidSchedulers
import io.reactivex.schedulers.Schedulers

class MainActivity : AppCompatActivity() {

    companion object{
        private val TAG = "MainActivity"
    }


    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

//        PermissionX.init(this)
//            .permissions(
//                Manifest.permission.WRITE_EXTERNAL_STORAGE,
//                Manifest.permission.READ_EXTERNAL_STORAGE,
//                Manifest.permission.INTERNET)
//            .request(null)

        checkPermission()

        initViews()
    }

    private fun initViews() {
        binding.btnPlayLocal.setOnClickListener {
            val intent = Intent(this, FileListActivity::class.java)
            startActivity(intent)
        }

        binding.btnFfmpegMux.setOnClickListener {
            val intent = Intent(this, FFmpegMuxActivity::class.java)
            startActivity(intent)
        }
    }

    fun listPermissions(): ArrayList<String>
    {
        var result = ArrayList<String>()
        result.add(Manifest.permission.READ_EXTERNAL_STORAGE)
        result.add(Manifest.permission.WRITE_EXTERNAL_STORAGE)
        result.add(Manifest.permission.INTERNET)
        result.add(Manifest.permission.CAMERA)
        result.add(Manifest.permission.RECORD_AUDIO)
        return result
    }

    private fun checkPermission(): Boolean
    {
        val permissions = listPermissions()
        var allGet = true
        for(permission in permissions)
        {
            if(ActivityCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED)
            {
                Log.e(TAG, "permission $permission not granted")
                allGet = false
            }else{
                Log.d(TAG, "permission $permission granted")
            }
        }

        if(!allGet)
        {
            var permissionArray: Array<String> = Array(permissions.size){i: Int -> permissions[i] }
            ActivityCompat.requestPermissions(this, permissionArray, 33)
        }

        return allGet
    }



    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        when(requestCode){
            33 -> {
                for(i in grantResults.indices)
                {
                    if (grantResults[i] != PackageManager.PERMISSION_GRANTED)
                    {
                        Log.e(TAG, "permission ${permissions[i]} not granted")
                    }else{
                        Log.d(TAG, "permission ${permissions[i]} granted")
                    }
                }
//                Observable.fromCallable {
//                    loadVideoFiles(this)
//                }.subscribeOn(Schedulers.io())
//                    .observeOn(AndroidSchedulers.mainThread())
//                    .subscribe {
//                        videoList = it
//                    }
            }
        }
    }


}
