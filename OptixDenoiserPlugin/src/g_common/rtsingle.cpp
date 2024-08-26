#include <stdio.h>
#include <stdlib.h>

#include "g_common.h"
#include "rtsingle.h"


RTSingle::RTSingle()
{
  memset( this, 0, sizeof(RTSingle) );
}

RTSingle::~RTSingle()
{
  SAFE_DELETE( front_rt );
}

void RTSingle::load( int width, int height )
{
  w = width;
  h = height;
}

RenderTexture* RTSingle::front()
{
  if(front_rt==NULL) 
  {
    front_rt = new RenderTexture( w, h );
    front_rt->Initialize(true, false, false, false, false, 32,32,32,32);
  }
  return front_rt;
}

