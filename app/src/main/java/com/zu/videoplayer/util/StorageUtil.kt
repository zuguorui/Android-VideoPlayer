package com.zu.videoplayer.util

import android.content.ContentValues
import android.content.Context
import android.net.Uri
import android.os.Build
import android.os.Environment
import android.provider.MediaStore
import timber.log.Timber
import java.io.File

/**
 * @author zuguorui
 * @date 2024/5/14
 * @description
 */


fun createPictureUri(context: Context, name: String, isPending: Boolean = false): Uri? {
    val DCIM = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM)
    val folderPath = "$DCIM/CameraUtil/picture/"
    val relativePath = folderPath.substring(folderPath.indexOf("DCIM"))
    val contentValues = ContentValues().apply {
        put(MediaStore.Images.Media.DISPLAY_NAME, name)
        put(MediaStore.Images.Media.TITLE, name)
        put(MediaStore.Images.Media.MIME_TYPE, "image/jpeg")
        put(MediaStore.Images.Media.DATA, "$folderPath$name")
        put(MediaStore.Images.Media.RELATIVE_PATH, relativePath)
        if (isPending) {
            put(MediaStore.Images.Media.IS_PENDING, 1)
        }
    }

    contentValues.run {
        Timber.d("""
                    createPictureUri:
                        display_name = ${get(MediaStore.Images.Media.DISPLAY_NAME)}
                        title = ${get(MediaStore.Images.Media.TITLE)}
                        mime_type = ${get(MediaStore.Images.Media.MIME_TYPE)}
                        data = ${get(MediaStore.Images.Media.DATA)}
                        relative_path = ${get(MediaStore.Images.Media.RELATIVE_PATH)}
                """.trimIndent())
    }
    var collectionUri = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
        MediaStore.Images.Media.getContentUri(
            MediaStore.VOLUME_EXTERNAL_PRIMARY
        )
    } else {
        MediaStore.Images.Media.EXTERNAL_CONTENT_URI
    }
    val uri = context.contentResolver.insert(collectionUri, contentValues)
    return uri
}

fun createVideoUri(context: Context, name: String, isPending: Boolean = false): Pair<Uri?, String> {
    val DCIM = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM)
    val folderPath = "$DCIM/Android-VideoPlayer/video/"
    val relativePath = folderPath.substring(folderPath.indexOf("DCIM"))
    val contentValues = ContentValues().apply {
        put(MediaStore.Video.Media.DISPLAY_NAME, name)
        put(MediaStore.Video.Media.TITLE, name)
        put(MediaStore.Video.Media.MIME_TYPE, "video/mp4")
        put(MediaStore.Video.Media.DATA, "$folderPath$name")
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            put(MediaStore.Video.Media.RELATIVE_PATH, relativePath)
        }
        if (isPending) {
            put(MediaStore.Video.Media.IS_PENDING, 1)
        }
    }

    contentValues.run {
        Timber.d("""
                    createVideoUri:
                        display_name = ${get(MediaStore.Video.Media.DISPLAY_NAME)}
                        title = ${get(MediaStore.Video.Media.TITLE)}
                        mime_type = ${get(MediaStore.Video.Media.MIME_TYPE)}
                        data = ${get(MediaStore.Video.Media.DATA)}
                        relative_path = ${get(MediaStore.Video.Media.RELATIVE_PATH)}
                """.trimIndent())
    }
    var collectionUri = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
        MediaStore.Video.Media.getContentUri(
            MediaStore.VOLUME_EXTERNAL_PRIMARY
        )
    } else {
        MediaStore.Video.Media.EXTERNAL_CONTENT_URI
    }
    val uri = context.contentResolver.insert(collectionUri, contentValues)

    return Pair(uri, "$folderPath$name")
}

fun createVideoPath(name: String): String {
    val DCIM = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM)
    val folderPath = "$DCIM/Android-VideoPlayer/video/"
    val folder = File(folderPath)
    if (!folder.exists()) {
        folder.mkdirs()
    }
    return "$folderPath$name"
}
