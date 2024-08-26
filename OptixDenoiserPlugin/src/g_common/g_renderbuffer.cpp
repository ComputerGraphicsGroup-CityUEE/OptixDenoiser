#include <stdio.h>
#include <stdlib.h>

#include "g_renderbuffer.h"



RenderBuffer::RenderBuffer( int width, int height )
{
  if( width<=0 || height<=0 )
  {
    printf( "[Error] RenderBuffer, dimension too small\n" );
    exit(-1);
  }

  if( width>4096 || height>4096 )
  {
    printf( "[Error] RenderBuffer, dimension too large\n" );
    exit(-1);
  }

  w = width;
  h = height;
  rt = NULL;
}

RenderBuffer::~RenderBuffer()
{
  if(rt)
  {
    delete rt;
  }else
  {
    glDeleteTextures(1, &texture_id);
    //glDeleteFramebuffersEXT(1, &frame_buffer_id);
  }
}

void RenderBuffer::Initialize(
  bool bShare, 
  bool bDepth, 
  bool bStencil,
  bool bMipmap, 
  bool bAnisoFilter,
  unsigned int iRBits,
  unsigned int iGBits,
  unsigned int iBBits,
  unsigned int iABits
)
{
  if( iRBits==32 && iGBits==32 && iBBits==32 && iABits==32 )
  {
    texture_target = GL_TEXTURE_RECTANGLE_NV;

    glGenTextures(1, &texture_id);
      glBindTexture(texture_target, texture_id);
        glTexParameterf( texture_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameterf( texture_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameterf( texture_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf( texture_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(texture_target, 0, GL_FLOAT_RGBA32_NV, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

  }else
  {
    rt = new RenderTexture( w,h );
    rt->Initialize(bShare, bDepth, bStencil, bMipmap, bAnisoFilter, iRBits,iGBits,iBBits,iABits);
    texture_target = rt->GetTextureTarget();
    texture_id = rt->GetTextureID();
  }
}
void CheckFramebufferStatus()
{
    GLenum status;
    status = (GLenum) glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    switch(status) {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            printf("Unsupported framebuffer format\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            printf("Framebuffer incomplete, missing attachment\n");
            break;
        //case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
        //    printf("Framebuffer incomplete, duplicate attachment\n");
        //    break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            printf("Framebuffer incomplete, attached images must have same dimensions\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            printf("Framebuffer incomplete, attached images must have same format\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            printf("Framebuffer incomplete, missing draw buffer\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            printf("Framebuffer incomplete, missing read buffer\n");
            break;
        default:
            break;
    }
}

bool RenderBuffer::BeginCapture()
{
  if( rt )
  {
    return rt->BeginCapture();
  }else
  {

    static unsigned int frame_buffer_id = 0;

    if( frame_buffer_id==0 )
    {
      glGenFramebuffersEXT(1, &frame_buffer_id);
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frame_buffer_id);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, texture_target, texture_id, 0);

    //CheckFramebufferStatus();                                                    

    glPushAttrib(GL_VIEWPORT_BIT); 
    glViewport(0, 0, w, h);

    return true;
  }
}


bool RenderBuffer::EndCapture()
{
  if( rt )
  {
    return rt->EndCapture();
  }else
  {
    glPopAttrib();
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    return true;
  }
}

void RenderBuffer::EnableTextureTarget() const
{
  glEnable(texture_target);
}







void RenderBuffer::Bind() const
{
  glBindTexture(texture_target, texture_id);   
}

unsigned int RenderBuffer::GetTextureTarget() const
{
  return texture_target;
}

unsigned int RenderBuffer::GetTextureID() const
{
  return texture_id;
}

int RenderBuffer::GetWidth() const
{
  return w;
}

int RenderBuffer::GetHeight() const
{
  return h;
}
