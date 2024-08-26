#include <stdio.h>
#include <stdlib.h>

#include "g_common.h"
#include "rtdouble.h"

float RTDouble::total_size = 0;

RTDouble::RTDouble():front_rt(NULL),back_rt(NULL){}
RTDouble::~RTDouble()
{
  if(front_rt==NULL) delete front_rt;
  if(back_rt==NULL)  delete back_rt;
}

void RTDouble::load( int width, int height )
{ 
  w=width; 
  h=height; 
}

void RTDouble::load( GPfm &pfm )
{
  rt_load( front(), pfm );
}

void RTDouble::load( unsigned char *dat )
{
  RenderTexture *rt = front();
  rt->Bind();
  glTexImage2D( rt->GetTextureTarget(), 0, GL_FLOAT_RGB32_NV, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, dat );
}

void RTDouble::save( GPfm &pfm )
{
  rt_save( front(), pfm );
}

RenderTexture* RTDouble::front()
{ 
  if(front_rt==NULL) 
  {
    front_rt = new RenderTexture( w, h );
    front_rt->Initialize(true, false, false, false, false, 32,32,32,32);

    front_rt->BeginCapture();
      glClearColor(0,0,0,0);
      glClear( GL_COLOR_BUFFER_BIT );
    front_rt->EndCapture();

    total_size += float(w)*h*4*4 / 1024 / 1024 ;
  }
  return front_rt;
}
RenderTexture* RTDouble::back()
{ 
  if(back_rt==NULL) 
  {
    back_rt = new RenderTexture( w, h );
    back_rt->Initialize(true, false, false, false, false, 32,32,32,32);

    back_rt->BeginCapture();
      glClearColor(0,0,0,0);
      glClear( GL_COLOR_BUFFER_BIT );
    back_rt->EndCapture();

    total_size += float(w)*h*4*4 / 1024 / 1024 ;
  }
  return back_rt;
}

void RTDouble::sub( RTDouble &a )
{
  rt_sub( front(), a.front(), back() );
  swap();
}
/*
void RTDouble::sub( RTDouble &a, RTDouble &b )
{
  rt_sub( a.front(), b.front(), front() );
}
*/
void RTDouble::swap()
{
  RenderTexture *tmp;
  tmp = front_rt;
  front_rt = back_rt;
  back_rt = tmp;
}

void RTDouble::mul( RTDouble &a, RTDouble &b )
{
  if( a.w < b.w )
    rt_mul( a.front(), b.front(), front(), b.back() );
  else
    rt_mult( b.front(), a.front(), front(), a.back() );
}

void RTDouble::column_energy( RTDouble &col_energy )
{
  rt_emul( front(), front(), back() );
  rt_colsum( back(), w, h, col_energy.front(), 0, RT_ROW );
}

void RTDouble::IsResident()
{
  GLboolean residences;
  if( front_rt!=NULL )
  {
    GLuint id = front_rt->GetTextureID();
    glAreTexturesResident( 1, &id, &residences );
    if( residences == GL_TRUE )
      printf( "FRONT : RESIDENCE\n" );
    else
      printf( "FRONT : NOT RESIDENCE\n" );
  }

  if( back_rt!=NULL )
  {
    GLuint id = back_rt->GetTextureID();
    glAreTexturesResident( 1, &id, &residences );
    if( residences == GL_TRUE )
      printf( "BACK  : RESIDENCE\n" );
    else
      printf( "BACK  : NOT RESIDENCE\n" );
  }
}


#include <GL/glew.h>
#include <GL/glut.h>

#include <cg/cgGL.h>
#include <cg/cg.h>
#include "rtsingle.h"


CGcontext cgrt_context;
  CGprofile cgrt_fragment_profile;

    CGprogram cgrt_mul;
      CGparameter cgrt_mul_src0;
      CGparameter cgrt_mul_src1;
      CGparameter cgrt_mul_col;

    CGprogram cgrt_sum;
      CGparameter cgrt_sum_src0;
      CGparameter cgrt_sum_ybound;
      RTSingle abuf[2];

    CGprogram cgrt_emul;
      CGparameter cgrt_emul_src0;
      CGparameter cgrt_emul_src1;

    CGprogram cgrt_sub;
      CGparameter cgrt_sub_src0;
      CGparameter cgrt_sub_src1;

    CGprogram cgrt_draw;
      CGparameter cgrt_draw_src0;


char *cgrt_draw_src = 
"void main(                                                    \n"
"  float2 tex_cord : TEXCOORD0,                                \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"                                                              \n"
"  out float3 color : COLOR0                                   \n"
"){                                                            \n"
"                                                              \n"
"  float3 a;                                                   \n"
"    a = f3texRECT( src0, tex_cord );                          \n"
"                                                              \n"
"  color = a+.5;                                                  \n"
"                                                              \n"
"}                                                             \n";

char *cgrt_mul_src = 
"void main(                                                    \n"
"  float2 tex_cord : TEXCOORD0,                                \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform samplerRECT src1,                                   \n"
"  uniform float col,                                          \n"
"                                                              \n"
"  out float3 color : COLOR0                                   \n"
"){                                                            \n"
"                                                              \n"
"  float3 a, b;                                                \n"
"    a = f3texRECT( src0, float2(col,tex_cord.y) );            \n"
"    b = f3texRECT( src1, tex_cord );                          \n"
"                                                              \n"
"  color = a*b;                                                \n"
"                                                              \n"
"}                                                             \n";

char *cgrt_emul_src = 
"void main(                                                    \n"
"  float2 tex_cord : TEXCOORD0,                                \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform samplerRECT src1,                                   \n"
"                                                              \n"
"  out float3 color : COLOR0                                   \n"
"){                                                            \n"
"                                                              \n"
"  float3 a, b;                                                \n"
"    a = f3texRECT( src0, tex_cord );                          \n"
"    b = f3texRECT( src1, tex_cord );                          \n"
"                                                              \n"
"  color = a*b;                                                \n"
"                                                              \n"
"}                                                             \n";

char *cgrt_sub_src = 
"void main(                                                    \n"
"  float2 tex_cord : TEXCOORD0,                                \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform samplerRECT src1,                                   \n"
"                                                              \n"
"  out float3 color : COLOR0                                   \n"
"){                                                            \n"
"                                                              \n"
"  float3 a, b;                                                \n"
"    a = f3texRECT( src0, tex_cord );                          \n"
"    b = f3texRECT( src1, tex_cord );                          \n"
"                                                              \n"
"  color = a-b;                                                \n"
"                                                              \n"
"}                                                             \n";

char *cgrt_sum_src = 
"void main(                                                           \n"
"  float2 tex_cord : TEXCOORD0,                                       \n"
"                                                                     \n"
"  uniform samplerRECT src0,                                         \n"
"  uniform float ybound,                                              \n"
"                                                                     \n"
"  out float3 color : COLOR0                                          \n"
"){                                                                   \n"
"  color=0;                                                                   \n"
"  tex_cord.y = 16 * floor(tex_cord.y)+.5;                            \n"
"  for( int i=0; i<16; i++ )                                          \n"
"  {                                                                  \n"
"    color += f3texRECT( src0, tex_cord ) * step(tex_cord.y,ybound); \n"
"    tex_cord.y+=1;                                                   \n"
"  }                                                                  \n"
"                                                                     \n"
"}                                                                    \n";

void RTDouble::draw( int x, int y )
{
    cgGLSetTextureParameter( cgrt_draw_src0, front()->GetTextureID() );
    cgGLEnableTextureParameter( cgrt_draw_src0 );

    cgGLEnableProfile( cgrt_fragment_profile );
    cgGLBindProgram( cgrt_draw );

  glBegin(GL_QUADS);
    glTexCoord2i( 0, 0 );    glVertex2i( 0+x, 0+y );
    glTexCoord2i( w, 0 );    glVertex2i( w+x, 0+y );
    glTexCoord2i( w, h );    glVertex2i( w+x, h+y );
    glTexCoord2i( 0, h );    glVertex2i( 0+x, h+y );
  glEnd();

    cgGLDisableTextureParameter(cgrt_draw_src0);
    cgGLDisableProfile( cgrt_fragment_profile );
}


void rt_colsum( 
  RenderTexture *input_rt, int w0, int h0,
  RenderTexture *output_rt, int k, RT_STATE k_state
){

  int W, H;
    W = w0;
    H = h0;

  int ybound;
  RenderTexture *curr_rt, *src_rt;

  src_rt = input_rt;
  if( H>16 )
  {
    curr_rt = abuf[0].front();
  }else
  {
    curr_rt = output_rt;
  }

  int i=0;
  while( H>16 )
  {

    ybound = H;
    H = (H+15)/16;

    curr_rt->BeginCapture();
      
      cgGLSetParameter1f( cgrt_sum_ybound, ybound );

      glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
          gluOrtho2D( 0, curr_rt->GetWidth(), 0, curr_rt->GetHeight() );


      cgGLSetTextureParameter( cgrt_sum_src0, src_rt->GetTextureID() );
      cgGLEnableTextureParameter( cgrt_sum_src0 );

      cgGLEnableProfile( cgrt_fragment_profile );
      cgGLBindProgram( cgrt_sum );

        glBegin(GL_QUADS);
          glTexCoord2i( 0, 0 );    glVertex2i( 0, 0 );
          glTexCoord2i( W, 0 );    glVertex2i( W, 0 );
          glTexCoord2i( W, H );    glVertex2i( W, H );
          glTexCoord2i( 0, H );    glVertex2i( 0, H );
        glEnd();

      cgGLDisableTextureParameter(cgrt_sum_src0);
      cgGLDisableProfile( cgrt_fragment_profile );

    curr_rt->EndCapture();

    i++;

    src_rt = curr_rt;
    curr_rt = abuf[i%2].front();
  };


  curr_rt = output_rt;
  ybound = H;
  H = (H+15)/16;

    curr_rt->BeginCapture();
      
      cgGLSetParameter1f( cgrt_sum_ybound, ybound );

      glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
          gluOrtho2D( 0, curr_rt->GetWidth(), 0, curr_rt->GetHeight() );


      cgGLSetTextureParameter( cgrt_sum_src0, src_rt->GetTextureID() );
      cgGLEnableTextureParameter( cgrt_sum_src0 );

      cgGLEnableProfile( cgrt_fragment_profile );
      cgGLBindProgram( cgrt_sum );

        glBegin(GL_QUADS);

          if( k_state == RT_COL )
          {
            glTexCoord2i( 0, 0 );    glVertex2i( k  , 0 );
            glTexCoord2i( W, 0 );    glVertex2i( k  , W );
            glTexCoord2i( W, H );    glVertex2i( k+H, W );
            glTexCoord2i( 0, H );    glVertex2i( k+H, 0 );
          }

          if( k_state == RT_ROW )
          {
            glTexCoord2i( 0, 0 );    glVertex2i( 0, k   );
            glTexCoord2i( W, 0 );    glVertex2i( W, k   );
            glTexCoord2i( W, H );    glVertex2i( W, k+H );
            glTexCoord2i( 0, H );    glVertex2i( 0, k+H );
          }
        glEnd();

      cgGLDisableTextureParameter(cgrt_sum_src0);
      cgGLDisableProfile( cgrt_fragment_profile );

    curr_rt->EndCapture();

}

//
//  Matrix subtraction, a - b = c
//
void rt_sub( 
  RenderTexture *a, 
  RenderTexture *b,
  RenderTexture *c
){
  int aw,ah, bw,bh, cw, ch;
    aw = a->GetWidth();
    ah = a->GetHeight();
    bw = b->GetWidth();
    bh = b->GetHeight();
    cw = c->GetWidth();
    ch = c->GetHeight();

    if( aw!=bw || ah!=bh )
    {
      printf( "[Error] rt_sub, source RenderTexture are of different dimension.\n" );
      exit(-1);
    }

    if( aw!=cw || ah!=ch )
    {
      printf( "[Warning] rt_sub, the target texture is of different dimension, are you intended?\n" );
    }

  c->BeginCapture();

    glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
        gluOrtho2D( 0, cw, 0, ch);

    cgGLSetTextureParameter( cgrt_sub_src0, a->GetTextureID() );
    cgGLEnableTextureParameter( cgrt_sub_src0 );

    cgGLSetTextureParameter( cgrt_sub_src1, b->GetTextureID() );
    cgGLEnableTextureParameter( cgrt_sub_src1 );

    cgGLEnableProfile( cgrt_fragment_profile );
    cgGLBindProgram( cgrt_sub );

    glBegin(GL_QUADS);
      glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
      glTexCoord2i( aw,  0 );    glVertex2i( aw,  0 );
      glTexCoord2i( aw, ah );    glVertex2i( aw, ah );
      glTexCoord2i(  0, ah );    glVertex2i(  0, ah );
    glEnd();

    cgGLDisableTextureParameter(cgrt_sub_src0);
    cgGLDisableTextureParameter(cgrt_sub_src1);
    cgGLDisableProfile( cgrt_fragment_profile );

  c->EndCapture();
}

//
//  Matrix element-wise multiplication, a (*) b = c
//
void rt_emul( 
  RenderTexture *a, 
  RenderTexture *b,
  RenderTexture *c
){
  int aw,ah, bw,bh, cw, ch;
    aw = a->GetWidth();
    ah = a->GetHeight();
    bw = b->GetWidth();
    bh = b->GetHeight();
    cw = c->GetWidth();
    ch = c->GetHeight();

    if( aw!=bw || ah!=bh )
    {
      printf( "[Error] rt_emul, source RenderTexture are of different dimension.\n" );
      exit(-1);
    }

    if( aw!=cw || ah!=ch )
    {
      printf( "[Warning] rt_emul, the target texture is of different dimension, are you intended?\n" );
    }

  c->BeginCapture();

    glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
        gluOrtho2D( 0, cw, 0, ch);

    cgGLSetTextureParameter( cgrt_emul_src0, a->GetTextureID() );
    cgGLEnableTextureParameter( cgrt_emul_src0 );

    cgGLSetTextureParameter( cgrt_emul_src1, b->GetTextureID() );
    cgGLEnableTextureParameter( cgrt_emul_src1 );

    cgGLEnableProfile( cgrt_fragment_profile );
    cgGLBindProgram( cgrt_emul );

    glBegin(GL_QUADS);
      glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
      glTexCoord2i( aw,  0 );    glVertex2i( aw,  0 );
      glTexCoord2i( aw, ah );    glVertex2i( aw, ah );
      glTexCoord2i(  0, ah );    glVertex2i(  0, ah );
    glEnd();

    cgGLDisableTextureParameter(cgrt_emul_src0);
    cgGLDisableTextureParameter(cgrt_emul_src1);
    cgGLDisableProfile( cgrt_fragment_profile );

  c->EndCapture();
}

//
//  Matrix multiplication, a' * b = c
//
void rt_mul( 
  RenderTexture *a, RenderTexture *b, RenderTexture *c,
  RenderTexture *b_buf
){

  int aw,ah, bw,bh, cw,ch;
    aw = a->GetWidth();
    ah = a->GetHeight();
    bw = b->GetWidth();
    bh = b->GetHeight();
    cw = c->GetWidth();
    ch = c->GetHeight();

  if( ah!=bh || cw!=bw || ch!=aw )
  {
    printf( "[Error] rt_mul, matrix dimension does not match\n" );
    exit(-1);
  }
  if( bw>b_buf->GetWidth() || bh>b_buf->GetHeight() )
  {
    printf( "[Error] rt_mul, buffer RenterTexture is not big enough\n" );
    exit(-1);
  }

  int k;
  for( k=0; k<aw; k++ )
  {
    b_buf->BeginCapture();

    glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
        gluOrtho2D( 0, b_buf->GetWidth(), 0, b_buf->GetHeight() );
      cgGLSetTextureParameter( cgrt_mul_src0, a->GetTextureID() );
      cgGLSetTextureParameter( cgrt_mul_src1, b->GetTextureID() );
      cgSetParameter1f( cgrt_mul_col, k+.5 );

      cgGLEnableTextureParameter( cgrt_mul_src0 );
      cgGLEnableTextureParameter( cgrt_mul_src1 );
      cgGLEnableProfile( cgrt_fragment_profile );
      cgGLBindProgram( cgrt_mul );

      glBegin(GL_QUADS);
        glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
        glTexCoord2i( bw,  0 );    glVertex2i( bw,  0 );
        glTexCoord2i( bw, bh );    glVertex2i( bw, bh );
        glTexCoord2i(  0, bh );    glVertex2i(  0, bh );
      glEnd();

    cgGLDisableTextureParameter(cgrt_mul_src0);
    cgGLDisableTextureParameter(cgrt_mul_src1);
    cgGLDisableProfile( cgrt_fragment_profile );
    b_buf->EndCapture();

    rt_colsum( b_buf, bw, bh, c, k, RT_ROW );
  }

}

//
//  Matrix multiplication, (a' * b)' = c
//
void rt_mult( 
  RenderTexture *a, RenderTexture *b, RenderTexture *c,
  RenderTexture *b_buf
){
  int aw,ah, bw,bh, cw,ch;
    aw = a->GetWidth();
    ah = a->GetHeight();
    bw = b->GetWidth();
    bh = b->GetHeight();
    cw = c->GetWidth();
    ch = c->GetHeight();

  //if( ah!=bh || cw!=bw || ch!=aw )
  if( ah!=bh || cw!=aw || ch!=bw )
  {
    printf( "[Error] rt_mul, matrix dimension does not match\n" );
    exit(-1);
  }
  if( bh>b_buf->GetHeight() )
  {
    printf( "[Error] rt_mul, buffer RenterTexture is not big enough\n" );
    exit(-1);
  }


  int k;
  for( k=0; k<aw; k++ )
  {
    b_buf->BeginCapture();

    glMatrixMode(GL_PROJECTION);

      glLoadIdentity();
        gluOrtho2D( 0, b_buf->GetWidth(), 0, b_buf->GetHeight() );
      cgGLSetTextureParameter( cgrt_mul_src0, a->GetTextureID() );
      cgGLSetTextureParameter( cgrt_mul_src1, b->GetTextureID() );

      cgSetParameter1f( cgrt_mul_col, k+.5 );

      cgGLEnableTextureParameter( cgrt_mul_src0 );
      cgGLEnableTextureParameter( cgrt_mul_src1 );
      cgGLEnableProfile( cgrt_fragment_profile );
      cgGLBindProgram( cgrt_mul );

      glBegin(GL_QUADS);
        glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
        glTexCoord2i( bw,  0 );    glVertex2i( bw,  0 );
        glTexCoord2i( bw, bh );    glVertex2i( bw, bh );
        glTexCoord2i(  0, bh );    glVertex2i(  0, bh );
      glEnd();

      cgGLDisableTextureParameter(cgrt_mul_src1);
      cgGLDisableTextureParameter(cgrt_mul_src0);
      cgGLDisableProfile( cgrt_fragment_profile );

    b_buf->EndCapture();

    rt_colsum( b_buf, bw, bh, c, k, RT_COL );
  }

}

void rt_load( RenderTexture *rt, GPfm &pfm )
{
  if( rt->GetWidth()!=pfm.w || rt->GetHeight()!=pfm.h )
  {
    printf( "[Error] rt_load, source image is not of the same dimension as RenderTexture\n" );
    exit(-1);
  }

  //rt->Bind();
  //glTexImage2D( rt->GetTextureTarget(), 0, GL_FLOAT_RGB32_NV, pfm.w, pfm.h, 0, GL_RGB, GL_FLOAT, pfm.fm );

  rt->BeginCapture();
    glDrawPixels( pfm.w, pfm.h, GL_RGB, GL_FLOAT, pfm.fm );
  rt->EndCapture();
}

void rt_save( RenderTexture *rt, GPfm &pfm )
{
  if( pfm.w!=rt->GetWidth() || pfm.h!=rt->GetHeight() )
    pfm.load( rt->GetWidth(),rt->GetHeight() );

  //rt->Bind();
  //glGetTexImage( rt->GetTextureTarget(), 0, GL_RGB, GL_FLOAT, pfm.fm );

  rt->BeginCapture();
    glReadPixels( 0, 0, pfm.w, pfm.h, GL_RGB, GL_FLOAT, pfm.fm );
  rt->EndCapture();
}



void cgErrorCallback2()
{
  CGerror lastError = cgGetError();
  if(lastError)
  {
    printf("%s\n\n", cgGetErrorString(lastError));
    printf("%s\n", cgGetLastListing(cgrt_context));
    printf("Cg error, exiting...\n");
    exit(0);
  }
}

void prepare_rtdouble( int mw )
{
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB );
  glutCreateWindow( "dummy window" );
  glewInit();
  rt_prepare( mw );
}

void rt_prepare( int mw )
{
  abuf[0].load( mw, (mw+15)/16 );
  abuf[1].load( mw, (abuf[0].h+15)/16 );

  cgrt_context = cgCreateContext();
    cgrt_fragment_profile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
    cgGLSetOptimalOptions(cgrt_fragment_profile);

  cgSetErrorCallback(cgErrorCallback2);

  cgrt_mul = cgCreateProgram( cgrt_context, CG_SOURCE, cgrt_mul_src, cgrt_fragment_profile, NULL, 0 );
    cgGLLoadProgram(cgrt_mul);
      cgrt_mul_src0 = cgGetNamedParameter( cgrt_mul, "src0" );
      cgrt_mul_src1 = cgGetNamedParameter( cgrt_mul, "src1" );
      cgrt_mul_col  = cgGetNamedParameter( cgrt_mul, "col" );

  cgrt_sum = cgCreateProgram( cgrt_context, CG_SOURCE, cgrt_sum_src, cgrt_fragment_profile, NULL, 0 );
    cgGLLoadProgram(cgrt_sum);
      cgrt_sum_src0   = cgGetNamedParameter( cgrt_sum, "src0" );
      cgrt_sum_ybound = cgGetNamedParameter( cgrt_sum, "ybound" );

  cgrt_emul = cgCreateProgram( cgrt_context, CG_SOURCE, cgrt_emul_src, cgrt_fragment_profile, NULL, 0 );
    cgGLLoadProgram(cgrt_emul);
      cgrt_emul_src0 = cgGetNamedParameter( cgrt_emul, "src0" );
      cgrt_emul_src1 = cgGetNamedParameter( cgrt_emul, "src1" );

  cgrt_sub = cgCreateProgram( cgrt_context, CG_SOURCE, cgrt_sub_src, cgrt_fragment_profile, NULL, 0 );
    cgGLLoadProgram(cgrt_sub);
      cgrt_sub_src0 = cgGetNamedParameter( cgrt_sub, "src0" );
      cgrt_sub_src1 = cgGetNamedParameter( cgrt_sub, "src1" );

  cgrt_draw = cgCreateProgram( cgrt_context, CG_SOURCE, cgrt_draw_src, cgrt_fragment_profile, NULL, 0 );
    cgGLLoadProgram(cgrt_draw);
      cgrt_draw_src0 = cgGetNamedParameter( cgrt_draw, "src0" );
}

