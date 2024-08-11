package com.zu.videoplayer.codec.encoder

/**
 * @author zuguorui
 * @date 2024/6/6
 * @description 编码器状态。状态切换流程是：
 * new: IDLE
 * prepare: IDLE -> PREPARED
 * start: PREPARED -> STARTED
 * pause: STARTED -> PAUSED
 * resume: PAUSED -> STARTED
 * stop: STARTED -> IDLE or PAUSED -> IDLE
 * release: * -> IDLE
 */
enum class EncoderState {
    // 无效状态，对象创建成功后未prepare时，或者stop后，会置为此状态。
    IDLE,
    // 编码器已准备好，prepared之后会置为此状态
    PREPARED,
    // 编码器正在编码中，start或者resume之后会置为此状态
    STARTED,
    // 编码器暂停，pause之后会置为此状态。
    PAUSED
}