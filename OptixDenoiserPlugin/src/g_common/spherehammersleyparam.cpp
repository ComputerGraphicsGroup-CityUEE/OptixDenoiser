#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

//#include "pcvalues.h"
#include "spherehammersleyparam.h"

void SphereHammersleyParam::Get_HemiSphereHammersley(FLOAT4 *pL, int n_point)
{

  n_point = 2 * n_point;

  int     k, kk;
  double  p, phi;

  double cost, sint;
  double cosp, sinp;

  FLOAT4 *tL = pL;
  float l_w = (float) (4 * G_PI / n_point );

  for( k=0; k<n_point/2; k++, tL++ )
  {
    cost = 0;
    for( kk = k, p = 0.5; kk; p *= 0.5, kk >>= 1 )
      if( kk & 1 )  // kk mod 2 == 1
        cost += p;

    // a slight shift and then map to [0, 2 pi)
    phi = 2.0*G_PI * ( (k+0.5)/n_point );
    
    // map from [0,1] to [-1,1]
    cost = 2.0 * cost - 1.0;

    sint = sqrt(1.0 - cost * cost);
    sinp = sin(phi);
    cosp = cos(phi);
    
    tL->x = (float)(  sint * cosp );
    tL->y = (float)(  cost        );
    tL->z = (float)( -sint * sinp );
    tL->w = l_w;
  }
}

void SphereHammersleyParam::Get_SphereHammersley(FLOAT4 *pL, int n_point )
{
  int     k, kk;
  double  p, phi;

  double cost, sint;
  double cosp, sinp;

  FLOAT4 *tL = pL;
  float l_w = (float) (4 * G_PI / n_point );

  for( k=0; k<n_point; k++, tL++ )
  {
    cost = 0;
    for( kk = k, p = 0.5; kk; p *= 0.5, kk >>= 1 )
      if( kk & 1 )  // kk mod 2 == 1
        cost += p;

    // a slight shift and then map to [0, 2 pi)
    phi = 2.0*G_PI * ( (k+0.5)/n_point );
    
    // map from [0,1] to [-1,1]
    cost = 2.0 * cost - 1.0;

    sint = sqrt(1.0 - cost * cost);
    sinp = sin(phi);
    cosp = cos(phi);
    
    tL->x = (float)(  sint * cosp );
    tL->y = (float)(  cost        );
    tL->z = (float)( -sint * sinp );
    tL->w = l_w;
  }
}

SphereHammersleyParam::SphereHammersleyParam( P_TYPE T, int SAMPLE_NUM )
{
  m_type = T;
  n_sample = SAMPLE_NUM;
  m_pL = NULL;
}

SphereHammersleyParam::~SphereHammersleyParam()
{
  if( m_pL!=NULL )
  {
    free(m_pL);
    m_pL=NULL;
  }
}

FLOAT4* SphereHammersleyParam::GetSamplesPt()
{
  if( m_pL!=NULL ) return m_pL;
  
  m_pL = (FLOAT4*) malloc( n_sample * sizeof(FLOAT4) );
  
  switch( m_type )
  {
	case FULL_SPHERE:
      Get_SphereHammersley( m_pL, n_sample );
      break;

    case HALF_SPHERE_POS_X:
      Get_HemiSphereHammersley( m_pL, n_sample );
      rotate( (float*)ROTATE_Y_AXIS_90 );
      break;

    case HALF_SPHERE_POS_Y:
      Get_HemiSphereHammersley( m_pL, n_sample );
      rotate( (float*)ROTATE_X_AXIS_90 );
      break;

    case HALF_SPHERE_POS_Z:
      Get_HemiSphereHammersley( m_pL, n_sample );
      rotate( (float*)ROTATE_X_AXIS_180 );
      break;

    default:
      printf( "[Error] : SphereHammersleyParam::GetSamplesPt(), unsupported region type.\n" );
      exit(-1);
      break;
  }
  return m_pL;
}
