//
// Created by 祖国瑞 on 2020-04-25.
//

#ifndef ANDROID_VIDEOPLAYER_IPLAYSTATELISTENER_H
#define ANDROID_VIDEOPLAYER_IPLAYSTATELISTENER_H

#include <stdlib.h>
#include <stdint.h>

class IPlayStateListener {
public:
    virtual void progressChanged(int64_t currentProgress, bool isPlayFinished) = 0;
    virtual void playStateChanged(bool isPlay) = 0;
};


#endif //ANDROID_VIDEOPLAYER_IPLAYSTATELISTENER_H
