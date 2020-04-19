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
import com.zu.videoplayer.adapter.VideoFileAdapter
import com.zu.videoplayer.bean.VideoBean
import com.zu.videoplayer.util.loadVideoFiles
import io.reactivex.Observable
import io.reactivex.android.schedulers.AndroidSchedulers
import io.reactivex.schedulers.Schedulers
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    companion object{
        private val TAG = "MainActivity"
    }

    private var videoList: ArrayList<VideoBean>? = null
        set(value){
            field = value
            adapter.data = value
        }

    private var adapter = VideoFileAdapter()


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        rv_files.layoutManager = LinearLayoutManager(this, RecyclerView.VERTICAL, false)
        rv_files.adapter = adapter
        adapter.itemClickListener = {
            val path = videoList!![it].path
            var intent = Intent(this, PlayActivity::class.java)
            intent.putExtra("path", path)
            startActivity(intent)
        }
        checkPermission()

        Observable.fromCallable {
            loadVideoFiles(this)
        }.subscribeOn(Schedulers.io())
            .observeOn(AndroidSchedulers.mainThread())
            .subscribe {
                videoList = it
            }
    }


    fun listPermissions(): ArrayList<String>
    {
        var result = ArrayList<String>()
        result.add(Manifest.permission.READ_EXTERNAL_STORAGE)
        result.add(Manifest.permission.WRITE_EXTERNAL_STORAGE)
        return result
    }

    fun checkPermission()
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
    }



    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
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
            }
        }
    }

}
