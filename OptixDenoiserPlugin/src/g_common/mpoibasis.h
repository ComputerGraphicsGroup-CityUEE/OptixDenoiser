#ifndef MPOIBASIS_H
#define MPOIBASIS_H

#include "g_vector.h"
#include "poibasis.h" 

class MPOIBasis : public GBasis
{
  public:
  
    MPOIBasis( int basis_num, float GX, GParam::P_TYPE domain = GParam::FULL_SPHERE);
    virtual ~MPOIBasis();
  
    void GetSamples( float *pSample, const FLOAT3 *tL ) const ;
  
    int n_level;
    POIBasis **lpoi;
    float gx;
  
};

#endif