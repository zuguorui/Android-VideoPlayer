package com.zu.videoplayer.camera

/**
 * @author zuguorui
 * @date 2024/1/9
 * @description
 */
data class FPS(
    val value: Int,
    val type: Type
) {
    enum class Type {
        NORMAL,
        HIGH_SPEED
    }

    override fun hashCode(): Int {
        return value.hashCode() + type.name.hashCode()
    }

    override fun toString(): String {
        return "${value}FPS ${if (type == Type.HIGH_SPEED) "高速" else "普通"}"
    }
}
