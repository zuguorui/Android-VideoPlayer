package com.zu.videoplayer.camera

/**
 * @author zuguorui
 * @date 2024/1/7
 * @description
 */
/**
 * 指定物理摄像头的方式。目前有两种方式：
 * 1. 在openDevice时指定物理摄像头归属的逻辑摄像头，在createSession的OutputConfiguration中再通过setPhysicalCameraID
 * 来指定该物理摄像头。
 * 2. 直接在openDevice时指定该物理摄像头即可。不需要在createSession的OutputConfiguration中再指定摄像头
 * */
enum class OpenCameraMethod {
    // 方式2
    DIRECTLY,
    // 方式1
    IN_CONFIGURATION
}