#include <stdio.h>
#include <stdlib.h>

///////////////////////////////////////////////
// 
// awaiting task:
// 
//   1) automatically tuning EMUX_CONST and CSUM_CONST to 
//      maximise speed.
// 
//   2) use 'best match' instead of 'first contained' as 
//      condition for buffer picking.
// 
//   3) evalute and choose the best methods of loading
//      texture data with given source
// 
//   4) introduce scheme for clearing up dummy buffers 
// 
//   5) macro class for binary operation optimization.
//      possibly using binary tree to store + - * .   
//      Such that we can prevent temp variables from long
//      binary operator statement.
// 
//   6) use GPf4 instead of GPfm as parameter 
// 
//   7) cg version partial sort a vector to give sorted index
// 
//   8) extends matrix dimension bound
// 
//   9) introduce 'offset scheme' so that get() function
//      can really return part of a big texture, instead
//      of trival copying.
// 
//  10) verify to see whether cg 4x4 matrix mul speed the
//      same as four dot or not.
// 
//  11) better organize FLOAT3 and FLOAT4 function prototype.
// 
//  12) introduce scalar state.
// 
///////////////////////////////////////////////


///////////////////////////////////////////////
// 
// this two parameters should be set to favour
// the most demanding matrix multiplication
// therefore, smallest dimension value k = EMUX_CONST^m * CSUM_CONST^n
// with EMUX_CONST larger.
// we should be able to determined them at run-time
// 
#define EMUX_CONST 16
#define CSUM_CONST 16
// 
///////////////////////////////////////////////

#include "rtm.h"

int RTm::N_RTBUF = 0;
RenderBuffer *RTm::RTBUF[RTM_MAXBUF];
int RTm::RTBUF_read_access[RTM_MAXBUF];
float RTm::curr_size = 0;
float RTm::peak_size = 0;
int RTm::sum_pass = 0;


RTm::RTm()
{
  w = 0;
  h = 0;
  ts = false;
  ls = false;
  ps = false;
  buf_id = -1;
}

RTm::RTm( const RTm &a )
{
  w = a.w;
  h = a.h;
  ts = a.ts;
  ls = a.ls;
  ps = a.ps;
  buf_id = QueryReadBuf( a.buf_id );
}

void RTm::kill()
{
  w = 0;
  h = 0;
  ts = false;
  ls = false;
  ps = false;
  FreeBuf( buf_id );
  buf_id = -1;
}

RTm::~RTm()
{
  kill();
}

RTm& RTm::operator=( const RTm &a )
{
  if( this == &a )
    return *this;

  kill();
  w = a.w;
  h = a.h;
  ts = a.ts;
  ls = a.ls;
  ps = a.ps;
  buf_id = QueryReadBuf( a.buf_id );
  return *this;
}

void RTm::set( const RTm &a, int x, int y )
{
  RTm &c = *this;
  if( x+a.w>w || y+a.h>h )
  {
    printf( "[Error] RTm::set, dimension exceeded.\n" );
    exit(-1);
  }

  if( a.ps != ps )
  {
    printf( "[Error] RTm::set, packed and non-packed mismatch.\n" );
    exit(-1);
  }


  int anc = a.ps && ( a.ts != c.ts );
  RTM__draw( 
    a.rt(), 0,0, a.w,a.h, a.ts,
    c.rt(), x,y, a.w,a.h, c.ts, anc
  );
}

RTm RTm::get( int x, int y, int cw, int ch )
{
  RTm &a = *this;
  RTm c;
  c.load( cw,ch );
  c.ps = ps;


  int anc = a.ps && ( a.ts != c.ts );
  RTM__draw( 
    a.rt(), x,y, c.w,c.h, a.ts,
    c.rt(), 0,0, c.w,c.h, c.ts, anc
  );

  return c;
}

RTm& RTm::ground_state()
{
  //if( ts || ls || w!=rt()->GetWidth() || h!=rt()->GetHeight() )
  if( ts || ls )
  {
    RTm &a = *this;
    RTm c;
      c.load( w,h, true );
      c.ps = ps;

      int anc = a.ps && ( a.ts != c.ts );
      RTM__draw(
        a.rt(), 0,0, a.w,a.h, a.ts,
        c.rt(), 0,0, a.w,a.h, c.ts, anc
      );

    *this = c;
  }
  return *this;
}

void RTm::save( GPfm &pfm )
{
  pfm.load( w, h );

  ground_state();


  rt()->BeginCapture();
    glReadPixels( 0, 0, w, h, GL_RGB, GL_FLOAT, pfm.fm );
  rt()->EndCapture();


  //GPf4 pf4;
  //  pf4.load( w,h);

  //  rt()->Bind();
  //  glGetTexImage( rt()->GetTextureTarget(), 0, GL_RGBA, GL_FLOAT, pf4.fm );


  //FLOAT3 *t0 = pfm.fm;
  //FLOAT4 *t1 = pf4.fm;
  //int i, l;
  //l = w*h;
  //for( i=0; i<l; i++, t0++, t1++ )
  //  memcpy( t0, t1, sizeof(FLOAT3) );


  //int i,j;
  //for( j=0; j<h; j++ )
  //  for( i=0; i<w; i++ )
  //    pfm.pm[j][i] = float(rand())/RAND_MAX;
}

void RTm::save( const char *spath )
{
  GPfm pfm;
  save(pfm);
  pfm.save( spath );
}

void RTm::packed_load( GPf1 &pf1 )
{
  packed_load( pf1.w, pf1.h, pf1.fm );
}

void RTm::packed_load( int width, int height, bool state_free )
{
  load( (width+1)/2, (height+1)/2, state_free );
  ps = true;
}


void RTm::load( int width, int height, bool state_free )
{
  kill();
  w = width;
  h = height;

  int i;
  
  for( i=0; i<N_RTBUF; i++ )
  {
    if( RTBUF_read_access[i] == 0 )
    {
      if( RTBUF[i] && RTBUF[i]->GetWidth() == w && RTBUF[i]->GetHeight() == h )
      {
        ts = false;
        buf_id = QueryReadBuf(i);

        rt()->BeginCapture();
        glClearColor(0,0,0,0);
        glClear( GL_COLOR_BUFFER_BIT );
        rt()->EndCapture();
        
        return;
      }
      
      if( !state_free )
      {
        if( RTBUF[i] && RTBUF[i]->GetWidth() >= w && RTBUF[i]->GetHeight() >= h )
        {
          ts = false;
          buf_id = QueryReadBuf(i);
          return;
        }

        if( RTBUF[i] && RTBUF[i]->GetWidth() >= h && RTBUF[i]->GetHeight() >= w )
        {
          ts = true;
          buf_id = QueryReadBuf(i);
          return;
        }
      }
    }
  }
  
  int id = N_RTBUF++;
  RTBUF[id] = new RenderBuffer( w, h );
  RTBUF[id]->Initialize(true, false, false, false, false, 32,32,32,32);
  curr_size += float(w)*h*4*4 / 1024 / 1024 ;
  if( curr_size > peak_size )
    peak_size = curr_size;
  
  ts = false;
  buf_id = QueryReadBuf(id);

  rt()->BeginCapture();
  glClearColor(0,0,0,0);
  glClear( GL_COLOR_BUFFER_BIT );
  rt()->EndCapture();
}

RTm RTm::operator+( const RTm &b ) const
{
  if( buf_id==-1 || b.buf_id==-1 )
  {
    printf( "[Error] RTm::operator+, trying to access RTm before any initialization\n" );
    exit(-1);
  }

  if( w!=b.w || h!=b.h )
  {
    printf( "[Error] RTm::operator+, matrix dimension didn't match.\n" );
    exit(-1);
  }

  if( ps != b.ps )
  {
    printf( "[Error] RTm::operator+, packed and non-packed mismatch.\n" );
    exit(-1);
  }

  FLOAT4 sb = FLOAT4(1,1,1,1);


  const RTm &a = *this;
  RTm c;
    c.load(w,h);
    c.ps = ps;

  if( ps )
  {

    if( a.ts ^ b.ts )
    {
      RTM__a_add_bt_packed( a.rt(), a.ts, b.rt(), sb, c.rt(), c.w, c.h, c.ts );
    }else
    {
      if( a.ts == c.ts )
        RTM__a_add_b( a.rt(), a.ts, b.rt(), sb, c.rt(), c.w, c.h, c.ts );
      else
        RTM__at_add_bt_packed( a.rt(), a.ts, b.rt(), sb, c.rt(), c.w, c.h, c.ts );
    }
  }else
  {
    if( a.ts ^ b.ts )
      RTM__a_add_bt( a.rt(), a.ts, b.rt(), sb, c.rt(), c.w, c.h, c.ts );
    else
      RTM__a_add_b ( a.rt(), a.ts, b.rt(), sb, c.rt(), c.w, c.h, c.ts );
  }
   
  return c;
}

RTm RTm::operator-( const RTm &b ) const
{
  if( buf_id==-1 || b.buf_id==-1 )
  {
    printf( "[Error] RTm::operator-, trying to access RTm before any initialization\n" );
    exit(-1);
  }

  if( w!=b.w || h!=b.h )
  {
    printf( "[Error] RTm::operator-, matrix dimension didn't match.\n" );
    exit(-1);
  }

  if( ps != b.ps )
  {
    printf( "[Error] RTm::operator-, packed and non-packed mismatch.\n" );
    exit(-1);
  }

  FLOAT4 sb = FLOAT4(-1,-1,-1,-1);


  const RTm &a = *this;
  RTm c;
    c.load(w,h);
    c.ps = ps;

  if( ps )
  {

    if( a.ts ^ b.ts )
    {
      RTM__a_add_bt_packed( a.rt(), a.ts, b.rt(), sb, c.rt(), c.w, c.h, c.ts );
    }else
    {
      if( a.ts == c.ts )
        RTM__a_add_b( a.rt(), a.ts, b.rt(), sb, c.rt(), c.w, c.h, c.ts );
      else
        RTM__at_add_bt_packed( a.rt(), a.ts, b.rt(), sb, c.rt(), c.w, c.h, c.ts );
    }
  }else
  {
    if( a.ts ^ b.ts )
      RTM__a_add_bt( a.rt(), a.ts, b.rt(), sb, c.rt(), c.w, c.h, c.ts );
    else
      RTM__a_add_b ( a.rt(), a.ts, b.rt(), sb, c.rt(), c.w, c.h, c.ts );
  }

  return c;
}

RTm RTm::operator*( const RTm &b ) const
{
  if( buf_id==-1 || b.buf_id==-1 )
  {
    printf( "[Error] RTm::operator*, trying to access RTm before any initialization\n" );
    exit(-1);
  }

  if( w!=b.h )
  {
    printf( "[Error] RTm::operator*, matrix dimension didn't match.\n" );
    exit(-1);
  }

  if( ps != b.ps )
  {
    printf( "[Error] RTm::operator*, packed and non-packed mismatch.\n" );
    exit(-1);
  }

  RTm C;
  {
    RTm &A = (RTm&)*this;
    RTm &B = (RTm&)b;
    
    if( A.h > B.w ) 
    {
      C = ~(~B * ~A);
    }else
    {
      //C.load( A.h, B.w, true );
      //C = ~C;
      C.load( B.w, A.h );
      C.ps = ps;


      int xh = A.h * (( A.w +EMUX_CONST-1 )/EMUX_CONST);
      if( xh > 4096 )
      {
        int i;
        RTm a;
        RTm b;

        if( A.w%4!= 0 )
        {
          printf( "[Error] RTm::operator*, not implemented yet.\n" );
          exit(-1);
        }

        a = A.get( 0,0, A.w/4, A.h );
        b = B.get( 0,0, B.w, B.h/4 );
        C = a*b;
        for( i=1; i<4; i++ )
        {
          a = A.get( A.w/4 * i,0, A.w/4, A.h );
          b = B.get( 0,B.h/4 * i, B.w, B.h/4 );
          C = C+a*b;
        }

        return C;
      }

      if( A.w > EMUX_CONST )
      {
        int n;
          n = A.h;

        static RTm _mul_buf;
        if( _mul_buf.buf_id != -1 )
          RTBUF_read_access[_mul_buf.buf_id] = 0;

        RTm src, des;

          src.load( C.w, n * (( A.w +EMUX_CONST-1 )/EMUX_CONST) );
          src.ps = ps;
          src.emux_krc( A, 0,0, B, 0,0, 0,0, 0 );

          if( 
            ( _mul_buf.w < src.w && _mul_buf.h<src.h ) ||
            ( _mul_buf.w < src.h && _mul_buf.h<src.w )
          ){
            _mul_buf = src;
          }else
          {
            RTBUF_read_access[_mul_buf.buf_id] +=1;
          }

          while( src.h/n > CSUM_CONST )
          {
            des.load( src.w, n * ((src.h/n+CSUM_CONST-1)/CSUM_CONST) );
            des.ps = ps;
            des.csumx( src, 0,0,0,0, src.w, src.h/n, n );
            src = des;
          }

          C.csumx( src, 0,0,0,0, src.w, src.h/n, n );

      }else
      {
        C.emux_krc( A, 0,0, B, 0,0, 0,0, 0 );
      }
    }
  }
  
  return C;

}

RTm RTm::operator~() const
{
  if( buf_id==-1 )
  {
    printf( "[Error] RTm::operator~, trying to access RTm before any initialization\n" );
    exit(-1);
  }

  RTm b;
    b.w = h;
    b.h = w;
    b.ts = !ts;
    b.ps = ps;
    b.buf_id = QueryReadBuf(buf_id);
  return b;
}

int RTm::QueryReadBuf( int id )
{
  RTBUF_read_access[id]++;
  return id;
}

void RTm::FreeBuf( int id )
{
  if( id>=0 && RTBUF_read_access[id] > 0 )
    RTBUF_read_access[id]--;
}

void RTm::debug_print()
{
  int i;
  for( i=0; i<N_RTBUF; i++ )
    if( RTBUF_read_access[i]!=-1 )
    printf( "%i ", RTBUF_read_access[i] );
  printf( "\n" );

  printf( "RTm::curr_size = %f MB\n", curr_size );
  printf( "RTm::peak_size = %f MB\n", peak_size );
  printf( "RTm::sum_pass = %i \n", sum_pass );
}

void RTm::garbage_collect()
{
  int i;
  for( i=0; i<N_RTBUF; i++ )
    if( RTBUF_read_access[i]==0 )
    {
      curr_size -= float(RTBUF[i]->GetWidth())*RTBUF[i]->GetHeight()*4*4 / 1024 / 1024 ;
      delete RTBUF[i];
      RTBUF[i] = NULL;
      RTBUF_read_access[i] = -1;
    }
  peak_size = curr_size;
}



RenderBuffer* RTm::rt() const
{
  if( buf_id == -1 )
  {
    printf( "[Error] RTm::rt(), no buffer is assigned.\n" );
    exit(-1);
  }

  return RTBUF[buf_id];
}


#include <GL/glew.h>
#include <GL/glut.h>

#include <cg/cgGL.h>
#include <cg/cg.h>


char *rtm_draw_src = 
"void main(                                                    \n"
"  float2 tex_cord : TEXCOORD0,                                \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform float4 scale,                                       \n"
"  uniform float anc,                                          \n"
"                                                              \n"
"  out float4 color : COLOR0                                   \n"
"){                                                            \n"
"                                                              \n"
"  color = f4texRECT( src0, tex_cord ) * scale;                \n"
"                                                              \n"
"  color = (1-anc)*color + anc*color.xzyw;                     \n"
"}                                                             \n";

char *rtm_unpack_src = 
"void main(                                                    \n"
"  float2 t1 : TEXCOORD0,                                      \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"                                                              \n"
"  out float4 color : COLOR0                                   \n"
"){                                                            \n"
"                                                              \n"
"  float2 m2 = floor(fmod(t1,2));                              \n"
"  float m = 2*m2.y + m2.x;                                    \n"
"  float4 mask = step( abs(float4(0,1,2,3)-m), 0.5 );          \n"
"  float2 t0 = floor(floor(t1)/2)+.5;                          \n"
"  float4 a = f4texRECT( src0, t0 );                           \n"
"  color = dot(mask,a);                                        \n"
"                                                              \n"
"}                                                             \n";

char *rtm_a_add_b_src = 
"void main(                                                    \n"
"  float2 tex_cord : TEXCOORD0,                                \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform samplerRECT src1,                                   \n"
"  uniform float4 sb,                                          \n"
"                                                              \n"
"  out float4 color : COLOR0                                   \n"
"){                                                            \n"
"                                                              \n"
"  float4 a, b;                                                \n"
"    a = f4texRECT( src0, tex_cord );                          \n"
"    b = f4texRECT( src1, tex_cord );                          \n"
"                                                              \n"
"  color = a + sb*b;                                           \n"
"                                                              \n"
"}                                                             \n";

char *rtm_a_add_bt_src = 
"void main(                                                    \n"
"  float2 tex_cord : TEXCOORD0,                                \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform samplerRECT src1,                                   \n"
"  uniform float4 sb,                                          \n"
"                                                              \n"
"  out float4 color : COLOR0                                   \n"
"){                                                            \n"
"                                                              \n"
"  float4 a, b;                                                \n"
"    a = f4texRECT( src0, tex_cord );                          \n"
"    b = f4texRECT( src1, tex_cord.yx );                       \n"
"                                                              \n"
"  color = a + sb*b;                                           \n"
"                                                              \n"
"}                                                             \n";

char *rtm_a_add_bt_packed_src = 
"void main(                                                    \n"
"  float2 tex_cord : TEXCOORD0,                                \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform samplerRECT src1,                                   \n"
"  uniform float4 sb,                                          \n"
"                                                              \n"
"  out float4 color : COLOR0                                   \n"
"){                                                            \n"
"                                                              \n"
"  float4 a, b;                                                \n"
"    a = f4texRECT( src0, tex_cord );                          \n"
"    b = f4texRECT( src1, tex_cord.yx );                       \n"
"                                                              \n"
"  color = a + sb*b.xzyw;                                      \n"
"                                                              \n"
"}                                                             \n";

char *rtm_at_add_bt_packed_src = 
"void main(                                                    \n"
"  float2 tex_cord : TEXCOORD0,                                \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform samplerRECT src1,                                   \n"
"  uniform float4 sb,                                          \n"
"                                                              \n"
"  out float4 color : COLOR0                                   \n"
"){                                                            \n"
"                                                              \n"
"  float4 a, b;                                                \n"
"    a = f4texRECT( src0, tex_cord.yx );                       \n"
"    b = f4texRECT( src1, tex_cord.yx );                       \n"
"                                                              \n"
"  color = a.xzyw + sb*b.xzyw;                                 \n"
"                                                              \n"
"}                                                             \n";

char *rtm_emux_kcc_src = 
"void main(                                                    \n"
"  float2 tc : TEXCOORD0,                                      \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform samplerRECT src1,                                   \n"
"  uniform float k,                                            \n"
"  uniform float ybound,                                       \n"
"  uniform float blk,                                          \n"
"  uniform float2 ta0,                                         \n"
"  uniform float2 tb0,                                         \n"
"                                                              \n"
"  out float4 color : COLOR0                                   \n"
"){                                                            \n"
"  float2 ta, tb;                                              \n"
"    ta = ta0 + float2(                                        \n"
"      k + floor( tc.y / blk ),                                \n"
"      EMUX_CONST * floor( fmod( tc.y, blk ) )+.5 );           \n"
"    tb = tb0 + float2( tc.x, ta.y );                          \n"
"                                                              \n"
"  float4 a, b;                                                \n"
"  int i;                                                      \n"
"  for( i=0; i<EMUX_CONST; i++ )                               \n"
"  {                                                           \n"
"    a = f4texRECT( src0, ta );                                \n"
"    b = f4texRECT( src1, tb );                                \n"
"    color += a*b * step(tb.y,ybound);                         \n"
"    ta.y+=1;                                                  \n"
"    tb.y+=1;                                                  \n"
"  }                                                           \n"
"}                                                             \n";

char *rtm_emux_krc_src = 
"void main(                                                    \n"
"  float2 tc : TEXCOORD0,                                      \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform samplerRECT src1,                                   \n"
"  uniform float k,                                            \n"
"  uniform float ybound,                                       \n"
"  uniform float blk,                                          \n"
"  uniform float2 ta0,                                         \n"
"  uniform float2 tb0,                                         \n"
"                                                              \n"
"  out float4 color : COLOR0                                   \n"
"){                                                            \n"
"  float2 ta, tb;                                              \n"
"    ta = ta0 + float2(                                        \n"
"      EMUX_CONST * floor( fmod( tc.y, blk ) )+.5,             \n"
"      k + floor( tc.y / blk ) );                              \n"
"    tb = tb0 + float2( tc.x, ta.x );                          \n"
"                                                              \n"
"  float4 a, b;                                                \n"
"  int i;                                                      \n"
"  for( i=0; i<EMUX_CONST; i++ )                               \n"
"  {                                                           \n"
"    a = f4texRECT( src0, ta );                                \n"
"    b = f4texRECT( src1, tb );                                \n"
"    color += a*b * step(tb.y,ybound);                         \n"
"    ta.x+=1;                                                  \n"
"    tb.y+=1;                                                  \n"
"  }                                                           \n"
"}                                                             \n";

char *rtm_emux_kcr_src = 
"void main(                                                    \n"
"  float2 tc : TEXCOORD0,                                      \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform samplerRECT src1,                                   \n"
"  uniform float k,                                            \n"
"  uniform float xbound,                                       \n"
"  uniform float blk,                                          \n"
"  uniform float2 ta0,                                         \n"
"  uniform float2 tb0,                                         \n"
"                                                              \n"
"  out float4 color : COLOR0                                   \n"
"){                                                            \n"
"  float2 ta, tb;                                              \n"
"    ta = ta0 + float2(                                        \n"
"      k + floor( tc.y / blk ),                                \n"
"      EMUX_CONST * floor( fmod( tc.y, blk ) )+.5 );           \n"
"    tb = tb0 + float2( ta.y, tc.x );                          \n"
"                                                              \n"
"  float4 a, b;                                                \n"
"  int i;                                                      \n"
"  for( i=0; i<EMUX_CONST; i++ )                               \n"
"  {                                                           \n"
"    a = f4texRECT( src0, ta );                                \n"
"    b = f4texRECT( src1, tb );                                \n"
"    color += a*b * step(tb.x,xbound);                         \n"
"    ta.y+=1;                                                  \n"
"    tb.x+=1;                                                  \n"
"  }                                                           \n"
"}                                                             \n";

char *rtm_emux_krr_src = 
"void main(                                                    \n"
"  float2 tc : TEXCOORD0,                                      \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform samplerRECT src1,                                   \n"
"  uniform float k,                                            \n"
"  uniform float xbound,                                       \n"
"  uniform float blk,                                          \n"
"  uniform float2 ta0,                                         \n"
"  uniform float2 tb0,                                         \n"
"                                                              \n"
"  out float4 color : COLOR0                                   \n"
"){                                                            \n"
"  float2 ta, tb;                                              \n"
"    ta = ta0 + float2(                                        \n"
"        EMUX_CONST * floor( fmod( tc.y, blk ) )+.5,           \n" 
"        k + floor( tc.y / blk ) );                            \n"
"    tb = tb0 + float2( ta.x, tc.x );                          \n"
"                                                              \n"
"  float4 a, b;                                                \n"
"  int i;                                                      \n"
"  for( i=0; i<EMUX_CONST; i++ )                               \n"
"  {                                                           \n"
"    a = f4texRECT( src0, ta );                                \n"
"    b = f4texRECT( src1, tb );                                \n"
"    color += a*b * step(tb.x,xbound);                         \n"
"    ta.x+=1;                                                  \n"
"    tb.x+=1;                                                  \n"
"  }                                                           \n"
"}                                                             \n";

char *rtm_colsumx_src = 
"void main(                                                    \n"
"  float2 tc : TEXCOORD0,                                      \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform float h0,                                           \n"
"  uniform float h1,                                           \n"
"  uniform float2 ta0,                                         \n"
"  uniform float anc,                                          \n"
"                                                              \n"
"  out float4 color : COLOR0                                   \n"
"){                                                            \n"
"                                                              \n"
"  float ybound = h0 * floor( tc.y/h1 ) + h0 + ta0.y;          \n"
"                                                              \n"
"  float2 ta = ta0 + float2( tc.x,                             \n"
"    CSUM_CONST * floor( fmod(tc.y,h1) ) +                     \n"
"    h0 * floor( tc.y/h1 ) + 0.5 );                            \n"
"                                                              \n"
"  int i;                                                      \n"
"  for( i=0; i<CSUM_CONST; i++ )                               \n"
"  {                                                           \n"
"    color += f4texRECT( src0, ta ) * step( ta.y, ybound );    \n"
"    ta.y+=1;                                                  \n"
"  }                                                           \n"
"                                                              \n"
"  color = (1-anc)*color + anc*color.xzyw;                     \n"
"}                                                             \n";

char *rtm_rowsumx_src = 
"void main(                                                    \n"
"  float2 tc : TEXCOORD0,                                      \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform float w0,                                           \n"
"  uniform float h1,                                           \n"
"  uniform float2 ta0,                                         \n"
"  uniform float anc,                                          \n"
"                                                              \n"
"  out float4 color : COLOR0                                   \n"
"){                                                            \n"
"                                                              \n"
"  float xbound = w0 * floor( tc.y/h1 ) + w0 + ta0.x;          \n"
"                                                              \n"
"  float2 ta = ta0 + float2(                                   \n"
"      CSUM_CONST * floor( fmod(tc.y,h1) ) +                   \n"
"      w0 * floor( tc.y/h1 ) + 0.5 , tc.x );                   \n"
"                                                              \n"
"                                                              \n"
"  int i;                                                      \n"
"  for( i=0; i<CSUM_CONST; i++ )                               \n"
"  {                                                           \n"
"    color += f4texRECT( src0, ta ) * step( ta.x, xbound );    \n"
"    ta.x+=1;                                                  \n"
"  }                                                           \n"
"                                                              \n"
"  color = (1-anc)*color + anc*color.xzyw;                     \n"
"}                                                             \n";

char *rtm_emux_kcc_packed_src = 
"void main(                                                    \n"
"  float2 tc : TEXCOORD0,                                      \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform samplerRECT src1,                                   \n"
"  uniform float k,                                            \n"
"  uniform float ybound,                                       \n"
"  uniform float blk,                                          \n"
"  uniform float2 ta0,                                         \n"
"  uniform float2 tb0,                                         \n"
"  uniform float  anc,                                          \n"
"                                                              \n"
"  out float4 color : COLOR0                                   \n"
"){                                                            \n"
"  float2 ta, tb;                                              \n"
"    ta = ta0 + float2(                                        \n"
"      k + floor( tc.y / blk ),                                \n"
"      EMUX_CONST * floor( fmod( tc.y, blk ) )+.5 );           \n"
"    tb = tb0 + float2( tc.x, ta.y );                          \n"
"                                                              \n"
"  float4 a, b;                                                \n"
"  int i;                                                      \n"
"  for( i=0; i<EMUX_CONST; i++ )                               \n"
"  {                                                           \n"
"    a = f4texRECT( src0, ta );                                \n"
"    b = f4texRECT( src1, tb );                                \n"
"    color += ( a.xxyy * b.xyxy + a.zzww * b.zwzw  ) *         \n"
"             step(tb.y,ybound);                               \n"
"    ta.y+=1;                                                  \n"
"    tb.y+=1;                                                  \n"
"  }                                                           \n"
"                                                              \n"
"  color = (1-anc)*color + anc*color.xzyw;                     \n"
"}                                                             \n";

char *rtm_emux_krc_packed_src = 
"void main(                                                    \n"
"  float2 tc : TEXCOORD0,                                      \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform samplerRECT src1,                                   \n"
"  uniform float k,                                            \n"
"  uniform float ybound,                                       \n"
"  uniform float blk,                                          \n"
"  uniform float2 ta0,                                         \n"
"  uniform float2 tb0,                                         \n"
"  uniform float  anc,                                          \n"
"                                                              \n"
"  out float4 color : COLOR0                                   \n"
"){                                                            \n"
"  float2 ta, tb;                                              \n"
"    ta = ta0 + float2(                                        \n"
"      EMUX_CONST * floor( fmod( tc.y, blk ) )+.5,             \n"
"      k + floor( tc.y / blk ) );                              \n"
"    tb = tb0 + float2( tc.x, ta.x );                          \n"
"                                                              \n"
"  float4 a, b;                                                \n"
"  int i;                                                      \n"
"  for( i=0; i<EMUX_CONST; i++ )                               \n"
"  {                                                           \n"
"    a = f4texRECT( src0, ta );                                \n"
"    b = f4texRECT( src1, tb );                                \n"
"    color += ( a.xxzz * b.xyxy + a.yyww * b.zwzw  ) *         \n"
"               step(tb.y,ybound);                             \n"
"    ta.x+=1;                                                  \n"
"    tb.y+=1;                                                  \n"
"  }                                                           \n"
"                                                              \n"
"  color = (1-anc)*color + anc*color.xzyw;                     \n"
"}                                                             \n";

char *rtm_emux_kcr_packed_src = 
"void main(                                                    \n"
"  float2 tc : TEXCOORD0,                                      \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform samplerRECT src1,                                   \n"
"  uniform float k,                                            \n"
"  uniform float xbound,                                       \n"
"  uniform float blk,                                          \n"
"  uniform float2 ta0,                                         \n"
"  uniform float2 tb0,                                         \n"
"  uniform float  anc,                                         \n"
"                                                              \n"
"  out float4 color : COLOR0                                   \n"
"){                                                            \n"
"  float2 ta, tb;                                              \n"
"    ta = ta0 + float2(                                        \n"
"      k + floor( tc.y / blk ),                                \n"
"      EMUX_CONST * floor( fmod( tc.y, blk ) )+.5 );           \n"
"    tb = tb0 + float2( ta.y, tc.x );                          \n"
"                                                              \n"
"  float4 a, b;                                                \n"
"  int i;                                                      \n"
"  for( i=0; i<EMUX_CONST; i++ )                               \n"
"  {                                                           \n"
"    a = f4texRECT( src0, ta );                                \n"
"    b = f4texRECT( src1, tb );                                \n"
"    color += ( a.xxyy * b.xzxz + a.zzww * b.ywyw  ) *         \n"
"               step(tb.x,xbound);                             \n"
"    ta.y+=1;                                                  \n"
"    tb.x+=1;                                                  \n"
"  }                                                           \n"
"                                                              \n"
"  color = (1-anc)*color + anc*color.xzyw;                     \n"
"}                                                             \n";

char *rtm_emux_krr_packed_src = 
"void main(                                                    \n"
"  float2 tc : TEXCOORD0,                                      \n"
"                                                              \n"
"  uniform samplerRECT src0,                                   \n"
"  uniform samplerRECT src1,                                   \n"
"  uniform float k,                                            \n"
"  uniform float xbound,                                       \n"
"  uniform float blk,                                          \n"
"  uniform float2 ta0,                                         \n"
"  uniform float2 tb0,                                         \n"
"  uniform float  anc,                                         \n"
"                                                              \n"
"  out float4 color : COLOR0                                   \n"
"){                                                            \n"
"  float2 ta, tb;                                              \n"
"    ta = ta0 + float2(                                        \n"
"        EMUX_CONST * floor( fmod( tc.y, blk ) )+.5,           \n" 
"        k + floor( tc.y / blk ) );                            \n"
"    tb = tb0 + float2( ta.x, tc.x );                          \n"
"                                                              \n"
"  float4 a, b;                                                \n"
"  int i;                                                      \n"
"  for( i=0; i<EMUX_CONST; i++ )                               \n"
"  {                                                           \n"
"    a = f4texRECT( src0, ta );                                \n"
"    b = f4texRECT( src1, tb );                                \n"
"    color += ( a.xxzz * b.xzxz + a.yyww * b.ywyw  ) *         \n"
"               step(tb.x,xbound);                             \n"
"    ta.x+=1;                                                  \n"
"    tb.x+=1;                                                  \n"
"  }                                                           \n"
"                                                              \n"
"  color = (1-anc)*color + anc*color.xzyw;                     \n"
"}                                                             \n";

CGcontext rtm_context;
  CGprofile rtm_fragment_profile;

    CGprogram rtm_draw;
      CGparameter rtm_draw_src0;
      CGparameter rtm_draw_scale;
      CGparameter rtm_draw_anc;

    CGprogram rtm_unpack;
      CGparameter rtm_unpack_src0;

    CGprogram rtm_a_add_b;
      CGparameter rtm_a_add_b_src0;
      CGparameter rtm_a_add_b_src1;
      CGparameter rtm_a_add_b_sb;

    CGprogram rtm_a_add_bt;
      CGparameter rtm_a_add_bt_src0;
      CGparameter rtm_a_add_bt_src1;
      CGparameter rtm_a_add_bt_sb;

    CGprogram rtm_a_add_bt_packed;
      CGparameter rtm_a_add_bt_packed_src0;
      CGparameter rtm_a_add_bt_packed_src1;
      CGparameter rtm_a_add_bt_packed_sb;

    CGprogram rtm_at_add_bt_packed;
      CGparameter rtm_at_add_bt_packed_src0;
      CGparameter rtm_at_add_bt_packed_src1;
      CGparameter rtm_at_add_bt_packed_sb;

    CGprogram rtm_colsumx;
      CGparameter rtm_colsumx_src0;
      CGparameter rtm_colsumx_h0;
      CGparameter rtm_colsumx_h1;
      CGparameter rtm_colsumx_ta0;
      CGparameter rtm_colsumx_anc;
      
    CGprogram rtm_rowsumx;
      CGparameter rtm_rowsumx_src0;
      CGparameter rtm_rowsumx_w0;
      CGparameter rtm_rowsumx_h1;
      CGparameter rtm_rowsumx_ta0;
      CGparameter rtm_rowsumx_anc;
      
    CGprogram rtm_emux_kcc;
      CGparameter rtm_emux_kcc_src0;
      CGparameter rtm_emux_kcc_src1;
      CGparameter rtm_emux_kcc_k;
      CGparameter rtm_emux_kcc_ybound;
      CGparameter rtm_emux_kcc_blk;
      CGparameter rtm_emux_kcc_ta0;
      CGparameter rtm_emux_kcc_tb0;
      
    CGprogram rtm_emux_krc;
      CGparameter rtm_emux_krc_src0;
      CGparameter rtm_emux_krc_src1;
      CGparameter rtm_emux_krc_k;
      CGparameter rtm_emux_krc_ybound;
      CGparameter rtm_emux_krc_blk;
      CGparameter rtm_emux_krc_ta0;
      CGparameter rtm_emux_krc_tb0;
      
    CGprogram rtm_emux_kcr;
      CGparameter rtm_emux_kcr_src0;
      CGparameter rtm_emux_kcr_src1;
      CGparameter rtm_emux_kcr_k;
      CGparameter rtm_emux_kcr_xbound;
      CGparameter rtm_emux_kcr_blk;
      CGparameter rtm_emux_kcr_ta0;
      CGparameter rtm_emux_kcr_tb0;
      
    CGprogram rtm_emux_krr;
      CGparameter rtm_emux_krr_src0;
      CGparameter rtm_emux_krr_src1;
      CGparameter rtm_emux_krr_k;
      CGparameter rtm_emux_krr_xbound;
      CGparameter rtm_emux_krr_blk;
      CGparameter rtm_emux_krr_ta0;
      CGparameter rtm_emux_krr_tb0;
      
    CGprogram rtm_emux_krc_packed;
      CGparameter rtm_emux_krc_packed_src0;
      CGparameter rtm_emux_krc_packed_src1;
      CGparameter rtm_emux_krc_packed_k;
      CGparameter rtm_emux_krc_packed_ybound;
      CGparameter rtm_emux_krc_packed_blk;
      CGparameter rtm_emux_krc_packed_ta0;
      CGparameter rtm_emux_krc_packed_tb0;
      CGparameter rtm_emux_krc_packed_anc;
      
    CGprogram rtm_emux_kcc_packed;
      CGparameter rtm_emux_kcc_packed_src0;
      CGparameter rtm_emux_kcc_packed_src1;
      CGparameter rtm_emux_kcc_packed_k;
      CGparameter rtm_emux_kcc_packed_ybound;
      CGparameter rtm_emux_kcc_packed_blk;
      CGparameter rtm_emux_kcc_packed_ta0;
      CGparameter rtm_emux_kcc_packed_tb0;
      CGparameter rtm_emux_kcc_packed_anc;
      
    CGprogram rtm_emux_kcr_packed;
      CGparameter rtm_emux_kcr_packed_src0;
      CGparameter rtm_emux_kcr_packed_src1;
      CGparameter rtm_emux_kcr_packed_k;
      CGparameter rtm_emux_kcr_packed_xbound;
      CGparameter rtm_emux_kcr_packed_blk;
      CGparameter rtm_emux_kcr_packed_ta0;
      CGparameter rtm_emux_kcr_packed_tb0;
      CGparameter rtm_emux_kcr_packed_anc;
      
    CGprogram rtm_emux_krr_packed;
      CGparameter rtm_emux_krr_packed_src0;
      CGparameter rtm_emux_krr_packed_src1;
      CGparameter rtm_emux_krr_packed_k;
      CGparameter rtm_emux_krr_packed_xbound;
      CGparameter rtm_emux_krr_packed_blk;
      CGparameter rtm_emux_krr_packed_ta0;
      CGparameter rtm_emux_krr_packed_tb0;
      CGparameter rtm_emux_krr_packed_anc;
      
void rtm_cgErrorCallback()
{
  CGerror lastError = cgGetError();
  if(lastError)
  {
    printf("%s\n\n", cgGetErrorString(lastError));
    printf("%s\n", cgGetLastListing(rtm_context));
    printf("Cg error, exiting...\n");
    exit(0);
  }
}


void rtm_prepare()
{
  cgSetErrorCallback(rtm_cgErrorCallback);

  rtm_context = cgCreateContext();
    rtm_fragment_profile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
    cgGLSetOptimalOptions(rtm_fragment_profile);

  rtm_draw = cgCreateProgram( rtm_context, CG_SOURCE, rtm_draw_src, rtm_fragment_profile, NULL, 0 );
    cgGLLoadProgram(rtm_draw);
      rtm_draw_src0  = cgGetNamedParameter( rtm_draw, "src0" );
      rtm_draw_scale = cgGetNamedParameter( rtm_draw, "scale" );
      rtm_draw_anc   = cgGetNamedParameter( rtm_draw, "anc" );

  rtm_unpack = cgCreateProgram( rtm_context, CG_SOURCE, rtm_unpack_src, rtm_fragment_profile, NULL, 0 );
    cgGLLoadProgram(rtm_unpack);
      rtm_unpack_src0 = cgGetNamedParameter( rtm_unpack, "src0" );

  rtm_a_add_b = cgCreateProgram( rtm_context, CG_SOURCE, rtm_a_add_b_src, rtm_fragment_profile, NULL, 0 );
    cgGLLoadProgram(rtm_a_add_b);
      rtm_a_add_b_src0 = cgGetNamedParameter( rtm_a_add_b, "src0" );
      rtm_a_add_b_src1 = cgGetNamedParameter( rtm_a_add_b, "src1" );
      rtm_a_add_b_sb   = cgGetNamedParameter( rtm_a_add_b, "sb" );

  rtm_a_add_bt = cgCreateProgram( rtm_context, CG_SOURCE, rtm_a_add_bt_src, rtm_fragment_profile, NULL, 0 );
    cgGLLoadProgram(rtm_a_add_bt);
      rtm_a_add_bt_src0 = cgGetNamedParameter( rtm_a_add_bt, "src0" );
      rtm_a_add_bt_src1 = cgGetNamedParameter( rtm_a_add_bt, "src1" );
      rtm_a_add_bt_sb   = cgGetNamedParameter( rtm_a_add_bt, "sb" );

  rtm_a_add_bt_packed = cgCreateProgram( rtm_context, CG_SOURCE, rtm_a_add_bt_packed_src, rtm_fragment_profile, NULL, 0 );
    cgGLLoadProgram(rtm_a_add_bt_packed);
      rtm_a_add_bt_packed_src0 = cgGetNamedParameter( rtm_a_add_bt_packed, "src0" );
      rtm_a_add_bt_packed_src1 = cgGetNamedParameter( rtm_a_add_bt_packed, "src1" );
      rtm_a_add_bt_packed_sb   = cgGetNamedParameter( rtm_a_add_bt_packed, "sb" );

  rtm_at_add_bt_packed = cgCreateProgram( rtm_context, CG_SOURCE, rtm_at_add_bt_packed_src, rtm_fragment_profile, NULL, 0 );
    cgGLLoadProgram(rtm_at_add_bt_packed);
      rtm_at_add_bt_packed_src0 = cgGetNamedParameter( rtm_at_add_bt_packed, "src0" );
      rtm_at_add_bt_packed_src1 = cgGetNamedParameter( rtm_at_add_bt_packed, "src1" );
      rtm_at_add_bt_packed_sb   = cgGetNamedParameter( rtm_at_add_bt_packed, "sb" );





  char str[4096];





  sprintf( str, "#define CSUM_CONST %i\n%s", CSUM_CONST, rtm_colsumx_src );
  rtm_colsumx = cgCreateProgram( rtm_context, CG_SOURCE, str, rtm_fragment_profile, NULL, 0 );
    cgGLLoadProgram(rtm_colsumx);
      rtm_colsumx_src0   = cgGetNamedParameter( rtm_colsumx, "src0" );
      rtm_colsumx_h0     = cgGetNamedParameter( rtm_colsumx, "h0" );
      rtm_colsumx_h1     = cgGetNamedParameter( rtm_colsumx, "h1" );
      rtm_colsumx_ta0     = cgGetNamedParameter( rtm_colsumx, "ta0" );
      rtm_colsumx_anc     = cgGetNamedParameter( rtm_colsumx, "anc" );

  sprintf( str, "#define CSUM_CONST %i\n%s", CSUM_CONST, rtm_rowsumx_src );
  rtm_rowsumx = cgCreateProgram( rtm_context, CG_SOURCE, str, rtm_fragment_profile, NULL, 0 );
    cgGLLoadProgram(rtm_rowsumx);
      rtm_rowsumx_src0   = cgGetNamedParameter( rtm_rowsumx, "src0" );
      rtm_rowsumx_w0     = cgGetNamedParameter( rtm_rowsumx, "w0" );
      rtm_rowsumx_h1     = cgGetNamedParameter( rtm_rowsumx, "h1" );
      rtm_rowsumx_ta0     = cgGetNamedParameter( rtm_rowsumx, "ta0" );
      rtm_rowsumx_anc     = cgGetNamedParameter( rtm_rowsumx, "anc" );

  sprintf( str, "#define EMUX_CONST %i\n%s", EMUX_CONST, rtm_emux_kcc_src );
  rtm_emux_kcc = cgCreateProgram( rtm_context, CG_SOURCE, str, rtm_fragment_profile, NULL, 0 );
    cgGLLoadProgram(rtm_emux_kcc);
      rtm_emux_kcc_src0   = cgGetNamedParameter( rtm_emux_kcc, "src0" );
      rtm_emux_kcc_src1   = cgGetNamedParameter( rtm_emux_kcc, "src1" );
      rtm_emux_kcc_k      = cgGetNamedParameter( rtm_emux_kcc, "k" );
      rtm_emux_kcc_ybound = cgGetNamedParameter( rtm_emux_kcc, "ybound" );
      rtm_emux_kcc_blk    = cgGetNamedParameter( rtm_emux_kcc, "blk" );
      rtm_emux_kcc_ta0    = cgGetNamedParameter( rtm_emux_kcc, "ta0" );
      rtm_emux_kcc_tb0    = cgGetNamedParameter( rtm_emux_kcc, "tb0" );

  sprintf( str, "#define EMUX_CONST %i\n%s", EMUX_CONST, rtm_emux_krc_src );
  rtm_emux_krc = cgCreateProgram( rtm_context, CG_SOURCE, str, rtm_fragment_profile, NULL, 0 );
    cgGLLoadProgram(rtm_emux_krc);
      rtm_emux_krc_src0   = cgGetNamedParameter( rtm_emux_krc, "src0" );
      rtm_emux_krc_src1   = cgGetNamedParameter( rtm_emux_krc, "src1" );
      rtm_emux_krc_k      = cgGetNamedParameter( rtm_emux_krc, "k" );
      rtm_emux_krc_ybound = cgGetNamedParameter( rtm_emux_krc, "ybound" );
      rtm_emux_krc_blk    = cgGetNamedParameter( rtm_emux_krc, "blk" );
      rtm_emux_krc_ta0    = cgGetNamedParameter( rtm_emux_krc, "ta0" );
      rtm_emux_krc_tb0    = cgGetNamedParameter( rtm_emux_krc, "tb0" );

  sprintf( str, "#define EMUX_CONST %i\n%s", EMUX_CONST, rtm_emux_kcr_src );
  rtm_emux_kcr = cgCreateProgram( rtm_context, CG_SOURCE, str, rtm_fragment_profile, NULL, 0 );
    cgGLLoadProgram(rtm_emux_kcr);
      rtm_emux_kcr_src0   = cgGetNamedParameter( rtm_emux_kcr, "src0" );
      rtm_emux_kcr_src1   = cgGetNamedParameter( rtm_emux_kcr, "src1" );
      rtm_emux_kcr_k      = cgGetNamedParameter( rtm_emux_kcr, "k" );
      rtm_emux_kcr_xbound = cgGetNamedParameter( rtm_emux_kcr, "xbound" );
      rtm_emux_kcr_blk    = cgGetNamedParameter( rtm_emux_kcr, "blk" );
      rtm_emux_kcr_ta0    = cgGetNamedParameter( rtm_emux_kcr, "ta0" );
      rtm_emux_kcr_tb0    = cgGetNamedParameter( rtm_emux_kcr, "tb0" );

  sprintf( str, "#define EMUX_CONST %i\n%s", EMUX_CONST, rtm_emux_krr_src );
  rtm_emux_krr = cgCreateProgram( rtm_context, CG_SOURCE, str, rtm_fragment_profile, NULL, 0 );
    cgGLLoadProgram(rtm_emux_krr);
      rtm_emux_krr_src0   = cgGetNamedParameter( rtm_emux_krr, "src0" );
      rtm_emux_krr_src1   = cgGetNamedParameter( rtm_emux_krr, "src1" );
      rtm_emux_krr_k      = cgGetNamedParameter( rtm_emux_krr, "k" );
      rtm_emux_krr_xbound = cgGetNamedParameter( rtm_emux_krr, "xbound" );
      rtm_emux_krr_blk    = cgGetNamedParameter( rtm_emux_krr, "blk" );
      rtm_emux_krr_ta0    = cgGetNamedParameter( rtm_emux_krr, "ta0" );
      rtm_emux_krr_tb0    = cgGetNamedParameter( rtm_emux_krr, "tb0" );



  sprintf( str, "#define EMUX_CONST %i\n%s", EMUX_CONST, rtm_emux_krc_packed_src );
  rtm_emux_krc_packed = cgCreateProgram( rtm_context, CG_SOURCE, str, rtm_fragment_profile, NULL, 0 );
    cgGLLoadProgram(rtm_emux_krc_packed);
      rtm_emux_krc_packed_src0   = cgGetNamedParameter( rtm_emux_krc_packed, "src0" );
      rtm_emux_krc_packed_src1   = cgGetNamedParameter( rtm_emux_krc_packed, "src1" );
      rtm_emux_krc_packed_k      = cgGetNamedParameter( rtm_emux_krc_packed, "k" );
      rtm_emux_krc_packed_ybound = cgGetNamedParameter( rtm_emux_krc_packed, "ybound" );
      rtm_emux_krc_packed_blk    = cgGetNamedParameter( rtm_emux_krc_packed, "blk" );
      rtm_emux_krc_packed_ta0    = cgGetNamedParameter( rtm_emux_krc_packed, "ta0" );
      rtm_emux_krc_packed_tb0    = cgGetNamedParameter( rtm_emux_krc_packed, "tb0" );
      rtm_emux_krc_packed_anc    = cgGetNamedParameter( rtm_emux_krc_packed, "anc" );

  sprintf( str, "#define EMUX_CONST %i\n%s", EMUX_CONST, rtm_emux_kcc_packed_src );
  rtm_emux_kcc_packed = cgCreateProgram( rtm_context, CG_SOURCE, str, rtm_fragment_profile, NULL, 0 );
    cgGLLoadProgram(rtm_emux_kcc_packed);
      rtm_emux_kcc_packed_src0   = cgGetNamedParameter( rtm_emux_kcc_packed, "src0" );
      rtm_emux_kcc_packed_src1   = cgGetNamedParameter( rtm_emux_kcc_packed, "src1" );
      rtm_emux_kcc_packed_k      = cgGetNamedParameter( rtm_emux_kcc_packed, "k" );
      rtm_emux_kcc_packed_ybound = cgGetNamedParameter( rtm_emux_kcc_packed, "ybound" );
      rtm_emux_kcc_packed_blk    = cgGetNamedParameter( rtm_emux_kcc_packed, "blk" );
      rtm_emux_kcc_packed_ta0    = cgGetNamedParameter( rtm_emux_kcc_packed, "ta0" );
      rtm_emux_kcc_packed_tb0    = cgGetNamedParameter( rtm_emux_kcc_packed, "tb0" );
      rtm_emux_kcc_packed_anc    = cgGetNamedParameter( rtm_emux_kcc_packed, "anc" );

  sprintf( str, "#define EMUX_CONST %i\n%s", EMUX_CONST, rtm_emux_kcr_packed_src );
  rtm_emux_kcr_packed = cgCreateProgram( rtm_context, CG_SOURCE, str, rtm_fragment_profile, NULL, 0 );
    cgGLLoadProgram(rtm_emux_kcr_packed);
      rtm_emux_kcr_packed_src0   = cgGetNamedParameter( rtm_emux_kcr_packed, "src0" );
      rtm_emux_kcr_packed_src1   = cgGetNamedParameter( rtm_emux_kcr_packed, "src1" );
      rtm_emux_kcr_packed_k      = cgGetNamedParameter( rtm_emux_kcr_packed, "k" );
      rtm_emux_kcr_packed_xbound = cgGetNamedParameter( rtm_emux_kcr_packed, "xbound" );
      rtm_emux_kcr_packed_blk    = cgGetNamedParameter( rtm_emux_kcr_packed, "blk" );
      rtm_emux_kcr_packed_ta0    = cgGetNamedParameter( rtm_emux_kcr_packed, "ta0" );
      rtm_emux_kcr_packed_tb0    = cgGetNamedParameter( rtm_emux_kcr_packed, "tb0" );
      rtm_emux_kcr_packed_anc    = cgGetNamedParameter( rtm_emux_kcr_packed, "anc" );

  sprintf( str, "#define EMUX_CONST %i\n%s", EMUX_CONST, rtm_emux_krr_packed_src );
  rtm_emux_krr_packed = cgCreateProgram( rtm_context, CG_SOURCE, str, rtm_fragment_profile, NULL, 0 );
    cgGLLoadProgram(rtm_emux_krr_packed);
      rtm_emux_krr_packed_src0   = cgGetNamedParameter( rtm_emux_krr_packed, "src0" );
      rtm_emux_krr_packed_src1   = cgGetNamedParameter( rtm_emux_krr_packed, "src1" );
      rtm_emux_krr_packed_k      = cgGetNamedParameter( rtm_emux_krr_packed, "k" );
      rtm_emux_krr_packed_xbound = cgGetNamedParameter( rtm_emux_krr_packed, "xbound" );
      rtm_emux_krr_packed_blk    = cgGetNamedParameter( rtm_emux_krr_packed, "blk" );
      rtm_emux_krr_packed_ta0    = cgGetNamedParameter( rtm_emux_krr_packed, "ta0" );
      rtm_emux_krr_packed_tb0    = cgGetNamedParameter( rtm_emux_krr_packed, "tb0" );
      rtm_emux_krr_packed_anc    = cgGetNamedParameter( rtm_emux_krr_packed, "anc" );
}

void RTm::prepare()
{
  rtm_prepare();
}

void RTm::RTM__draw( 
  RenderBuffer *a, int ax, int ay, int aw, int ah, int at,
  RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, int anc
){
  int _cw, _ch;
    _cw = c->GetWidth();
    _ch = c->GetHeight();

  c->BeginCapture();

    glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
        gluOrtho2D( 0, _cw, 0, _ch );

    cgSetParameter1f( rtm_draw_anc, anc );
    cgSetParameter4f( rtm_draw_scale, 1,1,1,1 );
    cgGLSetTextureParameter( rtm_draw_src0, a->GetTextureID() );
    cgGLEnableTextureParameter( rtm_draw_src0 );

    cgGLEnableProfile( rtm_fragment_profile );

    cgGLBindProgram( rtm_draw );

    glBegin(GL_QUADS);
      if( ct )
      {
        if( at )
        {
          glTexCoord2i( ay +  0, ax +  0 );    glVertex2i( cy +  0, cx +  0 );
          glTexCoord2i( ay +  0, ax + aw );    glVertex2i( cy +  0, cx + cw );
          glTexCoord2i( ay + ah, ax + aw );    glVertex2i( cy + ch, cx + cw );
          glTexCoord2i( ay + ah, ax +  0 );    glVertex2i( cy + ch, cx +  0 );
        }else
        {
          glTexCoord2i( ax +  0, ay +  0 );    glVertex2i( cy +  0, cx +  0 );
          glTexCoord2i( ax + aw, ay +  0 );    glVertex2i( cy +  0, cx + cw );
          glTexCoord2i( ax + aw, ay + ah );    glVertex2i( cy + ch, cx + cw );
          glTexCoord2i( ax +  0, ay + ah );    glVertex2i( cy + ch, cx +  0 );
        }
      }else
      {
        if( at )
        {
          glTexCoord2i( ay +  0, ax +  0 );    glVertex2i( cx +  0, cy +  0 );
          glTexCoord2i( ay +  0, ax + aw );    glVertex2i( cx + cw, cy +  0 );
          glTexCoord2i( ay + ah, ax + aw );    glVertex2i( cx + cw, cy + ch );
          glTexCoord2i( ay + ah, ax +  0 );    glVertex2i( cx +  0, cy + ch );
        }else
        {
          glTexCoord2i( ax +  0, ay +  0 );    glVertex2i( cx +  0, cy +  0 );
          glTexCoord2i( ax + aw, ay +  0 );    glVertex2i( cx + cw, cy +  0 );
          glTexCoord2i( ax + aw, ay + ah );    glVertex2i( cx + cw, cy + ch );
          glTexCoord2i( ax +  0, ay + ah );    glVertex2i( cx +  0, cy + ch );
        }
      }
    glEnd();

    cgGLDisableTextureParameter(rtm_draw_src0);
    cgGLDisableProfile( rtm_fragment_profile );

  c->EndCapture();
    sum_pass++;
}

void RTm::RTM__unpack( 
  RenderBuffer *a, int ax, int ay, int aw, int ah, int at,
  RenderBuffer *c, int cx, int cy, int cw, int ch, int ct 
){
  int _cw, _ch;
    _cw = c->GetWidth();
    _ch = c->GetHeight();

  c->BeginCapture();

    glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
        gluOrtho2D( 0, _cw, 0, _ch );


    cgGLSetTextureParameter( rtm_unpack_src0, a->GetTextureID() );
    cgGLEnableTextureParameter( rtm_unpack_src0 );

    cgGLEnableProfile( rtm_fragment_profile );

    cgGLBindProgram( rtm_unpack );

    glBegin(GL_QUADS);
      if( ct )
      {
        if( at )
        {
          glTexCoord2i( ay +  0, ax +  0 );    glVertex2i( cy +  0, cx +  0 );
          glTexCoord2i( ay +  0, ax + aw );    glVertex2i( cy +  0, cx + cw );
          glTexCoord2i( ay + ah, ax + aw );    glVertex2i( cy + ch, cx + cw );
          glTexCoord2i( ay + ah, ax +  0 );    glVertex2i( cy + ch, cx +  0 );
        }else
        {
          glTexCoord2i( ax +  0, ay +  0 );    glVertex2i( cy +  0, cx +  0 );
          glTexCoord2i( ax + aw, ay +  0 );    glVertex2i( cy +  0, cx + cw );
          glTexCoord2i( ax + aw, ay + ah );    glVertex2i( cy + ch, cx + cw );
          glTexCoord2i( ax +  0, ay + ah );    glVertex2i( cy + ch, cx +  0 );
        }
      }else
      {
        if( at )
        {
          glTexCoord2i( ay +  0, ax +  0 );    glVertex2i( cx +  0, cy +  0 );
          glTexCoord2i( ay +  0, ax + aw );    glVertex2i( cx + cw, cy +  0 );
          glTexCoord2i( ay + ah, ax + aw );    glVertex2i( cx + cw, cy + ch );
          glTexCoord2i( ay + ah, ax +  0 );    glVertex2i( cx +  0, cy + ch );
        }else
        {
          glTexCoord2i( ax +  0, ay +  0 );    glVertex2i( cx +  0, cy +  0 );
          glTexCoord2i( ax + aw, ay +  0 );    glVertex2i( cx + cw, cy +  0 );
          glTexCoord2i( ax + aw, ay + ah );    glVertex2i( cx + cw, cy + ch );
          glTexCoord2i( ax +  0, ay + ah );    glVertex2i( cx +  0, cy + ch );
        }
      }
    glEnd();

    cgGLDisableTextureParameter(rtm_unpack_src0);
    cgGLDisableProfile( rtm_fragment_profile );

  c->EndCapture();
    sum_pass++;
}


// c = a + b
void RTm::RTM__a_add_b( 
  RenderBuffer *a, int at,  
  RenderBuffer *b, FLOAT4 sb,
  RenderBuffer *c, int cw, int ch, int ct )
{
  int _cw, _ch;
    _cw = c->GetWidth();
    _ch = c->GetHeight();

  c->BeginCapture();

    glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
        gluOrtho2D( 0, _cw, 0, _ch );

    cgSetParameter4fv( rtm_a_add_b_sb, (float*)&sb );

    cgGLSetTextureParameter( rtm_a_add_b_src0, a->GetTextureID() );
    cgGLEnableTextureParameter( rtm_a_add_b_src0 );

    cgGLSetTextureParameter( rtm_a_add_b_src1, b->GetTextureID() );
    cgGLEnableTextureParameter( rtm_a_add_b_src1 );

    cgGLEnableProfile( rtm_fragment_profile );

    cgGLBindProgram( rtm_a_add_b );

    glBegin(GL_QUADS);
      if( ct )
      {
        if( at )
        {
          glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
          glTexCoord2i(  0, cw );    glVertex2i(  0, cw );
          glTexCoord2i( ch, cw );    glVertex2i( ch, cw );
          glTexCoord2i( ch,  0 );    glVertex2i( ch,  0 );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
          glTexCoord2i( cw,  0 );    glVertex2i(  0, cw );
          glTexCoord2i( cw, ch );    glVertex2i( ch, cw );
          glTexCoord2i(  0, ch );    glVertex2i( ch,  0 );
        }
      }else
      {
        if( at )
        {
          glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
          glTexCoord2i(  0, cw );    glVertex2i( cw,  0 );
          glTexCoord2i( ch, cw );    glVertex2i( cw, ch );
          glTexCoord2i( ch,  0 );    glVertex2i(  0, ch );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
          glTexCoord2i( cw,  0 );    glVertex2i( cw,  0 );
          glTexCoord2i( cw, ch );    glVertex2i( cw, ch );
          glTexCoord2i(  0, ch );    glVertex2i(  0, ch );
        }
      }
    glEnd();

    cgGLDisableTextureParameter(rtm_a_add_b_src0);
    cgGLDisableTextureParameter(rtm_a_add_b_src1);
    cgGLDisableProfile( rtm_fragment_profile );

  c->EndCapture();
    sum_pass++;
}

// c = a + b'
void RTm::RTM__a_add_bt( 
  RenderBuffer *a, int at,  
  RenderBuffer *b, FLOAT4 sb,
  RenderBuffer *c, int cw, int ch, int ct 
){
  int _cw, _ch;
    _cw = c->GetWidth();
    _ch = c->GetHeight();

  c->BeginCapture();

    glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
        gluOrtho2D( 0, _cw, 0, _ch );

    cgSetParameter4fv( rtm_a_add_bt_sb, (float*)&sb );

    cgGLSetTextureParameter( rtm_a_add_bt_src0, a->GetTextureID() );
    cgGLEnableTextureParameter( rtm_a_add_bt_src0 );

    cgGLSetTextureParameter( rtm_a_add_bt_src1, b->GetTextureID() );
    cgGLEnableTextureParameter( rtm_a_add_bt_src1 );

    cgGLEnableProfile( rtm_fragment_profile );

    cgGLBindProgram( rtm_a_add_bt );

    glBegin(GL_QUADS);
      if( ct )
      {
        if( at )
        {
          glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
          glTexCoord2i(  0, cw );    glVertex2i(  0, cw );
          glTexCoord2i( ch, cw );    glVertex2i( ch, cw );
          glTexCoord2i( ch,  0 );    glVertex2i( ch,  0 );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
          glTexCoord2i( cw,  0 );    glVertex2i(  0, cw );
          glTexCoord2i( cw, ch );    glVertex2i( ch, cw );
          glTexCoord2i(  0, ch );    glVertex2i( ch,  0 );
        }
      }else
      {
        if( at )
        {
          glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
          glTexCoord2i(  0, cw );    glVertex2i( cw,  0 );
          glTexCoord2i( ch, cw );    glVertex2i( cw, ch );
          glTexCoord2i( ch,  0 );    glVertex2i(  0, ch );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
          glTexCoord2i( cw,  0 );    glVertex2i( cw,  0 );
          glTexCoord2i( cw, ch );    glVertex2i( cw, ch );
          glTexCoord2i(  0, ch );    glVertex2i(  0, ch );
        }
      }
    glEnd();

    cgGLDisableTextureParameter(rtm_a_add_bt_src0);
    cgGLDisableTextureParameter(rtm_a_add_bt_src1);
    cgGLDisableProfile( rtm_fragment_profile );

  c->EndCapture();
    sum_pass++;
}

void RTm::RTM__a_add_bt_packed( 
  RenderBuffer *a, int at,
  RenderBuffer *b, FLOAT4 sb,
  RenderBuffer *c, int cw, int ch, int ct
){
  int _cw, _ch;
    _cw = c->GetWidth();
    _ch = c->GetHeight();

  c->BeginCapture();

    glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
        gluOrtho2D( 0, _cw, 0, _ch );

    cgSetParameter4fv(  rtm_a_add_bt_packed_sb, (float*)&sb );

    cgGLSetTextureParameter( rtm_a_add_bt_packed_src0, a->GetTextureID() );
    cgGLEnableTextureParameter( rtm_a_add_bt_packed_src0 );

    cgGLSetTextureParameter( rtm_a_add_bt_packed_src1, b->GetTextureID() );
    cgGLEnableTextureParameter( rtm_a_add_bt_packed_src1 );

    cgGLEnableProfile( rtm_fragment_profile );

    cgGLBindProgram( rtm_a_add_bt_packed );

    glBegin(GL_QUADS);
      if( ct )
      {
        if( at )
        {
          glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
          glTexCoord2i(  0, cw );    glVertex2i(  0, cw );
          glTexCoord2i( ch, cw );    glVertex2i( ch, cw );
          glTexCoord2i( ch,  0 );    glVertex2i( ch,  0 );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
          glTexCoord2i( cw,  0 );    glVertex2i(  0, cw );
          glTexCoord2i( cw, ch );    glVertex2i( ch, cw );
          glTexCoord2i(  0, ch );    glVertex2i( ch,  0 );
        }
      }else
      {
        if( at )
        {
          glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
          glTexCoord2i(  0, cw );    glVertex2i( cw,  0 );
          glTexCoord2i( ch, cw );    glVertex2i( cw, ch );
          glTexCoord2i( ch,  0 );    glVertex2i(  0, ch );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
          glTexCoord2i( cw,  0 );    glVertex2i( cw,  0 );
          glTexCoord2i( cw, ch );    glVertex2i( cw, ch );
          glTexCoord2i(  0, ch );    glVertex2i(  0, ch );
        }
      }
    glEnd();

    cgGLDisableTextureParameter(rtm_a_add_bt_packed_src0);
    cgGLDisableTextureParameter(rtm_a_add_bt_packed_src1);
    cgGLDisableProfile( rtm_fragment_profile );

  c->EndCapture();
    sum_pass++;
}

void RTm::RTM__at_add_bt_packed( 
  RenderBuffer *a, int at,
  RenderBuffer *b, FLOAT4 sb,
  RenderBuffer *c, int cw, int ch, int ct
){
  int _cw, _ch;
    _cw = c->GetWidth();
    _ch = c->GetHeight();

  c->BeginCapture();

    glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
        gluOrtho2D( 0, _cw, 0, _ch );

    cgSetParameter4fv( rtm_at_add_bt_packed_sb, (float*)&sb );

    cgGLSetTextureParameter( rtm_at_add_bt_packed_src0, a->GetTextureID() );
    cgGLEnableTextureParameter( rtm_at_add_bt_packed_src0 );

    cgGLSetTextureParameter( rtm_at_add_bt_packed_src1, b->GetTextureID() );
    cgGLEnableTextureParameter( rtm_at_add_bt_packed_src1 );

    cgGLEnableProfile( rtm_fragment_profile );

    cgGLBindProgram( rtm_at_add_bt_packed );

    glBegin(GL_QUADS);
      if( ct )
      {
        if( !at )
        {
          glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
          glTexCoord2i(  0, cw );    glVertex2i(  0, cw );
          glTexCoord2i( ch, cw );    glVertex2i( ch, cw );
          glTexCoord2i( ch,  0 );    glVertex2i( ch,  0 );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
          glTexCoord2i( cw,  0 );    glVertex2i(  0, cw );
          glTexCoord2i( cw, ch );    glVertex2i( ch, cw );
          glTexCoord2i(  0, ch );    glVertex2i( ch,  0 );
        }
      }else
      {
        if( !at )
        {
          glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
          glTexCoord2i(  0, cw );    glVertex2i( cw,  0 );
          glTexCoord2i( ch, cw );    glVertex2i( cw, ch );
          glTexCoord2i( ch,  0 );    glVertex2i(  0, ch );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i(  0,  0 );
          glTexCoord2i( cw,  0 );    glVertex2i( cw,  0 );
          glTexCoord2i( cw, ch );    glVertex2i( cw, ch );
          glTexCoord2i(  0, ch );    glVertex2i(  0, ch );
        }
      }
    glEnd();

    cgGLDisableTextureParameter(rtm_at_add_bt_packed_src0);
    cgGLDisableTextureParameter(rtm_at_add_bt_packed_src1);
    cgGLDisableProfile( rtm_fragment_profile );

  c->EndCapture();
    sum_pass++;
}


void RTm::packed_load( int width, int height, float *fm )
{
  GPf4 pf4;
    pf4.load( (width+1)/2, (height+1)/2 );

  float *f4;
  int i,j;

    for( j=0; j<height; j++ )
      for( i=0; i<width; i++, fm++ )
      {
        f4 = (float*)(&pf4.pm[j/2][i/2]);
        f4[ 2*(j%2) + i%2 ] = *fm;
      }

  packed_load( pf4 );
}

void RTm::RTM__colsumx( 
  RenderBuffer *a, int ax, int ay,
  RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
  int h0, int h1, int anc
){

  int _cw, _ch;
    _cw = c->GetWidth();
    _ch = c->GetHeight();

    c->BeginCapture();

      glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
          gluOrtho2D( 0, _cw, 0, _ch );


      cgSetParameter1f( rtm_colsumx_anc, anc );
      cgSetParameter1f( rtm_colsumx_h0, h0 );
      cgSetParameter1f( rtm_colsumx_h1, h1 );
      cgSetParameter2f( rtm_colsumx_ta0, ax,ay );
      
      cgGLSetTextureParameter( rtm_colsumx_src0, a->GetTextureID() );
      cgGLEnableTextureParameter( rtm_colsumx_src0 );
      cgGLEnableProfile( rtm_fragment_profile );
      cgGLBindProgram( rtm_colsumx );


      glBegin(GL_QUADS);
        if( ct )
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cy     , cx      );
          glTexCoord2i( cw,  0 );    glVertex2i( cy     , cx + cw );
          glTexCoord2i( cw, ch );    glVertex2i( cy + ch, cx + cw );
          glTexCoord2i(  0, ch );    glVertex2i( cy + ch, cx      );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cx     , cy      );
          glTexCoord2i( cw,  0 );    glVertex2i( cx + cw, cy      );
          glTexCoord2i( cw, ch );    glVertex2i( cx + cw, cy + ch );
          glTexCoord2i(  0, ch );    glVertex2i( cx     , cy + ch );
        }
      glEnd();

      cgGLDisableTextureParameter(rtm_colsumx_src0);
      cgGLDisableProfile( rtm_fragment_profile );
    c->EndCapture();
    sum_pass++;
}

void RTm::RTM__rowsumx( 
  RenderBuffer *a, int ax, int ay,
  RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
  int w0, int h1, int anc
){

  int _cw, _ch;
    _cw = c->GetWidth();
    _ch = c->GetHeight();

    c->BeginCapture();

      glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
          gluOrtho2D( 0, _cw, 0, _ch );


      cgSetParameter1f( rtm_rowsumx_anc, anc );
      cgSetParameter1f( rtm_rowsumx_w0, w0 );
      cgSetParameter1f( rtm_rowsumx_h1, h1 );
      cgSetParameter2f( rtm_rowsumx_ta0, ax,ay );
      
      cgGLSetTextureParameter( rtm_rowsumx_src0, a->GetTextureID() );
      cgGLEnableTextureParameter( rtm_rowsumx_src0 );
      cgGLEnableProfile( rtm_fragment_profile );
      cgGLBindProgram( rtm_rowsumx );


      glBegin(GL_QUADS);
        if( ct )
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cy     , cx      );
          glTexCoord2i( cw,  0 );    glVertex2i( cy     , cx + cw );
          glTexCoord2i( cw, ch );    glVertex2i( cy + ch, cx + cw );
          glTexCoord2i(  0, ch );    glVertex2i( cy + ch, cx      );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cx     , cy      );
          glTexCoord2i( cw,  0 );    glVertex2i( cx + cw, cy      );
          glTexCoord2i( cw, ch );    glVertex2i( cx + cw, cy + ch );
          glTexCoord2i(  0, ch );    glVertex2i( cx     , cy + ch );
        }
      glEnd();

      cgGLDisableTextureParameter(rtm_rowsumx_src0);
      cgGLDisableProfile( rtm_fragment_profile );
    c->EndCapture();
    sum_pass++;
}

void RTm::csumx( 
  RTm &a, 
  int x0, int y0,
  int x1, int y1, 
  int w0, int h0, int n
){
  RTm &c = *this;

  int h1 = (h0+CSUM_CONST-1)/CSUM_CONST;

  int cw, ch;
    cw = w0;
    ch = h1*n;

  if( a.ps != c.ps )
  {
    printf( "[Error] RTm::csumx, packed and non-packed mismatch.\n" );
    exit(-1);
  }

  int anc = a.ps && ( a.ts != c.ts );

  if( a.ts )
  {
    RTM__rowsumx( 
      a.rt(), y0,x0,
      c.rt(), x1,y1, cw,ch, c.ts, 
      h0, h1, anc
    );
  }else
  {
    RTM__colsumx( 
      a.rt(), x0,y0,
      c.rt(), x1,y1, cw,ch, c.ts, 
      h0, h1, anc
    );
  }

}































void RTm::emux_krc( 
  RTm &a, int ax, int ay,
  RTm &b, int bx, int by,
  int cx, int cy,
  int k 
){

  if( a.w != b.h )
  {
    printf( "[Error] RTm::emux_krc, Opps...\n" );
    exit(-1);
  }

  if( !( a.ps==b.ps && b.ps==ps ) )
  {
    printf( "[Error] RTm::emux_krc, packed and non-packed mismatch.\n" );
    exit(-1);
  }


  RTm &c = *this;

  if( ps )
  {
    int anc = ( a.ts != c.ts );
    if( !a.ts && !b.ts )
    {
      RTM__emux_krc_packed(
        a.rt(), ax, ay,
        b.rt(), bx, by, b.w, b.h, 
        c.rt(), cx, cy, c.w, c.h, c.ts, 
        k, anc
      );
    }
  
    if( !a.ts && b.ts )
    {
      RTM__emux_krr_packed(
        a.rt(), ax, ay,
        b.rt(), by, bx, b.h, b.w, 
        c.rt(), cx, cy, c.w, c.h, c.ts, 
        k, anc
      );
    }

    if( a.ts && !b.ts )
    {
      RTM__emux_kcc_packed(
        a.rt(), ay, ax,
        b.rt(), bx, by, b.w, b.h, 
        c.rt(), cx, cy, c.w, c.h, c.ts, 
        k, !anc
      );
    }

    if( a.ts && b.ts )
    {
      RTM__emux_kcr_packed(
        a.rt(), ay, ax,
        b.rt(), by, bx, b.h, b.w, 
        c.rt(), cx, cy, c.w, c.h, c.ts, 
        k, !anc
      );
    }  
  }else
  {
    if( !a.ts && !b.ts )
    {
      RTM__emux_krc(
        a.rt(), ax, ay,
        b.rt(), bx, by, b.w, b.h, 
        c.rt(), cx, cy, c.w, c.h, c.ts, 
        k 
      );
    }
  
    if( !a.ts && b.ts )
    {
      RTM__emux_krr(
        a.rt(), ax, ay,
        b.rt(), by, bx, b.h, b.w, 
        c.rt(), cx, cy, c.w, c.h, c.ts, 
        k 
      );
    }

    if( a.ts && !b.ts )
    {
      RTM__emux_kcc(
        a.rt(), ay, ax,
        b.rt(), bx, by, b.w, b.h, 
        c.rt(), cx, cy, c.w, c.h, c.ts, 
        k 
      );
    }

    if( a.ts && b.ts )
    {
      RTM__emux_kcr(
        a.rt(), ay, ax,
        b.rt(), by, bx, b.h, b.w, 
        c.rt(), cx, cy, c.w, c.h, c.ts, 
        k 
      );
    }
  }
}

void RTm::RTM__emux_kcc( 
  RenderBuffer *a, int ax, int ay,
  RenderBuffer *b, int bx, int by, int bw, int bh, 
  RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
  int k
){

  if( bw != cw )
  {
    printf( "[Error] RTm::RTM__emux_kcc, Opps...\n" );
    exit(-1);
  }

  int blk;
    blk = (bh+EMUX_CONST-1)/EMUX_CONST;
    if( ch%blk!=0 )
    {
      printf( "[Error] RTm::RTM__emux_kcc, Opps...\n" );
      exit(-1);
    }


  int _cw, _ch;
    _cw = c->GetWidth();
    _ch = c->GetHeight();


    c->BeginCapture();

      glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
          gluOrtho2D( 0, _cw, 0, _ch );

      cgGLSetTextureParameter( rtm_emux_kcc_src0, a->GetTextureID() );
      cgGLSetTextureParameter( rtm_emux_kcc_src1, b->GetTextureID() );

      cgSetParameter1f( rtm_emux_kcc_k, k+.5 );
      cgSetParameter1f( rtm_emux_kcc_blk, blk );
      cgSetParameter2f( rtm_emux_kcc_ta0, ax, ay );
      cgSetParameter2f( rtm_emux_kcc_tb0, bx, by );
      cgSetParameter1f( rtm_emux_kcc_ybound, by+bh );

      cgGLEnableTextureParameter( rtm_emux_kcc_src0 );
      cgGLEnableTextureParameter( rtm_emux_kcc_src1 );

      cgGLEnableProfile( rtm_fragment_profile );
      cgGLBindProgram( rtm_emux_kcc );

      glBegin(GL_QUADS);
        if( ct )
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cy     , cx      );
          glTexCoord2i( cw,  0 );    glVertex2i( cy     , cx + cw );
          glTexCoord2i( cw, ch );    glVertex2i( cy + ch, cx + cw );
          glTexCoord2i(  0, ch );    glVertex2i( cy + ch, cx      );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cx     , cy      );
          glTexCoord2i( cw,  0 );    glVertex2i( cx + cw, cy      );
          glTexCoord2i( cw, ch );    glVertex2i( cx + cw, cy + ch );
          glTexCoord2i(  0, ch );    glVertex2i( cx     , cy + ch );
        }
      glEnd();

    cgGLDisableTextureParameter(rtm_emux_kcc_src0);
    cgGLDisableTextureParameter(rtm_emux_kcc_src1);
    cgGLDisableProfile( rtm_fragment_profile );

    c->EndCapture();

    sum_pass++;
}

void RTm::RTM__emux_krc( 
  RenderBuffer *a, int ax, int ay,
  RenderBuffer *b, int bx, int by, int bw, int bh, 
  RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
  int k
){

  if( bw != cw )
  {
    printf( "[Error] RTm::RTM__emux_krc, Opps...\n" );
    exit(-1);
  }

  int blk;
    blk = (bh+EMUX_CONST-1)/EMUX_CONST;
    if( ch%blk!=0 )
    {
      printf( "[Error] RTm::RTM__emux_krc, Opps...\n" );
      exit(-1);
    }


  int _cw, _ch;
    _cw = c->GetWidth();
    _ch = c->GetHeight();


    c->BeginCapture();

      glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
          gluOrtho2D( 0, _cw, 0, _ch );

      cgGLSetTextureParameter( rtm_emux_krc_src0, a->GetTextureID() );
      cgGLSetTextureParameter( rtm_emux_krc_src1, b->GetTextureID() );

      cgSetParameter1f( rtm_emux_krc_k, k+.5 );
      cgSetParameter1f( rtm_emux_krc_blk, blk );
      cgSetParameter2f( rtm_emux_krc_ta0, ax, ay );
      cgSetParameter2f( rtm_emux_krc_tb0, bx, by );
      cgSetParameter1f( rtm_emux_krc_ybound, by+bh );

      cgGLEnableTextureParameter( rtm_emux_krc_src0 );
      cgGLEnableTextureParameter( rtm_emux_krc_src1 );

      cgGLEnableProfile( rtm_fragment_profile );
      cgGLBindProgram( rtm_emux_krc );

      glBegin(GL_QUADS);
        if( ct )
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cy     , cx      );
          glTexCoord2i( cw,  0 );    glVertex2i( cy     , cx + cw );
          glTexCoord2i( cw, ch );    glVertex2i( cy + ch, cx + cw );
          glTexCoord2i(  0, ch );    glVertex2i( cy + ch, cx      );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cx     , cy      );
          glTexCoord2i( cw,  0 );    glVertex2i( cx + cw, cy      );
          glTexCoord2i( cw, ch );    glVertex2i( cx + cw, cy + ch );
          glTexCoord2i(  0, ch );    glVertex2i( cx     , cy + ch );
        }
      glEnd();

    cgGLDisableTextureParameter(rtm_emux_krc_src0);
    cgGLDisableTextureParameter(rtm_emux_krc_src1);
    cgGLDisableProfile( rtm_fragment_profile );

    c->EndCapture();

    sum_pass++;
}

void RTm::RTM__emux_kcr( 
  RenderBuffer *a, int ax, int ay,
  RenderBuffer *b, int bx, int by, int bw, int bh, 
  RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
  int k
){

  if( bh != cw )
  {
    printf( "[Error] RTm::RTM__emux_kcr, Opps...\n" );
    exit(-1);
  }

  int blk;
    blk = (bw+EMUX_CONST-1)/EMUX_CONST;
    if( ch%blk!=0 )
    {
      printf( "[Error] RTm::RTM__emux_kcr, Opps...\n" );
      exit(-1);
    }


  int _cw, _ch;
    _cw = c->GetWidth();
    _ch = c->GetHeight();


    c->BeginCapture();

      glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
          gluOrtho2D( 0, _cw, 0, _ch );

      cgGLSetTextureParameter( rtm_emux_kcr_src0, a->GetTextureID() );
      cgGLSetTextureParameter( rtm_emux_kcr_src1, b->GetTextureID() );

      cgSetParameter1f( rtm_emux_kcr_k, k+.5 );
      cgSetParameter1f( rtm_emux_kcr_blk, blk );
      cgSetParameter2f( rtm_emux_kcr_ta0, ax, ay );
      cgSetParameter2f( rtm_emux_kcr_tb0, bx, by );
      cgSetParameter1f( rtm_emux_kcr_xbound, bx+bw );

      cgGLEnableTextureParameter( rtm_emux_kcr_src0 );
      cgGLEnableTextureParameter( rtm_emux_kcr_src1 );

      cgGLEnableProfile( rtm_fragment_profile );
      cgGLBindProgram( rtm_emux_kcr );

      glBegin(GL_QUADS);
        if( ct )
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cy     , cx      );
          glTexCoord2i( cw,  0 );    glVertex2i( cy     , cx + cw );
          glTexCoord2i( cw, ch );    glVertex2i( cy + ch, cx + cw );
          glTexCoord2i(  0, ch );    glVertex2i( cy + ch, cx      );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cx     , cy      );
          glTexCoord2i( cw,  0 );    glVertex2i( cx + cw, cy      );
          glTexCoord2i( cw, ch );    glVertex2i( cx + cw, cy + ch );
          glTexCoord2i(  0, ch );    glVertex2i( cx     , cy + ch );
        }
      glEnd();

    cgGLDisableTextureParameter(rtm_emux_kcr_src0);
    cgGLDisableTextureParameter(rtm_emux_kcr_src1);
    cgGLDisableProfile( rtm_fragment_profile );

    c->EndCapture();

    sum_pass++;
}

void RTm::RTM__emux_krr( 
  RenderBuffer *a, int ax, int ay,
  RenderBuffer *b, int bx, int by, int bw, int bh, 
  RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
  int k
){

  if( bh != cw )
  {
    printf( "[Error] RTm::RTM__emux_krr, Opps...\n" );
    exit(-1);
  }

  int blk;
    blk = (bw+EMUX_CONST-1)/EMUX_CONST;
    if( ch%blk!=0 )
    {
      printf( "[Error] RTm::RTM__emux_krr, Opps...\n" );
      exit(-1);
    }


  int _cw, _ch;
    _cw = c->GetWidth();
    _ch = c->GetHeight();


    c->BeginCapture();

      glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
          gluOrtho2D( 0, _cw, 0, _ch );

      cgGLSetTextureParameter( rtm_emux_krr_src0, a->GetTextureID() );
      cgGLSetTextureParameter( rtm_emux_krr_src1, b->GetTextureID() );

      cgSetParameter1f( rtm_emux_krr_k, k+.5 );
      cgSetParameter1f( rtm_emux_krr_blk, blk );
      cgSetParameter2f( rtm_emux_krr_ta0, ax, ay );
      cgSetParameter2f( rtm_emux_krr_tb0, bx, by );
      cgSetParameter1f( rtm_emux_krr_xbound, bx+bw );

      cgGLEnableTextureParameter( rtm_emux_krr_src0 );
      cgGLEnableTextureParameter( rtm_emux_krr_src1 );

      cgGLEnableProfile( rtm_fragment_profile );
      cgGLBindProgram( rtm_emux_krr );

      glBegin(GL_QUADS);
        if( ct )
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cy     , cx      );
          glTexCoord2i( cw,  0 );    glVertex2i( cy     , cx + cw );
          glTexCoord2i( cw, ch );    glVertex2i( cy + ch, cx + cw );
          glTexCoord2i(  0, ch );    glVertex2i( cy + ch, cx      );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cx     , cy      );
          glTexCoord2i( cw,  0 );    glVertex2i( cx + cw, cy      );
          glTexCoord2i( cw, ch );    glVertex2i( cx + cw, cy + ch );
          glTexCoord2i(  0, ch );    glVertex2i( cx     , cy + ch );
        }
      glEnd();

    cgGLDisableTextureParameter(rtm_emux_krr_src0);
    cgGLDisableTextureParameter(rtm_emux_krr_src1);
    cgGLDisableProfile( rtm_fragment_profile );

    c->EndCapture();

    sum_pass++;
}



void RTm::packed_load( GPf4 &pf4 )
{
  load( pf4.w, pf4.h, true );
  ps = true;
  ls = true;
  rt()->Bind();
  glTexImage2D( GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGBA32_NV, pf4.w, pf4.h, 0, GL_RGBA, GL_FLOAT, pf4.fm );
}

RTm RTm::unpack()
{
  if( !ps )
  {
    printf( "[Error] RTm::unpack, trying to unpack non-packed RTm.\n" );
    exit(-1);
  }

  RTm &a = *this;
  RTm c;
    c.load( 2*w, 2*h );

  RTM__unpack(
    a.rt(), 0,0,c.w,c.h, a.ts,
    c.rt(), 0,0,c.w,c.h, c.ts
  );

  return c;
}

void RTm::load( GPfm &pfm )
{
  load( pfm.w, pfm.h, true );
  ls = true;
  //rt()->Bind();
  //glTexImage2D( rt()->GetTextureTarget(), 0, GL_FLOAT_RGB32_NV, pfm.w, pfm.h, 0, GL_RGB, GL_FLOAT, pfm.fm );

  //rt()->BeginCapture();
  //glDrawPixels( w,h, GL_RGB, GL_FLOAT, pfm.fm );
  //rt()->EndCapture();

  static GLuint tex = 0;
  if( tex==0 )
  {
    glGenTextures(1, &tex);
      glBindTexture(GL_TEXTURE_RECTANGLE_NV, tex);
        glTexParameterf( GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameterf( GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameterf( GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf( GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  glBindTexture(GL_TEXTURE_RECTANGLE_NV, tex);
  glTexImage2D( GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGB32_NV, pfm.w, pfm.h, 0, GL_RGB, GL_FLOAT, pfm.fm );

  rt()->BeginCapture();

  int _cw,_ch;
    _cw = rt()->GetWidth();
    _ch = rt()->GetHeight();

    glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
        gluOrtho2D( 0, _cw, 0, _ch );

    cgSetParameter1f( rtm_draw_anc, 0 );
    cgSetParameter4f( rtm_draw_scale, 1,1,1,1 );
    cgGLSetTextureParameter( rtm_draw_src0, tex );
    cgGLEnableTextureParameter( rtm_draw_src0 );

    cgGLEnableProfile( rtm_fragment_profile );

    cgGLBindProgram( rtm_draw );

    glBegin(GL_QUADS);
      glTexCoord2i( 0 + 0, 0 + 0 );    glVertex2i( 0 + 0, 0 + 0 );
      glTexCoord2i( 0 + 0, 0 + h );    glVertex2i( 0 + 0, 0 + h );
      glTexCoord2i( 0 + w, 0 + h );    glVertex2i( 0 + w, 0 + h );
      glTexCoord2i( 0 + w, 0 + 0 );    glVertex2i( 0 + w, 0 + 0 );
    glEnd();

    cgGLDisableTextureParameter(rtm_draw_src0);
    cgGLDisableProfile( rtm_fragment_profile );

  rt()->EndCapture();
  
  
  
  
}

void RTm::load( GPf1 &pf1 )
{
  load( pf1.w, pf1.h, true );
  ls = true;
  //rt()->Bind();
  //glTexImage2D( rt()->GetTextureTarget(), 0, GL_FLOAT_RGB32_NV, pfm.w, pfm.h, 0, GL_RGB, GL_FLOAT, pfm.fm );

  //rt()->BeginCapture();
  //glDrawPixels( w,h, GL_RGB, GL_FLOAT, pfm.fm );
  //rt()->EndCapture();

  static GLuint tex = 0;
  if( tex==0 )
  {
    glGenTextures(1, &tex);
      glBindTexture(GL_TEXTURE_RECTANGLE_NV, tex);
        glTexParameterf( GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameterf( GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameterf( GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf( GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  glBindTexture(GL_TEXTURE_RECTANGLE_NV, tex);
  glTexImage2D( GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGB32_NV, pf1.w, pf1.h, 0, GL_RED, GL_FLOAT, pf1.fm );

  rt()->BeginCapture();

  int _cw,_ch;
    _cw = rt()->GetWidth();
    _ch = rt()->GetHeight();

    glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
        gluOrtho2D( 0, _cw, 0, _ch );

    cgSetParameter1f( rtm_draw_anc, 0 );
    cgSetParameter4f( rtm_draw_scale, 1,1,1,1 );
    cgGLSetTextureParameter( rtm_draw_src0, tex );
    cgGLEnableTextureParameter( rtm_draw_src0 );

    cgGLEnableProfile( rtm_fragment_profile );

    cgGLBindProgram( rtm_draw );

    glBegin(GL_QUADS);
      glTexCoord2i( 0 + 0, 0 + 0 );    glVertex2i( 0 + 0, 0 + 0 );
      glTexCoord2i( 0 + 0, 0 + h );    glVertex2i( 0 + 0, 0 + h );
      glTexCoord2i( 0 + w, 0 + h );    glVertex2i( 0 + w, 0 + h );
      glTexCoord2i( 0 + w, 0 + 0 );    glVertex2i( 0 + w, 0 + 0 );
    glEnd();

    cgGLDisableTextureParameter(rtm_draw_src0);
    cgGLDisableProfile( rtm_fragment_profile );

  rt()->EndCapture();
  
  
  
  
}

void RTm::RTM__emux_krc_packed( 
  RenderBuffer *a, int ax, int ay, 
  RenderBuffer *b, int bx, int by, int bw, int bh,
  RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
  int k, int anc
){

  if( bw != cw )
  {
    printf( "[Error] RTm::RTM__emux_krc_packed, Opps...\n" );
    exit(-1);
  }

  int blk;
    blk = (bh+EMUX_CONST-1)/EMUX_CONST;
    if( ch%blk!=0 )
    {
      printf( "[Error] RTm::RTM__emux_krc_packed, Opps...\n" );
      exit(-1);
    }


  int _cw, _ch;
    _cw = c->GetWidth();
    _ch = c->GetHeight();


    c->BeginCapture();

      glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
          gluOrtho2D( 0, _cw, 0, _ch );

      cgGLSetTextureParameter( rtm_emux_krc_packed_src0, a->GetTextureID() );
      cgGLSetTextureParameter( rtm_emux_krc_packed_src1, b->GetTextureID() );

      cgSetParameter1f( rtm_emux_krc_packed_anc, anc );
      cgSetParameter1f( rtm_emux_krc_packed_k, k+.5 );
      cgSetParameter1f( rtm_emux_krc_packed_blk, blk );
      cgSetParameter2f( rtm_emux_krc_packed_ta0, ax, ay );
      cgSetParameter2f( rtm_emux_krc_packed_tb0, bx, by );
      cgSetParameter1f( rtm_emux_krc_packed_ybound, by+bh );

      cgGLEnableTextureParameter( rtm_emux_krc_packed_src0 );
      cgGLEnableTextureParameter( rtm_emux_krc_packed_src1 );

      cgGLEnableProfile( rtm_fragment_profile );
      cgGLBindProgram( rtm_emux_krc_packed );

      glBegin(GL_QUADS);
        if( ct )
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cy     , cx      );
          glTexCoord2i( cw,  0 );    glVertex2i( cy     , cx + cw );
          glTexCoord2i( cw, ch );    glVertex2i( cy + ch, cx + cw );
          glTexCoord2i(  0, ch );    glVertex2i( cy + ch, cx      );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cx     , cy      );
          glTexCoord2i( cw,  0 );    glVertex2i( cx + cw, cy      );
          glTexCoord2i( cw, ch );    glVertex2i( cx + cw, cy + ch );
          glTexCoord2i(  0, ch );    glVertex2i( cx     , cy + ch );
        }
      glEnd();

    cgGLDisableTextureParameter(rtm_emux_krc_packed_src0);
    cgGLDisableTextureParameter(rtm_emux_krc_packed_src1);
    cgGLDisableProfile( rtm_fragment_profile );

    c->EndCapture();

    sum_pass++;
}

void RTm::RTM__emux_kcc_packed( 
  RenderBuffer *a, int ax, int ay, 
  RenderBuffer *b, int bx, int by, int bw, int bh,
  RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
  int k, int anc
){

  if( bw != cw )
  {
    printf( "[Error] RTm::RTM__emux_kcc_packed, Opps...\n" );
    exit(-1);
  }

  int blk;
    blk = (bh+EMUX_CONST-1)/EMUX_CONST;
    if( ch%blk!=0 )
    {
      printf( "[Error] RTm::RTM__emux_kcc_packed, Opps...\n" );
      exit(-1);
    }


  int _cw, _ch;
    _cw = c->GetWidth();
    _ch = c->GetHeight();


    c->BeginCapture();

      glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
          gluOrtho2D( 0, _cw, 0, _ch );

      cgGLSetTextureParameter( rtm_emux_kcc_packed_src0, a->GetTextureID() );
      cgGLSetTextureParameter( rtm_emux_kcc_packed_src1, b->GetTextureID() );

      cgSetParameter1f( rtm_emux_kcc_packed_anc, anc );
      cgSetParameter1f( rtm_emux_kcc_packed_k, k+.5 );
      cgSetParameter1f( rtm_emux_kcc_packed_blk, blk );
      cgSetParameter2f( rtm_emux_kcc_packed_ta0, ax, ay );
      cgSetParameter2f( rtm_emux_kcc_packed_tb0, bx, by );
      cgSetParameter1f( rtm_emux_kcc_packed_ybound, by+bh );

      cgGLEnableTextureParameter( rtm_emux_kcc_packed_src0 );
      cgGLEnableTextureParameter( rtm_emux_kcc_packed_src1 );

      cgGLEnableProfile( rtm_fragment_profile );
      cgGLBindProgram( rtm_emux_kcc_packed );

      glBegin(GL_QUADS);
        if( ct )
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cy     , cx      );
          glTexCoord2i( cw,  0 );    glVertex2i( cy     , cx + cw );
          glTexCoord2i( cw, ch );    glVertex2i( cy + ch, cx + cw );
          glTexCoord2i(  0, ch );    glVertex2i( cy + ch, cx      );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cx     , cy      );
          glTexCoord2i( cw,  0 );    glVertex2i( cx + cw, cy      );
          glTexCoord2i( cw, ch );    glVertex2i( cx + cw, cy + ch );
          glTexCoord2i(  0, ch );    glVertex2i( cx     , cy + ch );
        }
      glEnd();

    cgGLDisableTextureParameter(rtm_emux_kcc_packed_src0);
    cgGLDisableTextureParameter(rtm_emux_kcc_packed_src1);
    cgGLDisableProfile( rtm_fragment_profile );

    c->EndCapture();

    sum_pass++;
}

void RTm::RTM__emux_kcr_packed( 
  RenderBuffer *a, int ax, int ay, 
  RenderBuffer *b, int bx, int by, int bw, int bh,
  RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
  int k, int anc
){

  if( bh != cw )
  {
    printf( "[Error] RTm::RTM__emux_kcr_packed, Opps...\n" );
    exit(-1);
  }

  int blk;
    blk = (bw+EMUX_CONST-1)/EMUX_CONST;
    if( ch%blk!=0 )
    {
      printf( "[Error] RTm::RTM__emux_kcr_packed, Opps...\n" );
      exit(-1);
    }


  int _cw, _ch;
    _cw = c->GetWidth();
    _ch = c->GetHeight();


    c->BeginCapture();

      glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
          gluOrtho2D( 0, _cw, 0, _ch );

      cgGLSetTextureParameter( rtm_emux_kcr_packed_src0, a->GetTextureID() );
      cgGLSetTextureParameter( rtm_emux_kcr_packed_src1, b->GetTextureID() );

      cgSetParameter1f( rtm_emux_kcr_packed_anc, anc );
      cgSetParameter1f( rtm_emux_kcr_packed_k, k+.5 );
      cgSetParameter1f( rtm_emux_kcr_packed_blk, blk );
      cgSetParameter2f( rtm_emux_kcr_packed_ta0, ax, ay );
      cgSetParameter2f( rtm_emux_kcr_packed_tb0, bx, by );
      cgSetParameter1f( rtm_emux_kcr_packed_xbound, bx+bw );

      cgGLEnableTextureParameter( rtm_emux_kcr_packed_src0 );
      cgGLEnableTextureParameter( rtm_emux_kcr_packed_src1 );

      cgGLEnableProfile( rtm_fragment_profile );
      cgGLBindProgram( rtm_emux_kcr_packed );

      glBegin(GL_QUADS);
        if( ct )
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cy     , cx      );
          glTexCoord2i( cw,  0 );    glVertex2i( cy     , cx + cw );
          glTexCoord2i( cw, ch );    glVertex2i( cy + ch, cx + cw );
          glTexCoord2i(  0, ch );    glVertex2i( cy + ch, cx      );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cx     , cy      );
          glTexCoord2i( cw,  0 );    glVertex2i( cx + cw, cy      );
          glTexCoord2i( cw, ch );    glVertex2i( cx + cw, cy + ch );
          glTexCoord2i(  0, ch );    glVertex2i( cx     , cy + ch );
        }
      glEnd();

    cgGLDisableTextureParameter(rtm_emux_kcr_packed_src0);
    cgGLDisableTextureParameter(rtm_emux_kcr_packed_src1);
    cgGLDisableProfile( rtm_fragment_profile );

    c->EndCapture();

    sum_pass++;
}

void RTm::RTM__emux_krr_packed( 
  RenderBuffer *a, int ax, int ay, 
  RenderBuffer *b, int bx, int by, int bw, int bh,
  RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
  int k, int anc
){

  if( bh != cw )
  {
    printf( "[Error] RTm::RTM__emux_krr_packed, Opps...\n" );
    exit(-1);
  }

  int blk;
    blk = (bw+EMUX_CONST-1)/EMUX_CONST;
    if( ch%blk!=0 )
    {
      printf( "[Error] RTm::RTM__emux_krr_packed, Opps...\n" );
      exit(-1);
    }


  int _cw, _ch;
    _cw = c->GetWidth();
    _ch = c->GetHeight();


    c->BeginCapture();

      glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
          gluOrtho2D( 0, _cw, 0, _ch );

      cgGLSetTextureParameter( rtm_emux_krr_packed_src0, a->GetTextureID() );
      cgGLSetTextureParameter( rtm_emux_krr_packed_src1, b->GetTextureID() );

      cgSetParameter1f( rtm_emux_krr_packed_anc, anc );
      cgSetParameter1f( rtm_emux_krr_packed_k, k+.5 );
      cgSetParameter1f( rtm_emux_krr_packed_blk, blk );
      cgSetParameter2f( rtm_emux_krr_packed_ta0, ax, ay );
      cgSetParameter2f( rtm_emux_krr_packed_tb0, bx, by );
      cgSetParameter1f( rtm_emux_krr_packed_xbound, bx+bw );

      cgGLEnableTextureParameter( rtm_emux_krr_packed_src0 );
      cgGLEnableTextureParameter( rtm_emux_krr_packed_src1 );

      cgGLEnableProfile( rtm_fragment_profile );
      cgGLBindProgram( rtm_emux_krr_packed );

      glBegin(GL_QUADS);
        if( ct )
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cy     , cx      );
          glTexCoord2i( cw,  0 );    glVertex2i( cy     , cx + cw );
          glTexCoord2i( cw, ch );    glVertex2i( cy + ch, cx + cw );
          glTexCoord2i(  0, ch );    glVertex2i( cy + ch, cx      );
        }else
        {
          glTexCoord2i(  0,  0 );    glVertex2i( cx     , cy      );
          glTexCoord2i( cw,  0 );    glVertex2i( cx + cw, cy      );
          glTexCoord2i( cw, ch );    glVertex2i( cx + cw, cy + ch );
          glTexCoord2i(  0, ch );    glVertex2i( cx     , cy + ch );
        }
      glEnd();

    cgGLDisableTextureParameter(rtm_emux_krr_packed_src0);
    cgGLDisableTextureParameter(rtm_emux_krr_packed_src1);
    cgGLDisableProfile( rtm_fragment_profile );

    c->EndCapture();

    sum_pass++;
}
