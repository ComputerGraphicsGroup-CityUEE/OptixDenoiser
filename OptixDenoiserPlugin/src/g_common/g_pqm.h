#ifndef G_PQM_H
#define G_PQM_H

#include "g_vector.h"

class GPqm
{
  public:
    int format;
    int w, h, d;
    unsigned char *qm;
    FLOAT3 *sf, *nf;

    int lm_size;
    int qm_size;

    GPqm();
    ~GPqm();

    void safe_load( int width, int height, int depth, int internalformat );
    void load( int width, int height, int depth, int internalformat );
    void load( const char *spath );
    void load( int level, const FLOAT3 *fm );
    void load( int level, const FLOAT3 *fm, FLOAT3 lower_bound, FLOAT3 upper_bound );
    void load( int level, const FLOAT3 *fm, float lower_bound, float upper_bound );

    void save( const char *spath, const char *type="pqm" );

    void* get_lm( int level );

    void TexImage3D();
    void TexImage2D( int level=0 );
    void TexImage1D();

  private:
    unsigned char *lm;
    int win_id;
    unsigned int tex_id;
};



class GPq4
{
  public:
    int format;
    int w, h, d;
    unsigned char *qm;
    FLOAT4 *sf, *nf;

    int lm_size;
    int qm_size;

    GPq4();
    ~GPq4();

    void safe_load( int width, int height, int depth, int internalformat );
    void load( int level, const FLOAT4 *fm, FLOAT4 lower_bound, FLOAT4 upper_bound );
    void load( int level, const FLOAT4 *fm );
    void load( int level, const FLOAT3 *fm );
    void load( int width, int height, int depth, int internalformat );
    void load( const char *spath );

    void save( const char *spath );

    void* get_lm( int level );

    void TexImage3D();
    void TexImage2D( int level=0 );
    void TexImage1D();

  private:
    unsigned char *lm;
    int win_id;
    unsigned int tex_id;
};

#endif
