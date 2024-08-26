#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "g_common.h"

#include "g_dct.h"


void GDct::forward( const float *source, float *destination )
{
  gdct( source, destination, FORWARD_DCT );
}

void GDct::inverse( const float *source, float *destination )
{
  gdct( source, destination, INVSESE_DCT );
}

void GDct::forward( const GPfm &pfm0, GPfm &pfm1 )
{
  gdct( pfm0, pfm1, FORWARD_DCT );
}

void GDct::inverse( const GPfm &pfm0, GPfm &pfm1 )
{
  gdct( pfm0, pfm1, INVSESE_DCT );
}




GDct::GDct()
{
  memset( this, 0, sizeof(GDct) );
}

GDct::GDct( int dimension )
{
  memset( this, 0, sizeof(GDct) );
  set( dimension );
}

GDct::~GDct()
{
  dim = 0;
  SAFE_FREE( mc );
  SAFE_FREE( mct );
  SAFE_FREE( tmp );
  SAFE_FREE( src );
  SAFE_FREE( des );
}


void GDct::set( int dimension )
{
  if( dim!=dimension )
  {
    int i,j;

    dim = dimension;
    SAFE_FREE( mc );
    SAFE_FREE( mct );
    SAFE_FREE( tmp );
    SAFE_FREE( src );
    SAFE_FREE( des );

    if( dim == 0 )
      return;

    mc = (float**) malloc( dim*sizeof(float*) + dim*dim*sizeof(float) );
    for( j=0; j<dim; j++ )
      mc[j] = ((float*)(mc+dim))+j*dim;

    mct = (float**) malloc( dim*sizeof(float*) + dim*dim*sizeof(float) );
    for( j=0; j<dim; j++ )
      mct[j] = ((float*)(mct+dim))+j*dim;

    tmp = (float**) malloc( dim*sizeof(float*) + dim*dim*sizeof(float) );
    for( j=0; j<dim; j++ )
      tmp[j] = ((float*)(tmp+dim))+j*dim;

    src = (float**) malloc( dim*sizeof(float*) );
    des = (float**) malloc( dim*sizeof(float*) );

    for( i=0; i<dim; i++ )
      mc[0][i]	= 1.0f / sqrtf((float)dim);
    for( j=1; j<dim; j++ )
      for( i=0; i<dim; i++ )
        mc[j][i]  = sqrtf(2.0f / dim) * cosf( G_PI *(2*i+1)*j / (2.0f*dim));

    for( j=0; j<dim; j++ )
      for( i=0; i<dim; i++ )
        mct[i][j] = mc[j][i];
  }
}

void GDct::gdct( const float *source, float *destination, DCT_STATUS status )
{

  int i,j,k;
  float a;
  float *v0, *v1;
  float **m;



  for( j=0; j<dim; j++ )
    src[j] = (float*)source+j*dim;
  for( j=0; j<dim; j++ )
    des[j] = destination+j*dim;


  switch( status )
  {
    case FORWARD_DCT:
      // des = mc * src * mc'
      m = mc;
    break;

    case INVSESE_DCT:
      // des = mct * src * mct'
      m = mct;
    break;
  }


  // des = m * src * m'
  // 
  // des = m * ( (src * m')' )'
  // tmp = (src * m')'
  // des = m * tmp'
  //

  // tmp = (src * m')'
  for( j=0; j<dim; j++ ) 
    for( i=0; i<dim; i++ ) 
    {
      for( k=0, a=0.0, v0=src[j], v1=m[i]; k<dim; k++, v0++, v1++ )
        a += *v0 * *v1;
      tmp[i][j] = a;
    }

  // des = m * tmp'
  for( j=0; j<dim; j++ ) 
    for( i=0; i<dim; i++ ) 
    {
      for( k=0, a=0.0, v0=m[j], v1=tmp[i]; k<dim; k++, v0++, v1++ )
        a += *v0 * *v1;
      des[j][i] = a;
    }
}

void GDct::gdct( const GPfm &pfm0, GPfm &pfm1, DCT_STATUS status )
{
  int i,j, w,h, bw,bh;
  float *AB;
  float *Ar, *Ag, *Ab;
  float *Br, *Bg, *Bb;
  GPfm pfmblk;

  w = pfm0.w;
  h = pfm0.h;
  bw = w/dim;
  bh = h/dim;

  pfm1.load( w, h );
    AB = (float*) malloc( 6 * dim*dim * sizeof(float) );
    Ar = &AB[ 0 * dim*dim ];
    Ag = &AB[ 1 * dim*dim ];
    Ab = &AB[ 2 * dim*dim ];
    Br = &AB[ 3 * dim*dim ];
    Bg = &AB[ 4 * dim*dim ];
    Bb = &AB[ 5 * dim*dim ];

  for( j=0; j<bh; j++ )
    for( i=0; i<bw; i++ )
    {
      pfm0.getblk( pfmblk, i*dim, j*dim, dim, dim );
      pfmblk.getchannel( Ar, Ag, Ab );

        this->gdct( Ar, Br, status );
        this->gdct( Ag, Bg, status );
        this->gdct( Ab, Bb, status );

      pfmblk.load( dim, dim, Br, Bg, Bb );
      pfm1.draw( pfmblk, i*dim, j*dim );
    }

  free( AB );
}
