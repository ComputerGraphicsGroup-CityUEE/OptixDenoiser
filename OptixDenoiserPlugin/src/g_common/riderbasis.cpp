#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

#include "g_common.h"
#include "g_basis.h" 

#include "riderbasis.h"

RiderBasis::RiderBasis( GBasis *b0, int a_grid, int b_grid, int r_grid )
{

  if( b0->m_type == BASIS_RIDER )
  {
    printf( "[Error] : RiderBasis::RiderBasis(), BASIS_RIDER on BASIS_RIDER is not allowed.\n" );
    exit(-1);
  }

  puppet_basis = b0;

  n_basis = puppet_basis->n_basis;
  bOrthonormal = puppet_basis->bOrthonormal;
  m_domain = puppet_basis->m_domain;
  
  m_type = BASIS_RIDER;

    A_GRID = a_grid;
    B_GRID = b_grid;
    R_GRID = r_grid;

  table_offset = 0;
}





RiderBasis::~RiderBasis()
{
  SAFE_DELETE( puppet_basis );
}



void RiderBasis::SetRot( const float *mRotation )
{
  int ilen = A_GRID;
  int jlen = B_GRID;
  int klen = R_GRID;
  int tlen = n_basis;

  float h,a,b;
  Matrix2Euler( mRotation, &h,&a,&b );

  float i0 = -G_PI;
  float j0 = -G_PI/2;
  float k0 = -G_PI;

  float istep = 2*G_PI/ilen;
  float jstep =   G_PI/jlen;
  float kstep = 2*G_PI/klen;

  int i = (int)( (h-i0)/istep );
  int j = (int)( (a-j0)/jstep );
  int k = (int)( (b-k0)/kstep );

  table_offset = i*tlen + j*ilen*tlen + k*jlen*ilen*tlen;
}

void RiderBasis::GetRotatedCoef( const float *c0, float *c1 )
{
  memcpy( c1, &c0[table_offset], n_basis*sizeof(float) );
}



void   RiderBasis::GetSamples( float *pSample, const FLOAT3 *tL ) const
{ puppet_basis->GetSamples( pSample, tL ); }

void   RiderBasis::set_map2_basis( GBasis *b1 )
{ puppet_basis->set_map2_basis( b1 ); }

void   RiderBasis::map2_basis( const float *c0, float *c1 )
{ puppet_basis->map2_basis( c0, c1 ); }

float         RiderBasis::mean( const float *c0 )
{ return puppet_basis->mean( c0 ); }

float         RiderBasis::val( const float *c0, const FLOAT3 *v )
{ return puppet_basis->val( c0, v ); }

void   RiderBasis::GetRotationMatrix( float *mR, const float *mRotation )
{ puppet_basis->GetRotationMatrix( mR, mRotation ); }

GParam*        RiderBasis::GetParam( int param_density, GParam::P_TYPE domain )
{  return puppet_basis->GetParam( param_density ); }




void RiderBasis::prepare_coef_table( 
  const float *pSrc_r, const float *pSrc_g, const float *pSrc_b, const char *puppet_path,
  const char *spath
){
  printf( "Preparing pre-computed coefficients table... \n" );

  float i0 = -G_PI;
  float j0 = -G_PI/2;
  float k0 = -G_PI;

  int ilen = A_GRID;
  int jlen = B_GRID;
  int klen = R_GRID;
  int tlen = n_basis;

  float istep = 2*G_PI/ilen;
  float jstep =   G_PI/jlen;
  float kstep = 2*G_PI/klen;

  GMrx mrx_pc[3];
    mrx_pc[0].load( n_basis, ilen*jlen*klen );
    mrx_pc[1].load( n_basis, ilen*jlen*klen );
    mrx_pc[2].load( n_basis, ilen*jlen*klen );

    for( int k=0; k<klen; k++ )
    {
      for( int j=0; j<jlen; j++ )
      {
        for( int i=0; i<ilen; i++ )
        {
          float mR[16];

          float heading  = i0 + i*istep;   //[-pi  , pi  ]
          float attitude = j0 + j*jstep;   //[-pi/2, pi/2]
          float bank     = k0 + k*kstep;   //[-pi  , pi  ]


          Euler2Matrix( mR, heading,attitude,bank );
          puppet_basis->SetRot(mR);


          int offset = i + j*ilen + k*jlen*ilen;
            puppet_basis->GetRotatedCoef( pSrc_r, mrx_pc[0].matrix[offset] );
            puppet_basis->GetRotatedCoef( pSrc_g, mrx_pc[1].matrix[offset] );
            puppet_basis->GetRotatedCoef( pSrc_b, mrx_pc[2].matrix[offset] );

        }
        printf( "%i,%i\n", j,k);
      }
      printf( "\n" );
    }

      char s_comment[256], *t_comment=s_comment;
        t_comment += sprintf( t_comment, "PMD\n" );
        t_comment += sprintf( t_comment, "BASIS RIDER %i\n", n_basis );
        t_comment += sprintf( t_comment, "EULER_GRID %i %i %i\n", A_GRID, B_GRID, R_GRID );
        t_comment += sprintf( t_comment, "PUPPET_PATH %s\n", puppet_path );
        t_comment += sprintf( t_comment, "/PMD\n" );
    GMrx::save_all( spath, mrx_pc, 3, "bin", s_comment );


  printf( "done\n" );

}

void RiderBasis::prepare_map2_table( 
  const float *src_r, const float *src_g, const float *src_b,
  GBasis *b1,
  const char *spath 
){
  printf( "Preparing pre-computed coefficients table... \n" );

  float i0 = -G_PI;
  float j0 = -G_PI/2;
  float k0 = -G_PI;

  int ilen = A_GRID;
  int jlen = B_GRID;
  int klen = R_GRID;
  int tlen = b1->n_basis;

  float istep = 2*G_PI/ilen;
  float jstep =   G_PI/jlen;
  float kstep = 2*G_PI/klen;

  GMrx mrx_pc[3];
    mrx_pc[0].load( tlen, ilen*jlen*klen );
    mrx_pc[1].load( tlen, ilen*jlen*klen );
    mrx_pc[2].load( tlen, ilen*jlen*klen );


  GMrx mrx_des[3];
    mrx_des[0].load( n_basis,1 );
    mrx_des[1].load( n_basis,1 );
    mrx_des[2].load( n_basis,1 );

  
    
    
  set_map2_basis( b1 );

  
  for( int k=0; k<klen; k++ )
    {
      for( int j=0; j<jlen; j++ )
      {
        for( int i=0; i<ilen; i++ )
        {
          float mR[16];

          float heading  = i0 + i*istep;   //[-pi  , pi  ]
          float attitude = j0 + j*jstep;   //[-pi/2, pi/2]
          float bank     = k0 + k*kstep;   //[-pi  , pi  ]


          Euler2Matrix( mR, heading,attitude,bank );
          puppet_basis->SetRot(mR);


          int offset = i + j*ilen + k*jlen*ilen;
            puppet_basis->GetRotatedCoef( src_r, mrx_des[0].m );
            puppet_basis->GetRotatedCoef( src_g, mrx_des[1].m );
            puppet_basis->GetRotatedCoef( src_b, mrx_des[2].m );

            puppet_basis->map2_basis( mrx_des[0].m, mrx_pc[0].matrix[offset] );
            puppet_basis->map2_basis( mrx_des[1].m, mrx_pc[1].matrix[offset] );
            puppet_basis->map2_basis( mrx_des[2].m, mrx_pc[2].matrix[offset] );

        }
        printf( "%i,%i\n", j,k);
      }
      printf( "\n" );
    }

      char s_comment[256], *t_comment=s_comment;
        t_comment += sprintf( t_comment, "PMD\n" );
        t_comment += sprintf( t_comment, "BASIS RIDER %i\n", n_basis );
        t_comment += sprintf( t_comment, "EULER_GRID %i %i %i\n", A_GRID, B_GRID, R_GRID );
        t_comment += sprintf( t_comment, "PUPPET_PATH %s\n", "MAP2" );
        t_comment += sprintf( t_comment, "/PMD\n" );
    GMrx::save_all( spath, mrx_pc, 3, "bin", s_comment );


  printf( "done\n" );
}
