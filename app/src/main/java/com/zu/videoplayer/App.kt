package com.zu.videoplayer

import android.app.Application
import android.content.Context
import timber.log.Timber

class App: Application() {

    override fun onCreate() {
        super.onCreate()
        context = this
        Timber.plant(Timber.DebugTree())
    }

    companion object {
        lateinit var context: Context
            private set

        var task = Task.PLAY_LOCAL_FILE
    }
}