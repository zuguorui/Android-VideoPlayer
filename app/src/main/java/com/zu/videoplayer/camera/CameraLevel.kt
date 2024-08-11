package com.zu.videoplayer.camera

import android.hardware.camera2.CameraMetadata

enum class CameraLevel(val value: Int) {
    LIMITED(CameraMetadata.INFO_SUPPORTED_HARDWARE_LEVEL_LIMITED),
    FULL(CameraMetadata.INFO_SUPPORTED_HARDWARE_LEVEL_FULL),
    LEGACY(CameraMetadata.INFO_SUPPORTED_HARDWARE_LEVEL_LEGACY),
    LEVEL_3(CameraMetadata.INFO_SUPPORTED_HARDWARE_LEVEL_3),
    EXTERNAL(CameraMetadata.INFO_SUPPORTED_HARDWARE_LEVEL_EXTERNAL);

    companion object {
        @JvmStatic
        fun valueOf(value: Int): CameraLevel {
            for (v in values()) {
                if (v.value == value) {
                    return v
                }
            }
            throw IllegalArgumentException("no value $value in CameraLevel")
        }
    }

}