#ifndef HEALIPXPARAM_H
#define HEALIPXPARAM_H

#include "g_common.h"
#include "g_param.h" 

class HealpixParam : public GParam
{
  public:
    HealpixParam( P_TYPE T, int nside );
    ~HealpixParam();
    FLOAT4* GetSamplesPt();

  private:
    FLOAT4 *m_pL;
    int m_nside;
};


#endif
