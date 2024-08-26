#ifndef SPOIBASIS_H
#define SPOIBASIS_H

#include "g_vector.h"
#include "g_mrx.h"
#include "poibasis.h"

class SPOIBasis : public GBasis
{
  public:
  
    SPOIBasis( int basis_num, GParam::P_TYPE domain = GParam::FULL_SPHERE);
    virtual ~SPOIBasis();
  
    void GetSamples( float *pSample, const FLOAT3 *tL ) const ;
    void GetRotationMatrix( float *mR, const float *mRotation );
  
    int n_level;
    POIBasis **lpoi;

    GMrx nEt[16];
  
};

#endif