#ifndef G_RENDERBUFFER_H
#define G_RENDERBUFFER_H

#include "RenderTexture.h"

class RenderBuffer
{
  public:

    int w,h;

    RenderTexture *rt;
    unsigned int texture_target;
    unsigned int texture_id;

    RenderBuffer( int width, int height );
    ~RenderBuffer();
    void Initialize(
      bool bShare             = true, 
      bool bDepth             = false, 
      bool bStencil           = false,
      bool bMipmap            = false, 
      bool bAnisoFilter       = false,
      unsigned int iRBits     = 8,
      unsigned int iGBits     = 8,
      unsigned int iBBits     = 8,
      unsigned int iABits     = 8
    );

    void Bind() const;
    unsigned int GetTextureTarget() const;
    void EnableTextureTarget() const;
    bool BeginCapture();
    bool EndCapture();
    int GetHeight() const;
    int GetWidth() const;
    unsigned int GetTextureID() const;

};











#endif
