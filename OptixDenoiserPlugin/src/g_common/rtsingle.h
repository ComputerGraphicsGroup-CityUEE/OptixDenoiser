#ifndef RTSINGLE_H
#define RTSINGLE_H

#include "RenderTexture.h"
class RTSingle
{
  public:
    int w,h;
    RenderTexture *front_rt;

    RTSingle();
    ~RTSingle();

    void load( int width, int height );
    RenderTexture* front();
};

#endif
