#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <float.h>

#include "g_common.h"
#include "mpoibasis.h"
#include "poibasis.h"

float cal_n_factor( int n_center, float gx );

MPOIBasis::MPOIBasis( int basis_num, float GX, GParam::P_TYPE domain)
{
  if( domain!=GParam::FULL_SPHERE )
  {
    printf( "[Error] MPOIBasis, not implemented yet.\n" );
    exit(-1);
  }

  m_type = BASIS_MPOI;
  m_domain = domain;
  n_basis = basis_num;
  gx = GX;

  switch( basis_num )
  {
    case 20:
      n_level=1;
      lpoi = (POIBasis**) malloc( n_level * sizeof(POIBasis*) );
      lpoi[0] = new POIBasis(20, gx);
    break;
    case 100:
      n_level=2;
      lpoi = (POIBasis**) malloc( n_level * sizeof(POIBasis*) );
      lpoi[0] = new POIBasis(20, gx);
      lpoi[1] = new POIBasis(80, gx);
    break;
    case 420:
      n_level=3;
      lpoi = (POIBasis**) malloc( n_level * sizeof(POIBasis*) );
      lpoi[0] = new POIBasis(20, gx);
      lpoi[1] = new POIBasis(80, gx);
      lpoi[2] = new POIBasis(320, gx);
    break;
    case 1700:
      n_level=4;
      lpoi = (POIBasis**) malloc( n_level * sizeof(POIBasis*) );
      lpoi[0] = new POIBasis(20, gx);
      lpoi[1] = new POIBasis(80, gx);
      lpoi[2] = new POIBasis(320, gx);
      lpoi[3] = new POIBasis(1280, gx);
    break;
    case 6820:
      n_level=5;
      lpoi = (POIBasis**) malloc( n_level * sizeof(POIBasis*) );
      lpoi[0] = new POIBasis(20, gx);
      lpoi[1] = new POIBasis(80, gx);
      lpoi[2] = new POIBasis(320, gx);
      lpoi[3] = new POIBasis(1280, gx);
      lpoi[4] = new POIBasis(5120, gx);
    break;
    default:
      printf( "[Error] MPOIBasis, basis_num can only be 20, 100, 420, 1700, 6820.\n" );
      exit(-1);
    break;
  }
}

MPOIBasis::~MPOIBasis()
{
  int i;
  for( i=0; i<n_level; i++ )
    delete lpoi[i];
  free(lpoi);
}



void MPOIBasis::GetSamples( float *pSample, const FLOAT3 *tL ) const 
{
  int i,m,n;
  for( i=0, m=0; i<n_level; i++ )
  {
    n = lpoi[i]->n_basis;
    
    lpoi[i]->GetSamples( &pSample[m], tL );
    
    m+=n;
  }
}

