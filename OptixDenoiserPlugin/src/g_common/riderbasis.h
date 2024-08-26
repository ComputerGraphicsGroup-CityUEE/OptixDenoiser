#ifndef RIDERBASIS_H
#define RIDERBASIS_H

#include "g_vector.h"
#include "g_basis.h" 


/////////////////////////////////////////////////////////////////////////////
//  Sample code to prepare rider basis file.
//
//  RiderBasis *rb = new RiderBasis( parse_pmd( spath ), 50,50,50 );
//    rb->prepare_coef_table( vEnv[0].m, vEnv[1].m, vEnv[2].m, spath, "xxx.mrg" );
//  delete( rb);
//
//

/////////////////////////////////////////////////////////////////////////////
//  Sample code to prepare rider map2 basis file.
//  GMrx tmpEnv[3];
//  GMrx::load_all( "env_debug_rbf49.mrg", tmpEnv, 3 );
//  RiderBasis *rb = new RiderBasis( parse_pmd( "env_debug_rbf49.mrg" ), 50,50,50 );
//  GBasis *sh49 = new SHBasis2( 49, GParam::HALF_SPHERE_POS_Z );
//    rb->prepare_map2_table(
//      tmpEnv[0].m, tmpEnv[1].m, tmpEnv[2].m, 
//      sh49,
//      "env_debug_map2_sh49_rider.mrg"
//  );
//  delete( rb);
//  delete( sh49);
//




class RiderBasis : public GBasis
{
  public:

    RiderBasis( GBasis *b0, int a_grid, int b_grid, int r_grid );
    void GetSamples( float *pSample, const FLOAT3 *tL ) const;
    virtual ~RiderBasis();

    void SetRot( const float *mRotation );
    void GetRotatedCoef( const float *c0, float *c1 );

  
    void set_map2_basis( GBasis *b1 );
    void map2_basis( const float *c0, float *c1 );
    float mean( const float *c0 );
    float val( const float *c0, const FLOAT3 *v );
    void GetRotationMatrix( float *mR, const float *mRotation );
    GParam* GetParam( int param_density, GParam::P_TYPE domain );


    int A_GRID;
    int B_GRID;
    int R_GRID;


  void prepare_coef_table( 
    const float *pSrc_r, const float *pSrc_g, const float *pSrc_b, const char *puppet_path,
    const char *spath
  );

  void prepare_map2_table( 
    const float *src_r, const float *src_g, const float *src_b,
    GBasis *b1,
    const char *spath 
  );



  private:
    GBasis *puppet_basis;
    int table_offset;
};

#endif
