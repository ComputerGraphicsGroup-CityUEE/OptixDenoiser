#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>
#include <float.h>

//#include "pcvalues.h"
#include "rbfbasis.h"
#include "SphereHammersleyParam.h"
#include "rbfcal.h"


RBFBasis::RBFBasis( int basis_num, GParam::P_TYPE domain )
{
  RBF_TABLE_GRID = 1024;
  RBF_IN_TABLE = NULL;

  bOrthonormal = false;
  m_type = BASIS_RBF;
  m_domain = domain;

  n_basis = basis_num;

  rbfcal *tcal;

  if( domain == GParam::FULL_SPHERE )
    tcal = new rbfcal( basis_num, 180, 360 );
  else
    tcal = new rbfcal( basis_num, 90, 360 );


  m_delta = tcal->m_delta;
  m_pCenter = (FLOAT3*) malloc( n_basis * sizeof(FLOAT3) );
  memcpy( m_pCenter, tcal->m_centre, n_basis * sizeof(FLOAT3) );

  if( domain == GParam::HALF_SPHERE_POS_X )
  {
    /*
    FLOAT3 l1, *tC;

    tC = m_pCenter;
    for( int i=0; i<n_basis; i++, tC++ )
    {
      Mul_R3xM33( mRot, tC, &l1 );
      *tC = l1;
    }
    */
    printf( "[Error] RBFBasis, not implemented yet.\n" );
    exit(-1);
  }


  if( domain == GParam::HALF_SPHERE_POS_Z )
  {
    /*
    FLOAT3 l1, *tC;

    tC = m_pCenter;
    for( int i=0; i<n_basis; i++, tC++ )
    {
      Mul_R3xM33( mRot, tC, &l1 );
      *tC = l1;
    }
    */
    printf( "[Error] RBFBasis, not implemented yet.\n" );
    exit(-1);
  }

  delete tcal;

}


RBFBasis::RBFBasis( int basis_num, const float **tp_centers, const float delta, GParam::P_TYPE domain )
{
  RBF_TABLE_GRID = 1024;
  RBF_IN_TABLE = NULL;

  bOrthonormal = false;
  m_type = BASIS_RBF;
  m_domain = domain;

  m_delta = delta;
  n_basis = basis_num;
  m_pCenter = (FLOAT3*) malloc( n_basis * sizeof(FLOAT3) );

  for( int i=0; i<basis_num; i++ )
  {
	float theta = (float)(tp_centers[i][0]*G_PI/180);
	float phi   = (float)(tp_centers[i][1]*G_PI/180);

	Angle2Float3( theta, phi, &m_pCenter[i] );
  }
}

RBFBasis::RBFBasis( int basis_num, const float *t_centers, const float *p_centers, const float delta, GParam::P_TYPE domain)
{
  RBF_TABLE_GRID = 1024;
  RBF_IN_TABLE = NULL;
  m_domain = domain;

  bOrthonormal = false;
  m_type = BASIS_RBF;

  m_delta = delta;
  n_basis = basis_num;
  m_pCenter = (FLOAT3*) malloc( n_basis * sizeof(FLOAT3) );

  for( int i=0; i<basis_num; i++ )
  {
	float theta = (float)(t_centers[i]*G_PI/180);
	float phi   = (float)(p_centers[i]*G_PI/180);

	Angle2Float3( theta, phi, &m_pCenter[i] );
  }
}


RBFBasis::RBFBasis( int basis_num, const FLOAT3 *pCenter, const float delta, GParam::P_TYPE domain)
{

  RBF_TABLE_GRID = 1024;
  RBF_IN_TABLE = NULL;
  m_domain = domain;

  bOrthonormal = false;
  m_type = BASIS_RBF;

  n_basis = basis_num;
  m_pCenter = (FLOAT3*) malloc( basis_num * sizeof(FLOAT3) );
  memcpy( m_pCenter, pCenter, basis_num * sizeof(FLOAT3) );

  m_delta = delta;
}

RBFBasis::~RBFBasis()
{
  SAFE_FREE( m_pCenter );
  SAFE_FREE( RBF_IN_TABLE );
}

void RBFBasis::GetSamples( float *pSample, const FLOAT3 *tL )  const
{
  float *tSample = pSample;
  const FLOAT3 *tCenter = m_pCenter;
  
  for( int i=0; i<n_basis; i++, tCenter++, tSample++ )
    *tSample = rbfval( tCenter, m_delta, tL );
}

float RBFBasis::rbfval( const FLOAT3 *center, float delta, const FLOAT3 *L ) const
{
  float l = vdot( center, L );
  l = G_CLAMP( l, -1+FLT_EPSILON, 1-FLT_EPSILON );

  float d = (float) acos(  l  );
  float p = d/delta;
  
  float res = (float) exp( -p*p/2 );
  
  return res;
}
