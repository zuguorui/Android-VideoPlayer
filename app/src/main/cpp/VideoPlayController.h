//
// Created by 祖国瑞 on 2020-04-11.
//

#ifndef ANDROID_VIDEOPLAYER_VIDEOPLAYCONTROLLER_H
#define ANDROID_VIDEOPLAYER_VIDEOPLAYCONTROLLER_H

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "IAudioFrameProvider.h"
#include "IVideoFrameProvider.h"
#include "IMediaDataReceiver.h"

using namespace std;

class VideoPlayController: public IMediaDataReceiver, public IAudioFrameProvider, public IVideoFrameProvider{
public:
    VideoPlayController();
    ~VideoPlayController();

    bool openFile();

private:

};


#endif //ANDROID_VIDEOPLAYER_VIDEOPLAYCONTROLLER_H
