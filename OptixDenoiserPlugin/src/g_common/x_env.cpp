#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "g_common.h"
#include "g_basis.h"

#include "x_env.h"

xEnv::xEnv()
{
  PIF_BASIS = NULL;
  ENV_BASIS = NULL;
  reset_basis = true;
}

xEnv::~xEnv()
{
  SAFE_DELETE( PIF_BASIS );
  SAFE_DELETE( ENV_BASIS );
}


void xEnv::SetBasis_Pif( GBasis *pif_basis )
{
  SAFE_DELETE(PIF_BASIS);

  PIF_BASIS = pif_basis;

  reset_basis = true;
}

void xEnv::SetBasis_Env( GBasis *env_basis )
{
  SAFE_DELETE(ENV_BASIS);

  ENV_BASIS = env_basis;

  reset_basis = true;
}

void xEnv::SetRot( const float *mRotation )
{
  ENV_BASIS->SetRot( mRotation );
}

void xEnv::env_map2_pif( const float *env_coef, float *env2pif_coef )
{
  GMrx mrx_env_rot;
  mrx_env_rot.load( 1, ENV_BASIS->n_basis );

  if( reset_basis )
  {
    ENV_BASIS->set_map2_basis(PIF_BASIS);
    reset_basis = false;
  }
  ENV_BASIS->GetRotatedCoef( env_coef, mrx_env_rot.m );
  ENV_BASIS->map2_basis( mrx_env_rot.m, env2pif_coef );
}

void xEnv::env_rot( const float *env_coef0, float *env_coef1 )
{
  ENV_BASIS->GetRotatedCoef( env_coef0, env_coef1 );
}

void xEnv::map2pif( const float *env_coef0, float *env2pif_coef )
{
  if( reset_basis )
  {
    ENV_BASIS->set_map2_basis(PIF_BASIS);
    reset_basis = false;
  }

  ENV_BASIS->map2_basis( env_coef0, env2pif_coef );
}