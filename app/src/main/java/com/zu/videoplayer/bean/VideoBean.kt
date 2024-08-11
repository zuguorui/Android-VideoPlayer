package com.zu.videoplayer.bean

import java.net.URL

class VideoBean(var name: String, var path: String)
{
    var size: Int = 0
    var duration: Int = 0
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as VideoBean

        return name == other.name
    }

    override fun hashCode(): Int {
        var result = name.hashCode()
        result = 31 * result + path.hashCode()
        result = 31 * result + size
        result = 31 * result + duration
        return result
    }


}