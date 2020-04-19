package com.zu.videoplayer.util

import android.content.Context
import android.database.Cursor
import android.provider.MediaStore
import androidx.core.database.getIntOrNull
import com.zu.videoplayer.bean.VideoBean

public fun loadVideoFiles(context: Context): ArrayList<VideoBean>
{
    var result = ArrayList<VideoBean>()
    var contentResolver = context.contentResolver
    var cursor: Cursor? = contentResolver.query(MediaStore.Video.Media.EXTERNAL_CONTENT_URI, null, null, null, MediaStore.Video.Media.DEFAULT_SORT_ORDER)
    if(cursor?.moveToFirst() ?: false)
    {
        do{
            var path = cursor!!.getString(cursor!!.getColumnIndexOrThrow(MediaStore.Video.Media.DATA))
            var name = cursor!!.getString(cursor!!.getColumnIndexOrThrow(MediaStore.Video.Media.DISPLAY_NAME))
            var videoBean = VideoBean(name, path).apply {
                duration = cursor!!.getInt(cursor!!.getColumnIndexOrThrow(MediaStore.Video.Media.DURATION))
                size = cursor!!.getInt(cursor!!.getColumnIndexOrThrow(MediaStore.Video.Media.SIZE))
            }
            result.add(videoBean)
        }while(cursor!!.moveToNext())
    }
    cursor?.close()
    return result
}