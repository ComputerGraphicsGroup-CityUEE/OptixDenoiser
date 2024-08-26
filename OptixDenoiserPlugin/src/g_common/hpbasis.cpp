#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "chealpixcpp.h"
#include "hpp_xyp.h"

#include "healpixparam.h"


#include "hpbasis.h"



HPBasis::HPBasis( int basis_num )
{
  bOrthonormal = true; 
  n_basis = basis_num;
  m_type = BASIS_HP;
  N_SIDE = (int) sqrt(n_basis/12);

  hp_param = NULL;
}


void HPBasis::GetSamples( float *pSample, const FLOAT3 *tL ) const
{
  memset( pSample, 0, n_basis*sizeof(float) );

  float theta, phi;
  Float32Angle( &theta, &phi, tL );

  long ipix;
  chealpixcpp hpix;
  hpix.ang2pix_nest( N_SIDE, theta, phi, &ipix );

  pSample[ipix]=1.f;


}

void HPBasis::SetRot( const float *mRotation )
{
    m[0] = mRotation[0];
    m[1] = mRotation[1];
    m[2] = mRotation[2];
    m[3] = mRotation[4];
    m[4] = mRotation[5];
    m[5] = mRotation[6];
    m[6] = mRotation[8];
    m[7] = mRotation[9];
    m[8] = mRotation[10];
}

void HPBasis::GetRotatedCoef( const float *c0, float *c1 )
{

  chealpixcpp hpix;
  hpp_xyp hxyp;
  
  float theta, phi;

  FLOAT3 l1;
  long ipix;

  if( hp_param==NULL )
  {
    hp_param = new HealpixParam( GParam::FULL_SPHERE, N_SIDE );
  }

  const FLOAT4 *tL = hp_param->GetSamplesPt();

  for( int i=0; i<n_basis; i++, tL++ )
  {
    Mul_M33xC3( m, (FLOAT3*)tL, &l1 );

    Float32Angle( &theta, &phi, &l1 );

    const int fine_nside = 8192;

    hpix.ang2pix_nest( fine_nside, theta, phi, &ipix );

    int face_num = ipix / (fine_nside*fine_nside) ;
    unsigned int tmp_pix = ipix % (fine_nside*fine_nside) ;
    unsigned int x_pix, y_pix;

    hxyp.hp2xy( tmp_pix, &x_pix, &y_pix );

    float x,y;
      x = (x_pix/(float)fine_nside)*(N_SIDE)-.5f;
      y = (y_pix/(float)fine_nside)*(N_SIDE)-.5f;

      x=G_CLAMP(x,0,N_SIDE-1);
      y=G_CLAMP(y,0,N_SIDE-1);


    unsigned int ipix0, ipix1, ipix2, ipix3;

    unsigned int x0, y0;
      x0 = (int)floor(x);
      y0 = (int)floor(y);

    hxyp.xy2hp( x0,   y0, &ipix0 );
    hxyp.xy2hp( x0+1, y0, &ipix1 );
    ipix0 += face_num*N_SIDE*N_SIDE;
    ipix1 += face_num*N_SIDE*N_SIDE;

    hxyp.xy2hp( x0,   y0+1, &ipix2 );
    hxyp.xy2hp( x0+1, y0+1, &ipix3 );
    ipix2 += face_num*N_SIDE*N_SIDE;
    ipix3 += face_num*N_SIDE*N_SIDE;



  float rx = x-x0;
  float ry = y-y0;

    float cx0 = (1-rx)*c0[ipix0] + rx*c0[ipix1];
    float cx1 = (1-rx)*c0[ipix2] + rx*c0[ipix3];
    c1[i] = (1-ry)*cx0 + ry*cx1;

  }
}

HPBasis::~HPBasis()
{
  SAFE_DELETE( hp_param );
}
