#ifndef SHBASIS2_H
#define SHBASIS2_H

#include "g_vector.h"
#include "g_basis.h" 

class SHBasis2 : public GBasis
{
  public:

    SHBasis2( int basis_num, GParam::P_TYPE domain = GParam::FULL_SPHERE );
    void GetSamples( float *pSample, const FLOAT3 *tL ) const;


    //void SetRot( const float *mRotation );
    //void GetRotatedCoef( const float *c0, float *c1 );
  //float ea, eb, eg;
};

#endif
