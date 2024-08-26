#ifndef G_BASIS_H
#define G_BASIS_H

#include "g_vector.h"
#include "g_mrx.h"
#include "g_param.h"

enum BASIS_TYPE
{
  BASIS_NULL,
  BASIS_RBF,
  BASIS_SH,
  BASIS_HDCT,
  BASIS_HP,
  BASIS_RIDER,
  BASIS_HDCT2,
  BASIS_MRBF,
  BASIS_POI,
  BASIS_MPOI,
  BASIS_SPOI,
  BASIS_MMPOI,
};

#ifdef DEFINE_BASIS_STR
const char BASIS_STR[][16] = 
{
  "NULL",
  "RBF",
  "SH",
  "HDCT",
  "HP",
  "RIDER",
  "HDCT2",
  "MRBF",
  "POI",
  "MPOI",
  "SPOI",
  "MMPOI",
};
#endif

enum PARAM_DENSITY
{
  COARSE,
  DENSE,
  DENSER,
};

BASIS_TYPE StrBasis( const char *basis_tag );

class GBasis
{
  public:
    int n_basis;

    bool bOrthonormal;
    BASIS_TYPE m_type;

    GParam::P_TYPE m_domain;


    virtual char* BasisStr();

    virtual void SetRot( const float *mRotation );
    virtual void GetRotatedCoef( const float *c0, float *c1 );

    virtual void set_map2_basis( GBasis *b1 );
    virtual void map2_basis( const float *c0, float *c1 );
    
    virtual float mean( const float *c0 );
    virtual float variance( const float *c0 );

    GBasis();
    virtual ~GBasis();
    
    virtual void GetSamples( float *pSample, const FLOAT3 *tL ) const =0;

    virtual float val( const float *c0, const FLOAT3 *v );

    static void GetCorTrace(
      float *trace, 
      const GBasis *basis0, const GBasis *basis1, 
      GParam *param0
    );
    static void GetCorelation(
      float *pCorel, 
      const GBasis *basis0, const GBasis *basis1, 
      GParam *param0
    );
    static void GetCorelation2(
      float *pCorel, 
      GBasis *basis0, GBasis *basis1, 
      GParam *param0, float *mRot
    );

    virtual void GetRotationMatrix( float *mR, const float *mRotation );
    virtual GParam* GetParam( int param_density );
    GParam *param_denser;
    GParam *param_dense;
    GParam *param_coarse;

  private:
    GMrx *mrx_rotcof;
    GMrx *mrx_cri;
    GMrx *mrx_crx;

};

#endif