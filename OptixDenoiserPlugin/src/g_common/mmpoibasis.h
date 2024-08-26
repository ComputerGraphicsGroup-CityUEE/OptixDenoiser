#ifndef MMPOIBASIS_H
#define MMPOIBASIS_H

#include "g_vector.h"
#include "mpoibasis.h" 

class MMPOIBasis : public GBasis
{
  public:
  
    MMPOIBasis( int basis_num, float GX, GParam::P_TYPE domain = GParam::FULL_SPHERE);
    virtual ~MMPOIBasis();
  
    void GetSamples( float *pSample, const FLOAT3 *tL ) const ;
  
    MPOIBasis *mpoi;
    float gx;
  
};

#endif