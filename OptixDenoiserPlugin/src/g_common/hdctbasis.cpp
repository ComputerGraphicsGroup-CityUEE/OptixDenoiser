#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "g_vector.h"

#include "chealpixcpp.h"
#include "hpp_xyp.h"

#include "HDCTBasis.h"
#include "healpixparam.h"

HDCTBasis::HDCTBasis( int basis_num, int n_side )
{
  if( basis_num%12 != 0 )
  {
    printf( "[Error] HDCTBasis, number of basis must be divisable by 12\n" );
    exit(-1);
  }

  bOrthonormal = true;
  m_type = BASIS_HDCT;
  
  n_basis = basis_num;
  N_DCT = basis_num/12;
  N_SIDE = n_side;


  ZZ = (int*) malloc( N_SIDE*N_SIDE*sizeof(int) );
  gen_zmap( ZZ, N_SIDE );

  N0 = sqrtf(1.0f);
  N1 = sqrtf(2.0f);

}

GParam* HDCTBasis::GetParam( int param_density )
{
  switch( param_density )
  {
    case COARSE:
      if( param_coarse==NULL )
        param_coarse = new HealpixParam( GParam::FULL_SPHERE, 8 );
      return param_coarse;

    case DENSE:
      if( param_dense==NULL )
        param_dense = new HealpixParam( GParam::FULL_SPHERE, 32 );
      return param_dense;

    default:
      printf( "[Error] : (), unknown parameter density type.\n" );
      exit(-1);
  }
}

HDCTBasis::~HDCTBasis()
{
  SAFE_FREE( ZZ );
}

void HDCTBasis::GetSamples( float *pSample, const FLOAT3 *tL ) const
{
       
  memset( pSample, 0, n_basis*sizeof(float) );

  float theta, phi;
  Float32Angle( &theta, &phi, tL );

  const int fine_nside = 8192;

  long ipix;
  chealpixcpp hpix;
  hpix.ang2pix_nest( fine_nside, theta, phi, &ipix );

  int face_num = ipix / (fine_nside*fine_nside) ;

  unsigned int tmp_pix = ipix % (fine_nside*fine_nside) ;
  unsigned int x_pix, y_pix;

  hpp_xyp hxyp;
  hxyp.hp2xy( tmp_pix, &x_pix, &y_pix );

  float x,y;
    x = x_pix/(float)fine_nside;
    y = y_pix/(float)fine_nside;

  float *tSample = &pSample[face_num*N_DCT];

  for( int zig_t=0; zig_t<N_DCT; zig_t++, tSample++ )
  {
    int zx = ZZ[zig_t]%N_SIDE;
    int zy = ZZ[zig_t]/N_SIDE;


    float tx = zx*((float)M_PI) * x;
    float ty = zy*((float)M_PI) * y;

    float dctx;
    if( zx==0 )
      dctx = N0;
    else
      dctx = N1*cosf(tx);

    float dcty;
    if( zy==0 )
      dcty = N0;
    else
      dcty = N1*cosf(ty);

    *tSample = dctx * dcty;
  }
}

float HDCTBasis::mean( const float *c0 )
{
  float mean_val = 0;

  for( int face_t=0; face_t<12; face_t++ )
    mean_val += c0[ face_t*N_DCT ];

  return mean_val;
}

float HDCTBasis::hp_val( const float *c0, const int ipix )
{
  float res = 0;

  int n2 = N_SIDE*N_SIDE;
  int face_num = ipix / n2;
  unsigned int xy_pix = ipix % n2;

  unsigned int x_pix, y_pix;
    hpp_xyp hxyp;
    hxyp.hp2xy( xy_pix, &x_pix, &y_pix );

  float x,y;
    x = (x_pix+.5f)/N_SIDE;
    y = (y_pix+.5f)/N_SIDE;

  const float *t_c0 = &c0[face_num*N_DCT];

  for( int zig_t=0; zig_t<N_DCT; zig_t++, t_c0++ )
  {
    int zx = ZZ[zig_t]%N_SIDE;
    int zy = ZZ[zig_t]/N_SIDE;

    float tx = zx*((float)M_PI) * x;
    float ty = zy*((float)M_PI) * y;

    float dctx;
    if( zx==0 )
      dctx = N0;
    else
      dctx = N1*cosf(tx);

    float dcty;
    if( zy==0 )
      dcty = N0;
    else
      dcty = N1*cosf(ty);

    res += *t_c0 * dctx * dcty;
  }

  return res;
}


float HDCTBasis::hp_val( const float *c0, int face_num, float x, float y )
{
  float res = 0;

  const float *t_c0 = &c0[face_num*N_DCT];

  for( int zig_t=0; zig_t<N_DCT; zig_t++, t_c0++ )
  {
    int zx = ZZ[zig_t]%N_SIDE;
    int zy = ZZ[zig_t]/N_SIDE;

    float tx = zx*((float)M_PI) * x;
    float ty = zy*((float)M_PI) * y;

    float dctx;
    if( zx==0 )
      dctx = N0;
    else
      dctx = N1*cosf(tx);

    float dcty;
    if( zy==0 )
      dcty = N0;
    else
      dcty = N1*cosf(ty);

    res += *t_c0 * dctx * dcty;
  }

  return res;
}
