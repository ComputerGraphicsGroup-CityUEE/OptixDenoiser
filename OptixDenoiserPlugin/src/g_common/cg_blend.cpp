#include <stdio.h>
#include <stdlib.h>

#include "cg_blend.h"

#include <cg/cgGL.h>

CGBlend::CGBlend()
{
  memset( this, 0, sizeof(CGBlend) );
}

CGBlend::~CGBlend()
{
  int i;

  if( CG_blend ) cgDestroyProgram( CG_blend );
  if( CG_blendContext ) cgDestroyContext( CG_blendContext );
  if( CG_buf_rt ) delete[] CG_buf_rt;

  if( buf_rt )
  {
    for( i=0; i<n_buf; i++ )
      delete buf_rt[i];
    delete[] buf_rt;
  }

  if( src_rt ) delete src_rt;
  if( des_rt ) delete des_rt;
}

void CGBlend::load()
{
  auto_dimensioned = true;
}

int CGBlend::best_num( int num_of_pass )
{
  int i;
  int num_of_buffer = 1;
  for( i=2; i<=min(15,num_of_pass); i++ )
  {
    if( (num_of_pass+i-1)/i < (num_of_pass+num_of_buffer-1)/num_of_buffer )
      num_of_buffer = i;
  }

  return num_of_buffer;
}


void CGBlend::load( int width, int height, int num_of_buffer, bool bDepth, bool bStencil )
{
  int i;

  {
    if( CG_blend ) cgDestroyProgram( CG_blend );
    if( CG_blendContext ) cgDestroyContext( CG_blendContext );
    if( CG_buf_rt ) delete[] CG_buf_rt;

    if( buf_rt )
    {
      for( i=0; i<n_buf; i++ )
        delete buf_rt[i];
      delete[] buf_rt;
    }

    if( src_rt ) delete src_rt;
    if( des_rt ) delete des_rt;
  }

  w = width;
  h = height;
  n_buf = num_of_buffer<1 ? 1 : ( num_of_buffer>15 ? 15: num_of_buffer );

  glewInit();

  buf_rt = new RenderTexture*[n_buf];

  for( i=0; i<n_buf; i++ )
  {
    buf_rt[i] = new RenderTexture( w, h );
    buf_rt[i]->Initialize(true, bDepth, bStencil, false, false, 32,32,32,32);
  }

  src_rt = new RenderTexture( w, h );
  src_rt->Initialize(true, false, false, false, false, 32,32,32,32);
  des_rt = new RenderTexture( w, h );
  des_rt->Initialize(true, false, false, false, false, 32,32,32,32);


  CG_blendContext = cgCreateContext();
  CG_blendProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
  cgGLSetOptimalOptions(CG_blendProfile);

  char cg_blend_src[2048], *tstr;
    tstr  = cg_blend_src;
    tstr += sprintf( tstr, "#define n_buf %i                                \n", n_buf );
    tstr += sprintf( tstr, "void main(                                      \n" );
    tstr += sprintf( tstr, "  float2 tex_cord : TEXCOORD0,                  \n" );
    tstr += sprintf( tstr, "  uniform samplerRECT src_rt,                   \n" );
    tstr += sprintf( tstr, "  uniform samplerRECT buf_rt[n_buf],            \n" );
    tstr += sprintf( tstr, "  out float4 color : COLOR0                     \n" );
    tstr += sprintf( tstr, "){                                              \n" );
    tstr += sprintf( tstr, "  for(int i=0; i<n_buf; i++ )                   \n" );
    tstr += sprintf( tstr, "    color += f4texRECT( buf_rt[i], tex_cord );  \n" );
    tstr += sprintf( tstr, "  color += f4texRECT( src_rt, tex_cord );       \n" );
    tstr += sprintf( tstr, "}                                               \n" );
  
  CG_blend = cgCreateProgram( CG_blendContext, CG_SOURCE, cg_blend_src, CG_blendProfile, NULL, 0 );
  cgGLLoadProgram(CG_blend);

  char str[256];
  CG_buf_rt = new CGparameter[n_buf];
  for( i=0; i<n_buf; i++ )
  {
    sprintf( str, "buf_rt[%i]", i );
    CG_buf_rt[i] = cgGetNamedParameter( CG_blend, str );
  }

  CG_src_rt = cgGetNamedParameter( CG_blend, "src_rt" );
}

void CGBlend::prepare( int num_of_pass )
{
  if( auto_dimensioned )
  {
    GLint vp[4];
    glGetIntegerv( GL_VIEWPORT, vp );
    if( vp[2]>0 && vp[3]>0 )
    if( w!=vp[2] || h!=vp[3] )
      load( vp[2],vp[3] );
  }

  n_pass = num_of_pass;
  t_pass = 0;

  if( n_pass>1 )
  {
    src_rt->BeginCapture();
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    src_rt->EndCapture();
  }
}

void CGBlend::begin()
{
  if(n_pass>1)
  {
    in_rt = buf_rt[t_pass%n_buf];
    in_rt->BeginCapture();
  }
}

void CGBlend::end()
{
  int i;

  if(n_pass>1)
  {
    in_rt->EndCapture();

    if( (t_pass+1)%n_buf==0 || t_pass+1 == n_pass )
    {

      if( t_pass+1 != n_pass )
        des_rt->BeginCapture();

          glMatrixMode(GL_PROJECTION);
          glPushMatrix();
            glLoadIdentity();
              gluOrtho2D( 0, w, 0, h );

          cgGLEnableProfile( CG_blendProfile );
          cgGLBindProgram( CG_blend );

          cgGLSetTextureParameter( CG_src_rt, src_rt->GetTextureID() );
          cgGLEnableTextureParameter( CG_src_rt );

        int n_bind = min( n_buf, n_pass - t_pass/n_buf*n_buf );

        for( i=0; i<n_bind; i++ )
        {
          cgGLSetTextureParameter( CG_buf_rt[i], buf_rt[i]->GetTextureID() );
          cgGLEnableTextureParameter( CG_buf_rt[i] );
        }
        for(    ; i<n_buf; i++ )
        {
          cgGLSetTextureParameter( CG_buf_rt[i], 0 );
          cgGLEnableTextureParameter( CG_buf_rt[i] );
        }

          glBegin(GL_QUADS);
            glTexCoord2i( 0 ,0 );    glVertex2i( 0, 0 );
            glTexCoord2i( w, 0 );    glVertex2i( w, 0 );
            glTexCoord2i( w, h );    glVertex2i( w, h );
            glTexCoord2i( 0, h );    glVertex2i( 0, h );
          glEnd(); 

        for( i=0; i<n_buf; i++ )
        {
          cgGLDisableTextureParameter( CG_buf_rt[i] );
        }

          cgGLDisableTextureParameter( CG_src_rt );
          cgGLDisableProfile( CG_blendProfile );

      glPopMatrix();
      if( t_pass+1 != n_pass )
        des_rt->EndCapture();

      RenderTexture *tmp_rt;
      tmp_rt = src_rt;
      src_rt = des_rt;
      des_rt = tmp_rt;
    }
  }
  t_pass++;
}

