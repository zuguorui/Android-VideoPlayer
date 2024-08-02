package com.zu.videoplayer

import androidx.annotation.IntDef
import java.lang.annotation.RetentionPolicy

const val PLAY_STATE_START = 0
const val PLAY_STATE_PAUSE = 1
const val PLAY_STATE_COMPLETE = 2
const val PLAY_STATE_SEEK_START = 3
const val PLAY_STATE_SEEK_COMPLETE = 4

@IntDef(
    PLAY_STATE_START,
    PLAY_STATE_PAUSE,
    PLAY_STATE_COMPLETE,
    PLAY_STATE_SEEK_START,
    PLAY_STATE_SEEK_COMPLETE
)
@Retention(AnnotationRetention.SOURCE)
annotation class PlayState

interface PlayListener
{

    fun onProgressChanged(positionMS: Long)
    fun onPlayStateChanged(@PlayState playState: Int)
}