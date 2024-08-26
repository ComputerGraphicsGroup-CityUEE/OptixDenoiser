#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>

#include "g_common.h"

#define DEFINE_BASIS_STR
#include "g_basis.h"
#undef DEFINE_BASIS_STR

#include "SphereHammersleyParam.h"


void GBasis::GetCorTrace(
  float *trace, 
  const GBasis *basis0, const GBasis *basis1, 
  GParam *param0
){
  int n_trace = G_MIN( basis0->n_basis, basis1->n_basis );
  memset( trace, 0, n_trace * sizeof(float) );
  
  float *f0 = (float*) malloc( basis0->n_basis * sizeof(float) );
  float *f1 = (float*) malloc( basis1->n_basis * sizeof(float) );
  
  const FLOAT4 *l = param0->GetSamplesPt();

  int i, j;

  for( j=0; j<param0->n_sample; j++, l++ )
  {
    basis0->GetSamples( f0, (FLOAT3*)l );
    basis1->GetSamples( f1, (FLOAT3*)l );
    for( i=0; i<n_trace; i++ )
      trace[i] += f0[i] * f1[i] * l->w;
  }

  free( f0 );
  free( f1 );

}

void GBasis::GetCorelation(
  float *pCorel, 
  const GBasis *basis0, const GBasis *basis1, 
  GParam *param0
){
  memset( pCorel, 0, basis0->n_basis * basis1->n_basis * sizeof(float) );
  
  float *p_sample0 = (float*) malloc( basis0->n_basis * sizeof(float) );
  float *p_sample1 = (float*) malloc( basis1->n_basis * sizeof(float) );
  
  float *t_sample0;
  float *t_sample1;
  float *tCorel;
  
  const FLOAT4 *tL = param0->GetSamplesPt();
  for( int i=0; i<param0->n_sample; i++, tL++ )
  {
    basis0->GetSamples( p_sample0, (FLOAT3*)tL );
    basis1->GetSamples( p_sample1, (FLOAT3*)tL );
    
    t_sample1 = p_sample1;
    tCorel = pCorel;
    for( int fj=0; fj<basis1->n_basis; fj++, t_sample1++ )
    {
      t_sample0 = p_sample0;
      for( int fi=0; fi<basis0->n_basis; fi++, tCorel++, t_sample0++ )
        *tCorel += *t_sample0 * *t_sample1 * tL->w;
    }
    /*
    for( int fj=0; fj<m; fj++ )
    for( int fi=0; fi<m; fi++ )
				pCorelation[ fj*m + fi ] += p_sample0[fi] *p_sample1[fj] * tL->w;
    */
  }
  free( p_sample0 );
  free( p_sample1 );
  
}

void GBasis::GetCorelation2(
  float *pCorel, 
  GBasis *basis0, GBasis *basis1, 
  GParam *param0, float *mRot
){
  memset( pCorel, 0, basis0->n_basis * basis1->n_basis * sizeof(float) );
  
  float *p_sample0 = (float*) malloc( basis0->n_basis * sizeof(float) );
  float *p_sample1 = (float*) malloc( basis1->n_basis * sizeof(float) );
  
  float *t_sample0;
  float *t_sample1;
  float *tCorel;
  
  const FLOAT4 *tL = param0->GetSamplesPt();
  for( int i=0; i<param0->n_sample; i++, tL++ )
  {
    FLOAT3 tmpL;

      //Mul_R3xM33( mRot, (FLOAT3*)tL, &tmpL );
      //basis0->GetSamples( p_sample0, (FLOAT3*)tL );
      //basis1->GetSamples( p_sample1, &tmpL );
    
      Mul_M33xC3( mRot, (FLOAT3*)tL, &tmpL );
      basis0->GetSamples( p_sample0, &tmpL );
      basis1->GetSamples( p_sample1, (FLOAT3*)tL );

    t_sample1 = p_sample1;
    tCorel = pCorel;
    for( int fj=0; fj<basis1->n_basis; fj++, t_sample1++ )
    {
      t_sample0 = p_sample0;
      for( int fi=0; fi<basis0->n_basis; fi++, tCorel++, t_sample0++ )
        *tCorel += *t_sample0 * *t_sample1 * tL->w;
    }
    /*
      for( int fj=0; fj<m; fj++ )
        for( int fi=0; fi<m; fi++ )
		  mR[ fj*m + fi ] += p_sample0[fi] *p_sample1[fj] * tL->w;
    */
  }
  free( p_sample0 );
  free( p_sample1 );
  
}

GBasis::GBasis()
{ 
  bOrthonormal = false; 
  m_type = BASIS_NULL;
  m_domain = GParam::FULL_SPHERE;

  mrx_rotcof = NULL;
  mrx_cri = NULL;
  mrx_crx = NULL;

  param_coarse = NULL;
  param_dense = NULL;
  param_denser = NULL;
}

GParam* GBasis::GetParam( int param_density )
{
  switch( param_density )
  {
    case COARSE:
      if( param_coarse==NULL )
        param_coarse = new SphereHammersleyParam( m_domain, 500 );
      return param_coarse;

    case DENSE:
      if( param_dense==NULL )
        param_dense = new SphereHammersleyParam( m_domain, 10000 );
      return param_dense;

    case DENSER:
      if( param_denser==NULL )
        param_denser = new SphereHammersleyParam( m_domain, 100000 );
      return param_denser;

    default:
      printf( "[Error] : GBasis::GetParam(), unknown parameter density type.\n" );
      exit(-1);
  }
}

void GBasis::SetRot( const float *mRotation )
{
  
  if( mrx_rotcof==NULL )
  {
    mrx_rotcof = new GMrx( n_basis, n_basis );
  }


  GetRotationMatrix( mrx_rotcof->m, mRotation );

  if( !bOrthonormal )
  {
    if(mrx_cri==NULL)
    {
      mrx_cri = new GMrx();


      GMrx mrx_cr;
        mrx_cr.load( n_basis, n_basis );


      GParam *xParam = GetParam(DENSE);
        GBasis::GetCorelation( mrx_cr.m, this, this, xParam );
        mrx_cri->inverse( &mrx_cr );


    }

    mrx_rotcof->mul( mrx_cri, mrx_rotcof );
  }

  //printf( "mrx_rotcof norm = %f\n", mrx_rotcof->norm() );
}

void GBasis::GetRotatedCoef( const float *c0, float *c1 )
{

  GMrx mrx_c0, mrx_c1;
  mrx_c0.load( c0, 1, n_basis );
  mrx_c1.mul( mrx_rotcof, &mrx_c0 );

  memcpy( c1, mrx_c1.m, n_basis*sizeof(float) );
}




float GBasis::variance( const float *c0 )
{
  GParam *xParam = GetParam(DENSE);
  const FLOAT4 *l = xParam->GetSamplesPt();
  float mean_val, var, fval;
  int i, n;

  n = xParam->n_sample;
  mean_val = mean(c0);
  var = 0;
  for( i=0; i<n; i++, l++ )
  {
    fval  = val( c0, (FLOAT3*)l );
    fval -= mean_val;
    fval *= fval;
    var  += fval * l->w;
  }

  return var / 4 / G_PI;
}

float GBasis::mean( const float *c0 )
{

  GParam *xParam = GetParam(DENSE);



  float mean_val = 0;

    float *p_sample0 = (float*) malloc( n_basis * sizeof(float) );

    const FLOAT4 *tL = xParam->GetSamplesPt();
    for( int i=0; i<xParam->n_sample; i++, tL++ )
    {
      GetSamples( p_sample0, (FLOAT3*)tL );

      float t_val = 0;
      for( int j=0; j<n_basis; j++ )
        t_val += p_sample0[j]*c0[j];

      mean_val+=t_val * tL->w;
    }

    free( p_sample0 );

  return mean_val / 4 / G_PI;
}

// for some reason, even coarsely defined parameter will provide good result.
void GBasis::GetRotationMatrix( float *mR, const float *mRotation )
{
  GParam *pRotParam = GetParam(COARSE);

  float m[9];
    m[0] = mRotation[0];
    m[1] = mRotation[1];
    m[2] = mRotation[2];
    m[3] = mRotation[4];
    m[4] = mRotation[5];
    m[5] = mRotation[6];
    m[6] = mRotation[8];
    m[7] = mRotation[9];
    m[8] = mRotation[10];
  
  GBasis::GetCorelation2( mR, this, this, pRotParam, m );
}

GBasis::~GBasis()
{
  SAFE_DELETE( mrx_rotcof );
  SAFE_DELETE( mrx_cri );
  
  SAFE_DELETE( mrx_crx );

  SAFE_DELETE( param_dense );
  SAFE_DELETE( param_coarse );
  SAFE_DELETE( param_denser );
}

float GBasis::val( const float *c0, const FLOAT3 *v )
{
  float *p_samples = (float*) malloc( n_basis * sizeof(float) );
  GetSamples( p_samples, v );

  float res = 0;

  for( int i=0; i<n_basis; i++ )
    res += p_samples[i] * c0[i];

  free(p_samples);

  return res;
}



void GBasis::set_map2_basis( GBasis *b1 )
{
  SAFE_DELETE( mrx_crx );


  mrx_crx = new GMrx( b1->n_basis, this->n_basis );



  GParam *xParam = b1->GetParam(DENSE);
  GBasis::GetCorelation( mrx_crx->m, this, b1, xParam );

  //printf( "map2 norm = %f\n", mrx_crx->norm() );

}

void GBasis::map2_basis( const float *c0, float *c1 )
{
  GMrx mrx_c0;
    mrx_c0.load( c0, 1, this->n_basis );

  GMrx mrx_c1;
    mrx_c1.mul( mrx_crx, &mrx_c0 );

  memcpy( c1, mrx_c1.m, mrx_c1.row * sizeof(float) );
}

char* GBasis::BasisStr()
{
  if( m_type >= sizeof(BASIS_STR)/sizeof(BASIS_STR[0]) )
    m_type = BASIS_NULL;
  return (char*)BASIS_STR[m_type];
}

BASIS_TYPE StrBasis( const char *basis_tag )
{
  int i;

  for( i=0; i<sizeof(BASIS_STR)/sizeof(BASIS_STR[0]); i++ )
    if( strcmp( basis_tag, BASIS_STR[i] ) ==0 )
      return (BASIS_TYPE)i;
      return BASIS_NULL;

  /*
  if( strcmp( basis_tag, "RBF" ) ==0 )
    return BASIS_RBF;
  else if( strcmp( basis_tag, "SH" ) ==0 )
    return BASIS_SH;
  else if( strcmp( basis_tag, "HDCT" ) ==0 )
    return BASIS_HDCT;
  else if( strcmp( basis_tag, "HP" ) ==0 )
    return BASIS_HP;
  else if( strcmp( basis_tag, "RIDER" ) ==0 )
    return BASIS_RIDER;
  else if( strcmp( basis_tag, "MRBF" ) ==0 )
    return BASIS_MRBF;
  else
    return BASIS_NULL;
*/
}

