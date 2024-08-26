#ifndef G_DCT_H
#define G_DCT_H

#include "g_pfm.h"

enum DCT_STATUS
{
  FORWARD_DCT,
  INVSESE_DCT
};

class GDct
{
  public:
    GDct();
    GDct( int dimension );
    ~GDct();
    void set( int dimension );

    void forward( const float *source, float *destination );
    void inverse( const float *source, float *destination );

    void forward( const GPfm &pfm0, GPfm &pfm1 );
    void inverse( const GPfm &pfm0, GPfm &pfm1 );
    
  private:


    void gdct( const float *source, float *destination, DCT_STATUS status );
    void gdct( const GPfm &pfm0, GPfm &pfm1, DCT_STATUS status );
    int dim;
    float **mc;
    float **mct;
    float **tmp;
    float **src;
    float **des;

};

#endif

