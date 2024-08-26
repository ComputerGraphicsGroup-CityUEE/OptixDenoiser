#ifndef G_STL_H
#define G_STL_H

#include "g_vector.h"

class GStl
{
  public:
    GStl();
    ~GStl();
    void load_stl( const char *spath );
    void load_stl_ascii( const char *spath );
    void load_stl_binary( const char *spath );
    void save_stl( const char *spath ) const;
    void save_stl_ascii( const char *spath ) const;
    void clear();

    FLOAT3* faces;
    int nfaces;
    char stl_path[512];
  
    FLOAT3 vmax;
    FLOAT3 vmin;
};

#endif
