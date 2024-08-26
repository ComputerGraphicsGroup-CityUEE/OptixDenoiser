#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <float.h>

#include "rbfcal.h"

#include "g_common.h"
#include "mrbfbasis.h"
#include "rbfbasis.h"

MRBFBasis::MRBFBasis( int basis_num, GParam::P_TYPE domain)
{
  m_type = BASIS_MRBF;
  m_domain = domain;
  n_basis = basis_num;

  n_level = ROUND( log(3.0*basis_num/4+1)/log(4) );

  if( ROUND(4*(pow(4,n_level)-1)/3) != basis_num )
  {
    printf( "[Error] MRBFBasis::MRBFBasis, incorrect basis_num, must be sum(4^n)." );
    exit(-1);
  }

  lrbf = (GBasis**) malloc( n_level * sizeof(GBasis*) );

  int i, n;

  switch( domain )
  {
    case GParam::FULL_SPHERE:
      for( i=0; i<n_level; i++ )
      {
        n = ROUND( 4*pow(4,i) );
        RBFBasis *a = new RBFBasis(n);
        lrbf[i] = a;
      }

    break;

    case GParam::HALF_SPHERE_POS_Y:
    case GParam::HALF_SPHERE_POS_Z:
    case GParam::HALF_SPHERE_POS_X:
    default:
      printf( "[Error] RBFBasis, not implemented yet.\n" );
      exit(-1);
  }

}

MRBFBasis::~MRBFBasis()
{
  int i;
  for( i=0; i<n_level; i++ )
    delete lrbf[i];
  free(lrbf);
}



void MRBFBasis::GetSamples( float *pSample, const FLOAT3 *tL ) const 
{
  int i,m,n;
  for( i=0, m=0; i<n_level; i++ )
  {
    n = ROUND( 4*pow(4,i) );
    
    lrbf[i]->GetSamples( &pSample[m], tL );
    
    m+=n;
  }
}

void MRBFBasis::SetRot( const float *mRotation )
{
  int i;
  for( i=0; i<n_level; i++ )
    lrbf[i]->SetRot( mRotation );
}

void MRBFBasis::GetRotatedCoef( const float *c0, float *c1 )
{
  int i,m,n;
  for( i=0, m=0; i<n_level; i++ )
  {
    n = ROUND( 4*pow(4,i) );
    
    lrbf[i]->GetRotatedCoef( &c0[m], &c1[m] );
    
    m+=n;
  }
}
