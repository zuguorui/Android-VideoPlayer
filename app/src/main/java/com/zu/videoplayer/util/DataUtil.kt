package com.zu.videoplayer.util

import android.content.Context
import android.database.Cursor
import android.provider.MediaStore
import android.util.SparseArray
import androidx.core.database.getIntOrNull
import androidx.core.util.set
import com.zu.videoplayer.bean.VideoBean
import java.io.File
import java.util.Deque

fun loadVideoFiles(context: Context): ArrayList<VideoBean> {
    var result = ArrayList<VideoBean>()
    var contentResolver = context.contentResolver
    var cursor: Cursor? = contentResolver.query(
        MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
        null,
        null,
        null,
        MediaStore.Video.Media.DEFAULT_SORT_ORDER
    )
    if (cursor?.moveToFirst() ?: false) {
        do {
            var path =
                cursor!!.getString(cursor!!.getColumnIndexOrThrow(MediaStore.Video.Media.DATA))
            var name =
                cursor!!.getString(cursor!!.getColumnIndexOrThrow(MediaStore.Video.Media.DISPLAY_NAME))
            var videoBean = VideoBean(name, path).apply {
                duration =
                    cursor!!.getInt(cursor!!.getColumnIndexOrThrow(MediaStore.Video.Media.DURATION))
                size = cursor!!.getInt(cursor!!.getColumnIndexOrThrow(MediaStore.Video.Media.SIZE))
            }
            result.add(videoBean)
        } while (cursor!!.moveToNext())
    }
    cursor?.close()

//    loadVideoFromMovieFolder()?.let {
//        result.addAll(it)
//    }
    return result
}

fun loadVideoFromMovieFolder(): ArrayList<VideoBean>? {
    var folder = File("/sdcard/Movies")
    if (!folder.exists()) {
        folder.mkdirs()
        return null
    }
    var files = folder.listFiles { file ->
        file.isFile
    }
    var result = ArrayList<VideoBean>()
    for (file in files) {
        var name = file.name
        when {
            name.endsWith("mp4", true) ||
            name.endsWith("mkv", true) ||
            name.endsWith("flv", true) -> {
                var vb = VideoBean(name, file.path)
                result.add(vb)
            }
        }
    }
    return result
}