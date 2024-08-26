#ifndef REGULARPARAM_H
#define REGULARPARAM_H

#include "g_common.h"
#include "g_param.h" 

class RegularParam : public GParam
{
  public:
  
    float m_theta_start;
    float m_theta_end;
    int m_THETA_GRID;
  
    float m_phi_start;
    float m_phi_end;
    int m_PHI_GRID;
/*  
    RegularParam( 
      float theta_start, float theta_end, int THETA_GRID,	
      float phi_start, float phi_end, int PHI_GRID
      );
*/  
    RegularParam( P_TYPE T, int THETA_GRID, int PHI_GRID);
  
    ~RegularParam();
  
    FLOAT4* GetSamplesPt();
  
  private:
    FLOAT4 *m_pL;

};

#endif