#ifndef HPBAISS_H
#define HPBAISS_H

#include "g_common.h"
#include "g_basis.h" 


class HPBasis : public GBasis
{
  public:

    HPBasis( int basis_num );
    void GetSamples( float *pSample, const FLOAT3 *tL ) const;

    void SetRot( const float *mRotation );
    void GetRotatedCoef( const float *c0, float *c1 );

    ~HPBasis();

  private:
    float m[9];
    int N_SIDE;
    GParam *hp_param;
};

#endif
