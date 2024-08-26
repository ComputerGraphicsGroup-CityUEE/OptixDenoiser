#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>

#include <GL/glew.h>
#include <GL/glut.h>

#pragma comment (lib, "glew32.lib")    /* link with glew32 lib */


#include "g_common.h"
#include "g_pfm.h"
#include "g_pqm.h"




GPqm::GPqm()
{
  memset( this, 0, sizeof(GPqm) );
}

void GPqm::safe_load( int width, int height, int depth, int internalformat )
{
  depth  = pow( 2, ceil( log(depth) /log(2) ) );
  load( width, height, depth, internalformat );
}


void GPqm::load( int width, int height, int depth, int internalformat )
{
  if( 
    width  != pow( 2, ceil( log(width) /log(2) ) ) ||
    height != pow( 2, ceil( log(height)/log(2) ) ) ||
    depth  != pow( 2, ceil( log(depth) /log(2) ) )
  ){
    printf( "[Warning] : GPqm::load, not all width, height and depth are of power of 2.\n" );
    //exit( -1 );
  }

  SAFE_FREE( lm );
  SAFE_FREE( qm );
  SAFE_FREE( sf );
  SAFE_FREE( nf );

  w = width;
  h = height;
  d = depth;
  format = internalformat;

  sf = (FLOAT3*) malloc( d * sizeof(FLOAT3) );
  nf = (FLOAT3*) malloc( d * sizeof(FLOAT3) );

  
  memset( sf, 0, d * sizeof(FLOAT3) );
  memset( nf, 0, d * sizeof(FLOAT3) );

  switch( format )
  {
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      lm_size = w * h * 3 / 6;
      break;

    case GL_RGB:
      lm_size = w * h * 3;
      break;

    default:
      printf( "[Error] : GPqm::load, unsupported internal format.\n" );
      exit( -1 );
      break;
  };

  qm_size = d  *  lm_size;
  qm = (unsigned char*) malloc( d  *  lm_size * sizeof(unsigned char) );
  memset( qm, 0, d  *  lm_size * sizeof(unsigned char) );

  lm = (unsigned char*) malloc( lm_size * sizeof(unsigned char) );

}

GPqm::~GPqm()
{
  SAFE_FREE( lm );
  SAFE_FREE( qm );
  SAFE_FREE( sf );
  SAFE_FREE( nf );

  if( win_id )
    glutDestroyWindow( win_id );

  if( tex_id )
    glDeleteTextures( 1, &tex_id );

}

void GPqm::load( int level, const FLOAT3 *fm, FLOAT3 lower_bound, FLOAT3 upper_bound )
{
  if( glutGetWindow() == 0 )
    win_id = glutCreateWindow( "GPqm" );

  if( tex_id == 0  )
    glGenTextures( 1, &tex_id );


  GPfm tmp;
    tmp.load( w, h, fm );

  sf[level] = upper_bound-lower_bound;
  nf[level] = lower_bound;
  tmp.add(   -nf[level] );
  tmp.mul(  1/sf[level] );


  glBindTexture( GL_TEXTURE_2D, tex_id );
    glTexImage2D( GL_TEXTURE_2D, 0, format, w, h, 0, GL_RGB, GL_FLOAT, tmp.fm );
  switch( format )
  {
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      glGetCompressedTexImageARB=(PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)wglGetProcAddress("glGetCompressedTexImageARB");
      glGetCompressedTexImageARB( GL_TEXTURE_2D, 0, lm );
      {
        int blk;
        if( d == 1 ) blk = 1;
        if( d == 2 ) blk = 2;
        if( d > 2 ) blk = 4;

        unsigned char *tqm = qm + ( level/blk * lm_size *blk )  + level%blk * 8;
        unsigned char *tlm = lm;

        int i;
        for( i=0; i<lm_size; i+=8, tqm+=blk*8, tlm+=8 )
          memcpy( tqm, tlm, 8 );
      }
      break;

    case GL_RGB:
      glGetTexImage( GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, lm );
      {
        memcpy( qm+level*lm_size, lm, lm_size );
      }
      break;

    default:
      printf( "[Error] : GPqm::load, unsupported internal format.\n" );
      exit( -1 );
      break;
  };


/*
int internalformat;
int compressed_size;
int compressed;  // will be GL_TRUE or GL_FALSE

glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed);
glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalformat);
glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &compressed_size);
*/
}
void GPqm::load( int level, const FLOAT3 *fm, float lower_bound, float upper_bound )
{
  load( level, fm, FLOAT3(lower_bound), FLOAT3(upper_bound) );
}

void GPqm::load( int level, const FLOAT3 *fm )
{
  GPfm tmp;
    tmp.load( w, h, fm );
  FLOAT3 hmin, hmax;
    hmin = tmp.vmin();
    hmax = tmp.vmax();

  load( level, fm, hmin, hmax );
}


void GPqm::save( const char *spath, const char *type )
{
  if( strcmp( type, "pqm")==0 )
  {
    FILE *f0 = fopen( spath, "wb" );

      fprintf( f0, "PQM %i\n", format );
      fprintf( f0, "%i %i %i\n", w, h, d );

      fwrite( sf, sizeof(FLOAT3), d, f0 );
      fwrite( nf, sizeof(FLOAT3), d, f0 );
      fwrite( qm, 1, lm_size*d, f0 );

    fclose(f0);
  }else
  {
    printf( "[Error] GPqm::save, unsupported file format.\n" );
    exit(-1);
  }
}


void GPqm::load( const char *spath )
{
  int width, height, depth, internalformat;

  char str[256];
  char pqm_id[256];

  FILE *f0 = fopen( spath, "rb" );

    if( f0 == NULL )
    {
      printf( "[Error] GPqm::load, file %s not found.\n", spath );
      exit(-1);
    }

    fgets( str, 256, f0 );
    sscanf( str, "%s %i", pqm_id, &internalformat );
    fgets( str, 256, f0 );
    sscanf( str, "%i %i %i", &width, &height, &depth );
    fclose(f0);

  if( strcmp( pqm_id, "PQM" )!=0 )
  {
    printf( "[Error] GPqm::load, not a pqm file.\n" );
    exit(-1);
  }

  if( 
    internalformat != GL_COMPRESSED_RGB_S3TC_DXT1_EXT &&
    internalformat != GL_RGB
  ){
    printf( "[Error] GPqm::load, unsupport compression format.\n" );
    exit(-1);
  }

  load( width, height, depth, internalformat );

  f0 = fopen( spath, "rb" );

    fgets( str, 256, f0 );
    fgets( str, 256, f0 );

    fread( sf, sizeof(FLOAT3), d, f0 );
    fread( nf, sizeof(FLOAT3), d, f0 );

    fread( qm, 1, lm_size*d, f0 );

  fclose(f0);
}

void* GPqm::get_lm( int level )
{

  switch( format )
  {
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      {
        int blk;
        if( d == 1 ) blk = 1;
        if( d == 2 ) blk = 2;
        if( d > 2 ) blk = 4;

        unsigned char *tqm = qm + ( level/blk * lm_size *blk )  + level%blk * 8;
        unsigned char *tlm = lm;

        int i;
        for( i=0; i<lm_size; i+=8, tqm+=blk*8, tlm+=8 )
          memcpy( tlm, tqm, 8 );
      }
      break;

    case GL_RGB:
      {
        memcpy( lm, qm+level*lm_size, lm_size );
      }
      break;

    default:
      printf( "[Error] : GPqm::get_lm, unsupported internal format.\n" );
      exit( -1 );
      break;
  };

  return lm;
}

void GPqm::TexImage3D()
{

  switch( format )
  {
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      glCompressedTexImage3DARB = (PFNGLCOMPRESSEDTEXIMAGE3DARBPROC) wglGetProcAddress("glCompressedTexImage3DARB");
      glCompressedTexImage3DARB( 
        GL_TEXTURE_3D, 0, 
        GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 
        w, h, d, 0,
        qm_size, qm
      );
    break;

    case GL_RGB:
      glTexImage3D = (PFNGLTEXIMAGE3DPROC) wglGetProcAddress("glTexImage3D");
      glTexImage3D( GL_TEXTURE_3D, 0, GL_RGB, w, h, d, 0, GL_RGB, GL_UNSIGNED_BYTE, qm );
    break;
  }
}

void GPqm::TexImage2D( int level )
{

  switch( format )
  {
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      glCompressedTexImage2DARB = (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC) wglGetProcAddress("glCompressedTexImage2DARB");
      glCompressedTexImage2DARB( 
        GL_TEXTURE_2D, 0, 
        GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 
        w, h, 0,
        lm_size, get_lm(level)
      );
    break;

    case GL_RGB:
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, get_lm(level) );
    break;
  }
}

void GPqm::TexImage1D()
{

  if( d!=1 || h!=1 )
  {
    printf( "[Error] GPqm::TexImage1D, height and depth must be 1 for 1D texture.\n" );
    exit(-1);
  }

  switch( format )
  {
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      glCompressedTexImage1DARB = (PFNGLCOMPRESSEDTEXIMAGE1DARBPROC) wglGetProcAddress("glCompressedTexImage1DARB");
      glCompressedTexImage1DARB( 
        GL_TEXTURE_1D, 0, 
        GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 
        w, 0,
        qm_size, qm
      );
    break;

    case GL_RGB:
      glTexImage1D( GL_TEXTURE_1D, 0, GL_RGB, w, 0, GL_RGB, GL_UNSIGNED_BYTE, qm );
    break;
  }
}



GPq4::GPq4()
{
  memset( this, 0, sizeof(GPq4) );
}

GPq4::~GPq4()
{
  SAFE_FREE( lm );
  SAFE_FREE( qm );
  SAFE_FREE( sf );
  SAFE_FREE( nf );

  if( win_id )
    glutDestroyWindow( win_id );

  if( tex_id )
    glDeleteTextures( 1, &tex_id );

}

void GPq4::safe_load( int width, int height, int depth, int internalformat )
{
  depth  = pow( 2, ceil( log(depth) /log(2) ) );
  load( width, height, depth, internalformat );
}

void GPq4::load( int width, int height, int depth, int internalformat )
{
  if( 
    width  != pow( 2, ceil( log(width) /log(2) ) ) ||
    height != pow( 2, ceil( log(height)/log(2) ) ) ||
    depth  != pow( 2, ceil( log(depth) /log(2) ) )
  ){
    printf( "[warning] : GPq4::load, not all width, height and depth are of power of 2.\n" );
  }

  SAFE_FREE( lm );
  SAFE_FREE( qm );
  SAFE_FREE( sf );
  SAFE_FREE( nf );

  w = width;
  h = height;
  d = depth;
  format = internalformat;

  sf = (FLOAT4*) malloc( d * sizeof(FLOAT4) );
  nf = (FLOAT4*) malloc( d * sizeof(FLOAT4) );

  
  memset( sf, 0, d * sizeof(FLOAT4) );
  memset( nf, 0, d * sizeof(FLOAT4) );

  switch( format )
  {
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      lm_size = w * h;
      break;

    case GL_RGBA:
      lm_size = w * h * 4;
      break;

    default:
      printf( "[Error] : GPq4::load, unsupported internal format.\n" );
      exit( -1 );
      break;
  };

  qm_size = d  *  lm_size;
  qm = (unsigned char*) malloc( d  *  lm_size * sizeof(unsigned char) );
  memset( qm, 0, d  *  lm_size * sizeof(unsigned char) );

  lm = (unsigned char*) malloc( lm_size * sizeof(unsigned char) );

}

void GPq4::load( const char *spath )
{
  int width, height, depth, internalformat;

  char str[256];
  char pqm_id[256];

  FILE *f0 = fopen( spath, "rb" );

    if( f0 == NULL )
    {
      printf( "[Error] GPq4::load, file %s not found.\n", spath );
      exit(-1);
    }

    fgets( str, 256, f0 );
    sscanf( str, "%s %i", pqm_id, &internalformat );
    fgets( str, 256, f0 );
    sscanf( str, "%i %i %i", &width, &height, &depth );
    fclose(f0);

  if( strcmp( pqm_id, "PQ4" )!=0 )
  {
    printf( "[Error] GPq4::load, not a pq4 file.\n" );
    exit(-1);
  }

  if( 
    internalformat != GL_COMPRESSED_RGBA_S3TC_DXT5_EXT &&
    internalformat != GL_RGBA
  ){
    printf( "[Error] GPq4::load, unsupport compression format.\n" );
    exit(-1);
  }

  load( width, height, depth, internalformat );

  f0 = fopen( spath, "rb" );

    fgets( str, 256, f0 );
    fgets( str, 256, f0 );

    fread( sf, sizeof(FLOAT4), d, f0 );
    fread( nf, sizeof(FLOAT4), d, f0 );

    fread( qm, 1, lm_size*d, f0 );

  fclose(f0);
}

void GPq4::load( int level, const FLOAT4 *fm )
{
  GPf4 tmp;
    tmp.load( w, h, fm );
  FLOAT4 hmin, hmax;
    hmin = tmp.vmin();
    hmax = tmp.vmax();

  load( level, fm, hmin, hmax );

  // GPf4 tmp;
  //   tmp.load( w, h, fm );
  // FLOAT4 hmin, hmax, hmean, hvar, sd;
  //   hmin  = tmp.vmin();
  //   hmax  = tmp.vmax();
  //   hmean = tmp.vmean();
  //   hvar  = tmp.variance();
  //   sd  = FLOAT4( sqrtf(hvar.x), sqrtf(hvar.y), sqrtf(hvar.z), sqrtf(hvar.w) );
  // 
  // float ci = 6;
  // FLOAT4 lb, ub;
  //   lb = hmean - ci*sd;
  //   ub = hmean + ci*sd;
  // 
  // if( lb.x < hmin.x ) lb.x = hmin.x;
  // if( lb.y < hmin.y ) lb.y = hmin.y;
  // if( lb.z < hmin.z ) lb.z = hmin.z;
  // if( lb.w < hmin.w ) lb.w = hmin.w;
  // if( ub.x > hmax.x ) ub.x = hmax.x;
  // if( ub.y > hmax.y ) ub.y = hmax.y;
  // if( ub.z > hmax.z ) ub.z = hmax.z;
  // if( ub.w > hmax.w ) ub.w = hmax.w;
  // 
  // load( level, fm, lb, ub );
}

void GPq4::load( int level, const FLOAT3 *fm )
{
  GPf4 pf4;
    pf4.load( w,h );

  const FLOAT3 *tl = fm;
  FLOAT3 l;
  float lw;
  int i,j;

  for( j=0; j<h; j++ )
    for( i=0; i<w; i++, tl++ )
    {
      l = *tl;
      lw = sqrtf( vdot( &l,&l ) ); 

      if( lw>0 )
        pf4.pm[j][i] = FLOAT4( l.x/lw, l.y/lw, l.z/lw, lw );
      else
        pf4.pm[j][i] = 0;
    }

  load( level, pf4.fm );
}

void GPq4::load( int level, const FLOAT4 *fm, FLOAT4 lower_bound, FLOAT4 upper_bound )
{
  if( glutGetWindow() == 0 )
    win_id = glutCreateWindow( "GPq4" );

  if( tex_id == 0  )
    glGenTextures( 1, &tex_id );


  GPf4 tmp;
    tmp.load( w, h, fm );

  sf[level] = upper_bound-lower_bound;
  nf[level] = lower_bound;
  tmp.add(  -nf[level] );
  tmp.mul( 1/sf[level] );


  glBindTexture( GL_TEXTURE_2D, tex_id );
    glTexImage2D( GL_TEXTURE_2D, 0, format, w, h, 0, GL_RGBA, GL_FLOAT, tmp.fm );
  switch( format )
  {
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      glGetCompressedTexImageARB=(PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)wglGetProcAddress("glGetCompressedTexImageARB");
      glGetCompressedTexImageARB( GL_TEXTURE_2D, 0, lm );
      {
        int blk;
        if( d == 1 ) blk = 1;
        if( d == 2 ) blk = 2;
        if( d > 2 ) blk = 4;

        unsigned char *tqm = qm + ( level/blk * lm_size *blk )  + level%blk * 16;
        unsigned char *tlm = lm;

        int i;
        for( i=0; i<lm_size; i+=16, tqm+=blk*16, tlm+=16 )
          memcpy( tqm, tlm, 16 );
      }
      break;

    case GL_RGBA:
      glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, lm );
      {
        memcpy( qm+level*lm_size, lm, lm_size );
      }
      break;

    default:
      printf( "[Error] : GPq4::load, unsupported internal format.\n" );
      exit( -1 );
      break;
  };

}

void GPq4::save( const char *spath )
{
  FILE *f0 = fopen( spath, "wb" );
  
  fprintf( f0, "PQ4 %i\n", format );
  fprintf( f0, "%i %i %i\n", w, h, d );
  
  fwrite( sf, sizeof(FLOAT4), d, f0 );
  fwrite( nf, sizeof(FLOAT4), d, f0 );
  fwrite( qm, 1, lm_size*d, f0 );
  
  fclose(f0);
}

void* GPq4::get_lm( int level )
{
  switch( format )
  {
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      {
        int blk;
        if( d == 1 ) blk = 1;
        if( d == 2 ) blk = 2;
        if( d > 2 ) blk = 4;

        unsigned char *tqm = qm + ( level/blk * lm_size *blk )  + level%blk * 16;
        unsigned char *tlm = lm;

        int i;
        for( i=0; i<lm_size; i+=16, tqm+=blk*16, tlm+=16 )
          memcpy( tlm, tqm, 16 );
      }
      break;

    case GL_RGBA:
      {
        memcpy( lm, qm+level*lm_size, lm_size );
      }
      break;

    default:
      printf( "[Error] : GPqm::get_lm, unsupported internal format.\n" );
      exit( -1 );
      break;
  };

  return lm;
}

void GPq4::TexImage3D()
{
  switch( format )
  {
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      glCompressedTexImage3DARB = (PFNGLCOMPRESSEDTEXIMAGE3DARBPROC) wglGetProcAddress("glCompressedTexImage3DARB");
      glCompressedTexImage3DARB( 
        GL_TEXTURE_3D, 0, 
        GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 
        w, h, d, 0,
        qm_size, qm
      );
    break;

    case GL_RGBA:
      glTexImage3D = (PFNGLTEXIMAGE3DPROC) wglGetProcAddress("glTexImage3D");
      glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA, w, h, d, 0, GL_RGBA, GL_UNSIGNED_BYTE, qm );
    break;
  }
}

void GPq4::TexImage2D( int level )
{
  switch( format )
  {
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      glCompressedTexImage2DARB = (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC) wglGetProcAddress("glCompressedTexImage2DARB");
      glCompressedTexImage2DARB( 
        GL_TEXTURE_2D, 0, 
        GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 
        w, h, 0,
        lm_size, get_lm(level)
      );
    break;

    case GL_RGBA:
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, get_lm(level) );
    break;
  }
}

void GPq4::TexImage1D()
{
  if( d!=1 || h!=1 )
  {
    printf( "[Error] GPq4::TexImage1D, height and depth must be 1 for 1D texture.\n" );
    exit(-1);
  }

  switch( format )
  {
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      glCompressedTexImage1DARB = (PFNGLCOMPRESSEDTEXIMAGE1DARBPROC) wglGetProcAddress("glCompressedTexImage1DARB");
      glCompressedTexImage1DARB( 
        GL_TEXTURE_1D, 0, 
        GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 
        w, 0,
        qm_size, qm
      );
    break;

    case GL_RGBA:
      glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA, w, 0, GL_RGBA, GL_UNSIGNED_BYTE, qm );
    break;
  }
}
