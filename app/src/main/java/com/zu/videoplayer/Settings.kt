package com.zu.videoplayer

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.booleanPreferencesKey
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.intPreferencesKey
import androidx.datastore.preferences.core.stringPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import com.zu.videoplayer.camera.OpenCameraMethod
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import timber.log.Timber

/**
 * @author zuguorui
 * @date 2024/1/7
 * @description
 */

val Context.setting: DataStore<Preferences> by preferencesDataStore(name = "setting")

private val KEY_OPEN_CAMERA_METHOD = stringPreferencesKey("specify_camera_method")
private val KEY_HIGH_SPEED_PREVIEW_EXTRA_SURFACE = booleanPreferencesKey("high_speed_preview_extra_surface")
private val KEY_FILE_OUTPUT_TYPE = intPreferencesKey("file_output_type")
private val KEY_PUSH_STREAM_URL = stringPreferencesKey("push_stream_url")
private val KEY_PULL_STREAM_URL = stringPreferencesKey("pull_stream_url")

const val FILE_OUTPUT_TYPE_FILE = 0
const val FILE_OUTPUT_TYPE_STREAM = 1

object Settings {
    var openCameraMethod = OpenCameraMethod.IN_CONFIGURATION
        set(value) {
            val diff = field != value
            Timber.d("openCameraMethod, field = $field, value = $value")
            field = value
            if (diff) {
                saveData(KEY_OPEN_CAMERA_METHOD, value.name)
            }
        }

    var highSpeedPreviewExtraSurface: Boolean = true
        set(value) {
            val diff = field != value
            field = value
            if (diff) {
                saveData(KEY_HIGH_SPEED_PREVIEW_EXTRA_SURFACE, value)
            }
        }

    var fileOutputType: Int = FILE_OUTPUT_TYPE_FILE
        set(value) {
            val diff = field != value
            field = value
            if (diff) {
                saveData(KEY_FILE_OUTPUT_TYPE, value)
            }
        }

    var pushStreamUrl: String = ""
        set(value) {
            val diff = field != value
            field = value
            if (diff) {
                saveData(KEY_PUSH_STREAM_URL, value)
            }
        }

    var pullStreamUrl: String = ""
        set(value) {
            val diff = field != value
            field = value
            if (diff) {
                saveData(KEY_PULL_STREAM_URL, value)
            }
        }

    init {
        Timber.d("init start")
        // 阻塞加载配置
        runBlocking {
            Timber.d("init, runBlocking start")
            App.context.setting.data.first().let { preference ->
                openCameraMethod = preference[KEY_OPEN_CAMERA_METHOD]?.let{ name ->
                    Timber.d("read openCameraMethod = $name")
                    OpenCameraMethod.valueOf(name)
                } ?: openCameraMethod

                highSpeedPreviewExtraSurface = preference[KEY_HIGH_SPEED_PREVIEW_EXTRA_SURFACE]?.let {
                    it
                } ?: highSpeedPreviewExtraSurface

                fileOutputType = preference[KEY_FILE_OUTPUT_TYPE] ?: fileOutputType

                pushStreamUrl = preference[KEY_PUSH_STREAM_URL] ?: pushStreamUrl

                pullStreamUrl = preference[KEY_PULL_STREAM_URL] ?: pullStreamUrl

            }
            Timber.d("init, runBlocking end")
        }
        Timber.d("init end")
    }

    private fun <T> saveData(key: Preferences.Key<T>, value: T) {
        Timber.d("saveData start")
        GlobalScope.launch {
            App.context.setting.edit {
                Timber.d("saveData, key = ${key.name}, value = $value")
                it[key] = value
            }
        }
        Timber.d("saveData end")
    }


}