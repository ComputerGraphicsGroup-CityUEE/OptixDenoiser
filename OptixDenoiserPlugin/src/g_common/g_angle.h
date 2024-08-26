#ifndef G_ANGLE_H
#define G_ANGLE_H

#include "g_vector.h"

class GAngle
{
  public:
    GAngle();
    ~GAngle();

    void load( const char *spath );

    int nearest_index( FLOAT3 l );

    int n_src;
    FLOAT3 *v_src;
};




#endif