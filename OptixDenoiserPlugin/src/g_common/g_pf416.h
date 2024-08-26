#ifndef G_PF416_H
#define G_PF416_H

#include "g_common.h"
#include "g_vector.h"

class GPf416
{
  public:
    int w,h;
    FLOAT4 *fm;
    FLOAT4 **pm;
    GPf416();
    ~GPf416();
    void flip_vertical();
    void load( int width, int height, const float *r, const float *g, const float *b, const float *a );
    void load( int width, int height, const FLOAT4 *rgba );
    void load( int width, int height, const FLOAT3 *rgb );
    void load( int width, int height );

    void load16( int width, int height );

    void load( const char *spath );
    void save( const char *spath );


    void getchannel( float *r, float *g, float *b, float *a );

    FLOAT4 vmax();
    FLOAT4 vmin();
    FLOAT4 vmean();
    FLOAT4 variance();
    void mul( FLOAT4 a );
    void add( FLOAT4 a );

    void draw( const GPf416 &blk, int x, int y);
    //void load( GPfm &pfm );

    bool byte16_align;
};

#endif