#ifndef MRBFBASIS_H
#define MRBFBASIS_H

#include "g_vector.h"
#include "g_basis.h" 

class MRBFBasis : public GBasis
{
  public:
  
    MRBFBasis( int basis_num, GParam::P_TYPE domain = GParam::FULL_SPHERE);
    virtual ~MRBFBasis();
  
    void SetRot( const float *mRotation );
    void GetRotatedCoef( const float *c0, float *c1 );
    void GetSamples( float *pSample, const FLOAT3 *tL ) const ;
  
    int n_level;
    GBasis **lrbf;
  
};

#endif