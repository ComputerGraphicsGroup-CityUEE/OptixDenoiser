#ifndef POI_BASIS_H
#define POI_BASIS_H

#include "g_vector.h"
#include "g_basis.h" 

class POIBasis : public GBasis
{
  public:
  
    POIBasis( int basis_num, float GX, GParam::P_TYPE domain = GParam::FULL_SPHERE);
    POIBasis( POIBasis *p0, POIBasis *p1 );
    virtual ~POIBasis();
  
    void GetSamples( float *pSample, const FLOAT3 *tL ) const ;
    void GetRotationMatrix( float *mR, const float *mRotation );

    FLOAT3 *center;
    float eta;
    float gx;

    static float fpoi0( float x, float eta );


  private:
  
};

#endif