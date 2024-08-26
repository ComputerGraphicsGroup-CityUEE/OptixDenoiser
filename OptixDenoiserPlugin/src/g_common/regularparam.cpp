#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

//#include "pcvalues.h"
#include "regularparam.h"
/*
RegularParam::RegularParam( 
	float theta_start, float theta_end, int THETA_GRID,	
	float phi_start, float phi_end, int PHI_GRID
){
  m_theta_start = theta_start;
  m_theta_end = theta_end;
  m_THETA_GRID = THETA_GRID;
  
  m_phi_start = phi_start;
  m_phi_end = phi_end;
  m_PHI_GRID = PHI_GRID;
  
  n_sample = THETA_GRID*PHI_GRID;
  m_pL = NULL;
  
  m_type = CUSTOM;
}
*/
RegularParam::RegularParam( P_TYPE T, int THETA_GRID, int PHI_GRID )
{
  m_type = T;
  m_theta_start = 0;
 
  if( T==FULL_SPHERE )
    m_theta_end = (float)G_PI;
  else
    m_theta_end = (float) (G_PI/2);
  
  m_THETA_GRID = THETA_GRID;
  
  m_phi_start = 0;
  m_phi_end = (float) (2*G_PI);
  m_PHI_GRID = PHI_GRID;
  
  n_sample = THETA_GRID*PHI_GRID;
  m_pL = NULL;
}

RegularParam::~RegularParam()
{
  if(m_pL!=NULL)
  {
    free(m_pL);
    m_pL=NULL;
  }
}

FLOAT4* RegularParam::GetSamplesPt()
{
  if( m_pL!=NULL ) return m_pL;
  
  m_pL = (FLOAT4*) malloc( n_sample * sizeof(FLOAT4) );
  
  float theta_range = m_theta_end-m_theta_start;
  float phi_range = m_phi_end-m_phi_start;
  
  
  float theta_step = (float) ( theta_range / (m_THETA_GRID+1) );
  float phi_step = (float) ( phi_range / (m_PHI_GRID+1) );
  
  
  FLOAT4 *tL = m_pL;
  
  float t_theta = m_theta_start + theta_step;
  for( int i=0; i<m_THETA_GRID; i++, t_theta+=theta_step )
  {
    float t_phi = m_phi_start + phi_step;
    for( int j=0; j<m_PHI_GRID; j++, t_phi+=phi_step )
    {
      Angle2Float3( t_theta, t_phi, (FLOAT3*)tL );
      tL->w = (float) (sin( t_theta )*theta_step*phi_step);
      tL++;
    }
  }
  
  return m_pL;
}
