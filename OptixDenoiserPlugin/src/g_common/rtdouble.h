#ifndef RTDOUBLE_H
#define RTDOUBLE_H

#include "RenderTexture.h"
#include "g_pfm.h"

enum RT_STATE
{
  RT_ROW,
  RT_COL,
};

void rt_emul( RenderTexture *a, RenderTexture *b, RenderTexture *c );
void rt_sub( RenderTexture *a, RenderTexture *b, RenderTexture *c );
void rt_mul( RenderTexture *a, RenderTexture *b, RenderTexture *c, RenderTexture *b_buf );
void rt_mult( RenderTexture *a, RenderTexture *b, RenderTexture *c, RenderTexture *a_buf );
void rt_save( RenderTexture *rt, GPfm &pfm );
void rt_load( RenderTexture *rt, GPfm &pfm );

void rt_colsum( RenderTexture *input_rt, int w0, int h0, RenderTexture *output_rt, int k, RT_STATE k_state );

void rt_prepare( int mw );
void prepare_rtdouble( int mw );

class RTDouble
{
  public:
    static float total_size;

    RenderTexture *front_rt;
    RenderTexture *back_rt;

    int w,h;
    RTDouble();
    ~RTDouble();

    void load( unsigned char *dat );
    void load( GPfm &pfm );
    void load( int width, int height );


    void save( GPfm &pfm );

    void draw( int x, int y );

    RenderTexture* front();

    void mul( RTDouble &a, RTDouble &b );
    void sub( RTDouble &a );
    void column_energy( RTDouble &col_energy );
    void IsResident();

    void swap();

  private:
    RenderTexture* back();
};

#endif