#ifndef XENV_H
#define XENV_H

#include "g_common.h"
#include "g_basis.h"

class xEnv
{
  public:

    xEnv();
    ~xEnv();

    void SetBasis_Pif( GBasis *pif_basis );
    void SetBasis_Env( GBasis *env_basis );

    void SetRot( const float *mRotation );
    void env_map2_pif( const float *env_coef, float *env2pif_coef );
    void env_rot( const float *env_coef0, float *env_coef1 );
    void map2pif( const float *env_coef0, float *env2pif_coef );

    GBasis *PIF_BASIS;
    GBasis *ENV_BASIS;

  private:

    bool reset_basis;

};

#endif