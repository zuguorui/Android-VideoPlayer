package com.zu.videoplayer.camera

/**
 * @author zuguorui
 * @date 2024/4/7
 * @description
 */
object FlashUtil {
    enum class FlashMode(val id: Int) {
        OFF(0),
        ON(1),
        AUTO(2),
        TORCH(3);

        companion object {
            fun valueOf(id: Int): FlashMode? {
                for (v in values()) {
                    if (v.id == id) {
                        return v
                    }
                }
                return null
            }
        }
    }

    fun getFlushModeName(mode: FlashMode): String {
        return when (mode) {
            FlashMode.OFF -> "关"
            FlashMode.ON -> "开"
            FlashMode.AUTO -> "自动"
            FlashMode.TORCH -> "常亮"
        }
    }


}