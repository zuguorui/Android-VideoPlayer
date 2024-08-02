//
// Created by 祖国瑞 on 2020-04-25.
//

#ifndef ANDROID_VIDEOPLAYER_IPLAYSTATELISTENER_H
#define ANDROID_VIDEOPLAYER_IPLAYSTATELISTENER_H

#include <stdlib.h>
#include <stdint.h>

#define PLAY_STATE_START 0
#define PLAY_STATE_PAUSE 1
#define PLAY_STATE_COMPLETE 2
#define PLAY_STATE_SEEK_START 3
#define PLAY_STATE_SEEK_COMPLETE 4

class IPlayStateListener {
public:
    virtual void progressChanged(int64_t currentProgress) = 0;
    virtual void playStateChanged(int state) = 0;
};


#endif //ANDROID_VIDEOPLAYER_IPLAYSTATELISTENER_H
