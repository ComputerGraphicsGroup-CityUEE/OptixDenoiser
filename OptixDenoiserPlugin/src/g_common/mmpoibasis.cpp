#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <float.h>

#include "g_common.h"
#include "mpoibasis.h"
#include "mmpoibasis.h"

float cal_n_factor( int n_center, float gx );

MMPOIBasis::MMPOIBasis( int basis_num, float GX, GParam::P_TYPE domain)
{
  if( domain!=GParam::FULL_SPHERE )
  {
    printf( "[Error] MPOIBasis, not implemented yet.\n" );
    exit(-1);
  }

  m_type = BASIS_MMPOI;
  m_domain = domain;
  n_basis = basis_num;
  gx = GX;

  switch( basis_num )
  {
    case 420:
      mpoi = new MPOIBasis( basis_num, gx );
      mpoi->lpoi[1]->eta = cal_n_factor( int(mpoi->lpoi[1]->n_basis*2*.75), gx );
      mpoi->lpoi[2]->eta = cal_n_factor( int(mpoi->lpoi[2]->n_basis*4*.75), gx );
    break;
    default:
      printf( "[Error] MPOIBasis, basis_num can only be 420.\n" );
      exit(-1);
    break;
  }
}

MMPOIBasis::~MMPOIBasis()
{
  delete mpoi;
}



void MMPOIBasis::GetSamples( float *pSample, const FLOAT3 *tL ) const 
{
  mpoi->GetSamples( pSample, tL );
}

