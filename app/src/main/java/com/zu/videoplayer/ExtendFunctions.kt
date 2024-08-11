package com.zu.videoplayer

import android.graphics.Rect
import android.hardware.camera2.params.RggbChannelVector
import android.media.MediaFormat
import android.util.Range
import android.util.Rational
import android.util.Size
import java.lang.StringBuilder
import java.util.concurrent.locks.ReentrantLock
import kotlin.math.abs

inline fun <reified T> ArrayList<T>.copyToArray(): Array<T> {
    var result = Array(size){
        get(it)
    }

    return result
}

fun Size.toRational(): Rational = Rational(width, height)

fun Size.area(): Int = width * height

fun Rect.toRational(): Rational = Rational(width(), height())

fun <T> Collection<T>.toFormattedString(): String {
    return this.toFormattedString("\n")
}

fun <T> Collection<T>.toFormattedString(separator: String): String {
    return this.toFormattedString(separator) {
        it.toString()
    }
}

fun <T> Collection<T>.toFormattedString(separator: String, toStringFunction: ((T) -> String)): String {
    val sb = StringBuilder()
    var i = 0
    var it = this.iterator()
    while (it.hasNext()) {
        sb.append(toStringFunction(it.next()))
        if (i < this.size - 1) {
            sb.append(separator)
        }
        i++
    }
    return sb.toString()
}



fun <T> Array<T>.toFormattedString(): String {
    return this.toFormattedString("\n")
}

fun <T> Array<T>.toFormattedString(separator: String): String {
    return this.toFormattedString(separator) {
        it.toString()
    }
}

fun <T> Array<T>.toFormattedString(separator: String, toStringFunction: (T) -> String): String {
    val sb = StringBuilder()
    for (i in this.indices) {
        sb.append(toStringFunction(this[i]))
        if (i < this.size - 1) {
            sb.append(separator)
        }
    }
    return sb.toString()
}

fun IntArray.toFormattedString(): String {
    return this.toFormattedString("\n")
}
fun IntArray.toFormattedString(separator: String): String {
    val sb = StringBuilder()
    for (i in this.indices) {
        sb.append(this[i].toString())
        if (i < this.size - 1) {
            sb.append(separator)
        }
    }
    return sb.toString()
}

fun FloatArray.toFormattedString(): String {
    return this.toFormattedString("\n")
}

fun FloatArray.toFormattedString(points: Int): String {
    return this.toFormattedString("\n", points)
}

fun FloatArray.toFormattedString(separator: String): String {
    return this.toFormattedString(separator, 2)
}

fun FloatArray.toFormattedString(separator: String, points: Int): String {
    val formatText = "%.${points}f"
    return this.toFormattedString(separator) {
        String.format(formatText, it)
    }
}

fun FloatArray.toFormattedString(separator: String, transformer: (Float) -> String): String {
    val sb = StringBuilder()
    for (i in indices) {
        sb.append(transformer(this[i]))
        if (i < size - 1) {
            sb.append(separator)
        }
    }
    return sb.toString()
}


fun <T> lockBlock(lock: ReentrantLock, block: () -> T): T {
    lock.lock()
    val t = block()
    lock.unlock()
    return t
}

suspend fun <T> suspendLockBlock(lock: ReentrantLock, block: suspend () -> T): T {
    lock.lock()
    val t = block()
    lock.unlock()
    return t
}

fun Range<Int>.route(): Int {
    return upper - lower
}

fun isColorGainEqual(vec1: RggbChannelVector, vec2: RggbChannelVector): Boolean {
    val limit = 0.01f
    return abs(vec1.red - vec2.red) < limit && abs(vec1.greenOdd - vec2.greenOdd) < limit
            && abs(vec1.greenEven - vec2.greenEven) < limit && abs(vec1.blue - vec2.blue) < limit
}

fun MediaFormat.getIntegerSafe(key: String, defaultValue: Int = 0): Int {
    if (!containsKey(key)) {
        return defaultValue
    }
    return getInteger(key)
}

fun MediaFormat.getFloatSafe(key: String, defaultValue: Float = 0f): Float {
    if (!containsKey(key)) {
        return defaultValue
    }
    return getFloat(key)
}

fun MediaFormat.getStringSafe(key: String, defaultValue: String = ""): String {
    if (!containsKey(key)) {
        return defaultValue
    }
    return getString(key) ?: defaultValue
}

fun MediaFormat.getLongSafe(key: String, defaultValue: Long = 0L): Long {
    if (!containsKey(key)) {
        return defaultValue
    }
    return getLong(key)
}








