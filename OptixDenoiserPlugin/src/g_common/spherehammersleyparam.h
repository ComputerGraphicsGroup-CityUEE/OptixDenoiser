#ifndef SHPEREHAMMERSLEYPARAM_H
#define SHPEREHAMMERSLEYPARAM_H

#include "g_common.h"
#include "g_param.h"

class SphereHammersleyParam : public GParam
{
  public:
    SphereHammersleyParam( P_TYPE T, int SAMPLE_NUM );
    ~SphereHammersleyParam();
    FLOAT4* GetSamplesPt() ;

  private:
    FLOAT4 *m_pL;
    void Get_HemiSphereHammersley(FLOAT4 *pL, int n_point );
    void Get_SphereHammersley(FLOAT4 *pL, int n_point );
};

#endif
