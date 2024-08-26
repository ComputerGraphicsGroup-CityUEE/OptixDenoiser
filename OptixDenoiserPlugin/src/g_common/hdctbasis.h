#ifndef HDCTBASIS_H
#define HDCTBASIS_H

#include "g_vector.h"
#include "g_basis.h" 


class HDCTBasis : public GBasis
{
  public:

    HDCTBasis( int basis_num, int n_side );
    void GetSamples( float *pSample, const FLOAT3 *tL ) const;
    float hp_val( const float *c0, const int ipix );
    float hp_val( const float *c0, int face_num, float x, float y );

    GParam* GetParam( int param_density );

    ~HDCTBasis();
    float mean( const float *c0 );

  private:
    float N0;
    float N1;
    int N_DCT;
    int N_SIDE;

    int *ZZ;

};

#endif
