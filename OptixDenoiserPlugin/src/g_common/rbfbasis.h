#ifndef RBFBASIS_H
#define RBFBASIS_H

#include "g_vector.h"
#include "g_basis.h" 

class RBFBasis : public GBasis
{
  public:
  
    RBFBasis( int basis_num, GParam::P_TYPE domain = GParam::FULL_SPHERE);
    RBFBasis( int basis_num, const float **tp_centers, const float delta, GParam::P_TYPE domain = GParam::FULL_SPHERE);
    RBFBasis( int basis_num, const float *t_centers, const float *p_centers, const float delta, GParam::P_TYPE domain = GParam::FULL_SPHERE);
    RBFBasis( int basis_num, const FLOAT3 *pCenter, const float delta, GParam::P_TYPE domain = GParam::FULL_SPHERE );
    virtual ~RBFBasis();
  
    void GetSamples( float *pSample, const FLOAT3 *tL ) const ;
    void GetRotationMatrix( float *mR, const float *mRotation );
  
    FLOAT3 *m_pCenter;
    float m_delta;
  
  private:
    float rbfval( const FLOAT3 *center, float delta, const FLOAT3 *L ) const;
  
    int RBF_TABLE_GRID;
    float *RBF_IN_TABLE;
    float Interpo_In_RBFxRBF(FLOAT3 *tC0, FLOAT3 *tC1);
    void InitTable_In_RBFxRBF();
};

#endif