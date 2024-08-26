#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>
#include <float.h>

#include "healpixparam.h"

#include "chealpixcpp.h"

HealpixParam::HealpixParam( P_TYPE T, int nside )
{
  float fidx = (float) ( log(nside)/log(2) );
  int iidx = (int) ( floor(fidx+.5) );
  
  if( fidx-iidx!=0 )
  {
    printf( "Error : NSide of Healpix must be power of two.\n" );
    exit(0);
  }
  
  m_nside = nside;
  
  m_pL = NULL;
  
  m_type = T;
  
  if( m_type==FULL_SPHERE )
    n_sample = 12*m_nside*m_nside;
  if( m_type==HALF_SPHERE_POS_Y )
    n_sample = 6*m_nside*m_nside;

}

HealpixParam::~HealpixParam()
{
  if(m_pL!=NULL)
  {
    free(m_pL);
    m_pL=NULL;
  }
}

FLOAT4* HealpixParam::GetSamplesPt()
{
  if( m_pL!=NULL ) return m_pL;
  
  double theta, phi;
  float l_w;
  FLOAT4 *tL;
  int ipix;
  chealpixcpp HPix;
  l_w = (float) (4 * M_PI / (12*m_nside*m_nside) );
  
  int pL_count=0;
  
  switch( m_type )
  {
    case FULL_SPHERE:
      n_sample = 12*m_nside*m_nside;
    
      m_pL = (FLOAT4*) malloc( n_sample * sizeof(FLOAT4) );
    
      tL = m_pL;
      for( ipix=0; ipix<12*m_nside*m_nside; ipix++, tL++ )
      {
        HPix.pix2ang_nest(m_nside, ipix, &theta, &phi);
        Angle2Float3( (float)theta, (float)phi, (FLOAT3*) tL );
        tL->w = l_w;
      }
      break;
    case HALF_SPHERE_POS_Y:
      n_sample = 6*m_nside*m_nside;
      m_pL = (FLOAT4*) malloc( n_sample * sizeof(FLOAT4) );
    
      tL = m_pL;
      for( ipix=0; ipix<12*m_nside*m_nside; ipix++ )
      {
        HPix.pix2ang_nest(m_nside, ipix, &theta, &phi);
        Angle2Float3( (float)theta, (float)phi, (FLOAT3*) tL );
        tL->w = l_w;
      
        if( acos(tL->y) < M_PI/2 )
        {
          tL++;
          pL_count++;
        }
      }
      n_sample = pL_count;
    
      break;
  }
  return m_pL;
}
