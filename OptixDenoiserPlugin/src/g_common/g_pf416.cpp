#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <malloc.h>

#include "g_pf416.h"

GPf416::GPf416()
{
  w = 0;
  h = 0;
  fm = NULL;
  pm = NULL;
  byte16_align = false;
}

GPf416::~GPf416()
{
  if( byte16_align )
  {
    if(fm) _aligned_free( fm );
    SAFE_FREE( pm );
  }else
  {
    SAFE_FREE( fm );
    SAFE_FREE( pm );
  }
}

void GPf416::load16( int width, int height )
{
  if( byte16_align )
  {
    if(fm) _aligned_free( fm );
    SAFE_FREE( pm );
  }else
  {
    SAFE_FREE( fm );
    SAFE_FREE( pm );
  }

  byte16_align = true;

  w = width;
  h = height;
  fm = (FLOAT4*) _aligned_malloc( w * h * sizeof(FLOAT4), 16 );

  memset( fm, 0, w*h*sizeof(FLOAT4) );

  pm = (FLOAT4**) malloc( h * sizeof(FLOAT4*) );
  int j;
  for( j=0; j<h; j++ )
    pm[j] = &fm[j*w];
}


void GPf416::load( int width, int height )
{
  if( byte16_align )
  {
    if(fm) _aligned_free( fm );
    SAFE_FREE( pm );
  }else
  {
    SAFE_FREE( fm );
    SAFE_FREE( pm );
  }

  byte16_align = false;
  w = width;
  h = height;
  fm = (FLOAT4*) malloc( w * h * sizeof(FLOAT4) );

  memset( fm, 0, w*h*sizeof(FLOAT4) );

  pm = (FLOAT4**) malloc( h * sizeof(FLOAT4*) );
  int j;
  for( j=0; j<h; j++ )
    pm[j] = &fm[j*w];
}

void GPf416::flip_vertical()
{
  FLOAT4 *tline, *line0, *line1;
    tline = (FLOAT4*) malloc( w * sizeof(FLOAT4) );
    line0 = fm;
    line1 = fm + w*(h-1);

  int j, jlen = h/2;
  for( j=0; j<jlen; j++, line0+=w, line1-=w )
  {
    memcpy( tline, line0, w*sizeof(FLOAT4) );
    memcpy( line0, line1, w*sizeof(FLOAT4) );
    memcpy( line1, tline, w*sizeof(FLOAT4) );
  }

  free(tline);
}

void GPf416::load( const char *spath )
{
  char str[256], info[2];
  int width, height;
  short mode;


  FILE *f0 = fopen( spath, "rb" );
    if( f0==NULL )
    {
      printf( "[Error] : GPf416::load(), \"%s\" not found.\n", spath );
      exit(-1);
    }
    fread( &mode, sizeof(short), 1, f0 ); swapbyte( &mode, sizeof(mode) );
    fread( info, sizeof(char), 2, f0 );

  fclose(f0);


  if( mode=='pf'  )
  {
    FILE *f0 = fopen( spath, "rb" );

      float byte_ordering;
      if( info[0]=='\n' || info[1]=='\n' )
      {
        fgets( str, 256, f0 );
        while( fgets( str, 256, f0 ) && str[0] == '#' );
        sscanf(str, "%i %i", &width, &height);
        fgets( str, 256, f0 );
        sscanf(str, "%f", &byte_ordering);
      }else if( info[0]==' ' )
      {
        fgets( str, 256, f0 );
        char tmp[16];
        sscanf( str, "%s %i %i %f", tmp, &width, &height, &byte_ordering );
      }else if( info[0]=='\r' && info[1]!='\n' )
      {
        fscanf( f0, "%[^\r]", str ); fgetc(f0);
        do{ fscanf( f0, "%[^\r]", str ); fgetc(f0); }while( str[0] == '#' );
        sscanf(str, "%i %i", &width, &height);
        fscanf( f0, "%[^\r]", str ); fgetc(f0);
        sscanf(str, "%f", &byte_ordering);
      }else
      {
        printf( "[Error] GPf416::load, incorrect file format\n" );
        exit(-1);
      }

      load(width,height);
      fread( fm, sizeof(FLOAT4), w*h, f0 );
      flip_vertical();

      int i,j;
      if( byte_ordering==1 )
        for( j=0; j<h; j++ )
          for( i=0; i<w; i++ )
          {
            swapbyte( &pm[j][i].x, sizeof(float) );
            swapbyte( &pm[j][i].y, sizeof(float) );
            swapbyte( &pm[j][i].z, sizeof(float) );
            swapbyte( &pm[j][i].w, sizeof(float) );
          }

    fclose(f0);

  }else
  {
    printf( "[Error] : GPf416::load(), unsupported file format\n" );
    exit(-1);
  }
}

void GPf416::save( const char *spath )
{
  FILE *f0 = fopen( spath, "wb" );
    fprintf( f0, "pf\n");
	fprintf( f0, "%d %d\n", w, h );
	fprintf( f0, "%f\n", -1.f);
    this->flip_vertical();
    fwrite( fm, sizeof(FLOAT4), w*h, f0 );
    this->flip_vertical();
  fclose( f0 );
}

void GPf416::load( int width, int height, const float *r, const float *g, const float *b, const float *a )
{
  load( width, height );

  int i, l;
  l = w*h;
  for( i=0; i<l; i++ )
    fm[i] = FLOAT4( *r++, *g++, *b++, *a++ );
}

void GPf416::load( int width, int height, const FLOAT3 *rgb )
{
  load( width, height );

  int i, l;
  l = w*h;
  for( i=0; i<l; i++, rgb++ )
    fm[i] = FLOAT4( *rgb, 1 );
}


void GPf416::draw( const GPf416 &blk, int x, int y)
{
  int src_w, src_h;
    src_w = blk.w;
    src_h = blk.h;

  int des_w, des_h;
    des_w = w;
    des_h = h;

  FLOAT4 *src, *des;
    src = blk.fm;
    des = &fm[y*w + x];

  int j;
  for( j=0; j<src_h; j++, src+=src_w, des+=des_w )
    memcpy( des, src, src_w * sizeof(FLOAT4) );
}


void GPf416::getchannel( float *r, float *g, float *b, float *a )
{
  int t, l;
  FLOAT4 *tm;

  l=w*h;
  tm=fm;

  for( t=0; t<l; t++, tm++, r++, g++, b++, a++ )
  {
    *r = tm->x;
    *g = tm->y;
    *b = tm->z;
    *a = tm->w;
  }
}

void GPf416::load( int width, int height, const FLOAT4 *rgba )
{
  load( width, height );
  memcpy( fm, rgba, w * h * sizeof(FLOAT4) );
}

FLOAT4 GPf416::vmax()
{
  int t, l;
  FLOAT4 *tm;
  float mx,my,mz,mw;

  l = w*h;
  tm = fm;

  mx = tm->x;
  my = tm->y;
  mz = tm->z;
  mw = tm->w;

  for( t=0; t<l; t++, tm++ )
  {
    if( tm->x > mx ) mx=tm->x;
    if( tm->y > my ) my=tm->y;
    if( tm->z > mz ) mz=tm->z;
    if( tm->w > mw ) mw=tm->w;
  }

  return FLOAT4( mx,my,mz,mw );
}

FLOAT4 GPf416::vmin()
{
  int t, l;
  FLOAT4 *tm;
  float mx,my,mz,mw;

  l = w*h;
  tm = fm;

  mx = tm->x;
  my = tm->y;
  mz = tm->z;
  mw = tm->w;

  for( t=0; t<l; t++, tm++ )
  {
    if( tm->x < mx ) mx=tm->x;
    if( tm->y < my ) my=tm->y;
    if( tm->z < mz ) mz=tm->z;
    if( tm->w < mw ) mw=tm->w;
  }

  return FLOAT4( mx,my,mz,mw );
}

FLOAT4 GPf416::vmean()
{
  int t, l;
  FLOAT4 *tm;
  FLOAT4 res;

  l = w*h;
  tm = fm;

  res = 0;
  for( t=0; t<l; t++, tm++ )
    res = res + *tm;

  return res/float(l);
}

FLOAT4 GPf416::variance()
{
  int t, l;
  FLOAT4 *tm;
  FLOAT4 mn;
  FLOAT4 var;
  FLOAT4 tmp;
    
  l = w*h;
  tm = fm;
  mn = vmean();
  var = 0;
    
  for( t=0; t<l; t++, tm++ )
  {
    tmp = *tm - mn;
    var = var + tmp*tmp;
  }

  return var/float(l);
}

void GPf416::mul( FLOAT4 a )
{
  int t, l;
  FLOAT4 *tm;

  tm = fm;
  l = w*h;
  for( t=0; t<l; t++, tm++ )
    *tm = *tm * a;
}

void GPf416::add( FLOAT4 a )
{
  int t, l;
  FLOAT4 *tm;

  tm = fm;
  l = w*h;
  for( t=0; t<l; t++, tm++ )
    *tm = *tm + a;
}
