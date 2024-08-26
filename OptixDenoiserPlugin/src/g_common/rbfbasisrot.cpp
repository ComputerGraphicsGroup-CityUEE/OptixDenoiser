#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>
#include <float.h>

//#include "pcvalues.h"
#include "g_common.h"
#include "rbfbasis.h"
//#include "SphereHammersleyParam.h"

float RBFBasis::Interpo_In_RBFxRBF(FLOAT3 *tC0, FLOAT3 *tC1)
{
  float l = vdot( tC0, tC1 );

  if( l>=1.f )
    l=1-FLT_EPSILON;
  else if( l<=-1.f )
    l=-1+FLT_EPSILON;

  float d = (float) acos(  l  );

  float sd = (float) ((d/G_PI)*(RBF_TABLE_GRID-1));

  float s = RBF_IN_TABLE[(int)floor(sd)];
  float t = RBF_IN_TABLE[(int)ceil(sd)];
  float r = sd-floorf(sd);

  return (1-r)*s + t*r;
}

void RBFBasis::InitTable_In_RBFxRBF()
{

  printf( "Initializing RBF vs RBF Integration Table... " );

  GParam *pGParam = GetParam(DENSE);//new SphereHammersleyParam( GParam::FULL_SPHERE, 10000 );

  FLOAT3 *pC = (FLOAT3*) malloc( RBF_TABLE_GRID * sizeof(FLOAT3) );
  FLOAT3 *tC;

  float t_phi = 0.f;

  float theta_step = (float) (G_PI/(RBF_TABLE_GRID-1));
  float t_theta = 0.f;

  tC = pC;
  for( int i=0; i<RBF_TABLE_GRID; i++, t_theta+=theta_step, tC++ )
    Angle2Float3( t_theta, t_phi, tC );

  RBFBasis *B0 = new RBFBasis( 1, pC, m_delta);
  RBFBasis *B1 = new RBFBasis( RBF_TABLE_GRID, pC, m_delta);

  GBasis::GetCorelation( RBF_IN_TABLE, B0, B1, pGParam );

  SAFE_FREE( pC );
  SAFE_DELETE( B0 );
  SAFE_DELETE( B1 );

  printf( "Done\n" );
}

void RBFBasis::GetRotationMatrix( float *mR, const float *mRotation )
{
  if( RBF_IN_TABLE==NULL )
  {
    RBF_IN_TABLE = (float*) malloc( RBF_TABLE_GRID * sizeof(float) );
    InitTable_In_RBFxRBF();
  }

  float m[9];
    m[0] = mRotation[0];
    m[1] = mRotation[1];
    m[2] = mRotation[2];
    m[3] = mRotation[4];
    m[4] = mRotation[5];
    m[5] = mRotation[6];
    m[6] = mRotation[8];
    m[7] = mRotation[9];
    m[8] = mRotation[10];

  FLOAT3 *pCx = (FLOAT3*) malloc( n_basis * sizeof(FLOAT3) );

  for( int i=0; i<n_basis; i++ )
    Mul_R3xM33( m, &m_pCenter[i], &pCx[i] );
  
  FLOAT3 *tC1 = m_pCenter;
  for( int fj=0; fj<n_basis; fj++, tC1++ )
  {
    FLOAT3 *tCx = pCx;
    for( int fi=0; fi<n_basis; fi++, tCx++ )
    {
      mR[ fj*n_basis + fi ] = Interpo_In_RBFxRBF( tC1, tCx );
    }
  }

  SAFE_FREE(pCx);

}
