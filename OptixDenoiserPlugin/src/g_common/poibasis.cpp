#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <float.h>

#include "g_common.h"
#include "g_vector.h"
#include "poibasis.h"


float POIBasis::fpoi0( float x, float eta )
{
  return (1-eta*eta) / powf( 1 -2*eta*x +eta*eta, 1.5 );
}

float fpoi( float x, float eta )
{
  return ( (1-eta)*(1-eta)/(1+eta) ) * (1-eta*eta) / powf( 1 -2*eta*x +eta*eta, 1.5 );
  //return (1-eta*eta) / powf( 1 -2*eta*x +eta*eta, 1.5 );
}

float feta_gx;
float feta_x0;
float feta( float eta )
{
  return fpoi(1,eta)*feta_gx - fpoi(feta_x0,eta);
}

POIBasis::POIBasis( POIBasis *p0, POIBasis *p1 )
{

  float ffff = 10000*FLT_EPSILON;

  m_domain = p0->m_domain;
  n_basis = p1->n_basis - p0->n_basis;
  eta = p1->eta;

  center = (FLOAT3*) malloc( n_basis * sizeof(FLOAT3) );
  int i, j, k;

  FLOAT3 c0, c1;
  for( j=0, k=0; j<p1->n_basis; j++ )
  {
    c1 = p1->center[j];
    bool found = false;
    for( i=0; i<p0->n_basis; i++ )
    {
      c0 = p0->center[i];
      if( 
        fabs(c0.x-c1.x)<=.001 && 
        fabs(c0.y-c1.y)<=.001 && 
        fabs(c0.z-c1.z)<=.001 )
      {
        found = true;
      }
    }

    if( !found )
      center[k++] = c1;
  }

  if( k!=n_basis )
  {
    printf( "[Error] POIBasis::POIBasis, %i %i\n", k, n_basis );
    exit(-1);
  }
}

float cal_n_factor( int n_center, float gx );
POIBasis::POIBasis( int basis_num, float GX, GParam::P_TYPE domain)
{

  if( domain!=GParam::FULL_SPHERE )
  {
    printf( "[Error] POIBasis, not implemented yet.\n" );
    exit(-1);
  }

  if( 
    basis_num!=20 && 
    basis_num!=80 && 
    basis_num!=320 &&
    basis_num!=1280 &&
    basis_num!=5120 &&
    true
  ){
    printf( "[Error] POIBasis, basis_num can only be 20, 80, 320.\n" );
    exit(-1);
  }

  m_type = BASIS_POI;
  m_domain = domain;
  n_basis = basis_num;
  gx = GX;

  center = (FLOAT3*) malloc( basis_num * sizeof(FLOAT3) );



  char spath[256], str[256];
  float theta, phi;
  int i;

  sprintf( spath, "../../data/SW%06i.txt", n_basis );
    FILE *f0 = fopen( spath, "rb" );
      if( f0==NULL )
      {
        printf( "[Error] POIBasis, file %s not found.\n", spath );
        exit(-1);
      }
      for( i=0; i<n_basis; i++ )
      {
        fgets( str, 256, f0 );
        sscanf( str, "%f %f", &theta, &phi );
        Angle2Float3( theta, phi, &center[i] );
      }
    fclose(f0);

  //feta_gx = gx;
  //feta_x0 = 1-2.0f/basis_num;
  //eta = Bisect( 0.01f, .99f, feta );

  eta = cal_n_factor( n_basis, gx );

}

POIBasis::~POIBasis()
{
  free(center);
}

void POIBasis::GetSamples( float *pSample, const FLOAT3 *tL ) const 
{
  int i;
  for( i=0; i<n_basis; i++ )
    pSample[i] = fpoi( vdot(&center[i],tL), eta );
}



void binarysearch(float *n, float *LB, float *UB, int slope);

float cal_n_factor( int n_center, float gx )
{
  int   j;
  float a1, a2;
  float LB, UB;
  float n_fact;
  float r;
  float x;
  float geox;
  float dummy;
  float *p;
  a1 = 1.0f;
  a2 = 1.05f;

  p = new float [2];
    LB = 0.0f;
    UB = 1.0f;
    n_fact = (LB+UB)/2.0f;
    r = float( acos(1-2.0/double(n_center)) );
    p[0] = a1*r;
    p[1] = a2*r;

    dummy = float(  (double)pow(1-n_fact, 6)/(double)(gx*gx)  );
    dummy = float(  pow(dummy, 1.0/3.0)  );
    x     = 1 + n_fact*n_fact - dummy;
    x     = float(  x/(2.0*n_fact)  );
    geox  = float(  acos(x)  );

    for (j=0; j<16; j++)
    {
      if ((geox<=p[1]) && (geox>=p[0]))
      {
        j=16;
      }
      else if (geox>p[1])
      {
        binarysearch(&n_fact, &LB, &UB, 1);
        dummy = float(  (double)pow(1-n_fact, 6)/(double)(gx*gx)  );
        dummy = float(  pow(dummy, 1.0/3.0)  );
        x     = 1 + n_fact*n_fact - dummy;
        x     = float(  x/(2.0*n_fact)  );
        geox  = float(  acos(x)  );
      }
      else
      {
        binarysearch(&n_fact, &LB, &UB, -1);
        dummy = float(  (double)pow(1-n_fact, 6)/(double)(gx*gx)  );
        dummy = float(  pow(dummy, 1.0/3.0)  );
        x     = 1 + n_fact*n_fact - dummy;
        x     = float(  x/(2.0*n_fact)  );
        geox  = float(  acos(x)  );
      }
    }

  delete[] p;

  return n_fact;
}
void binarysearch(float *n, float *LB, float *UB, int slope)
{
  float nn;
  float LLB;
  float UUB;
  float range;

  nn  = (*n);
  LLB = (*LB);
  UUB = (*UB);

  if (slope==1)
  {
    *LB   = nn;
    *UB   = UUB;
  }
  else if (slope==-1)
  {
    *LB   = LLB;
    *UB   = nn;
  }
  else
  {
    fprintf(stderr, "[Err]: Error inputing binary search slop\n");
    exit(-1);
  }
  range = (*UB)-(*LB);
  *n    = (*LB)+range/2.0f;
}

void POIBasis::GetRotationMatrix( float *mR, const float *mRotation )
{
  //GBasis::GetRotationMatrix(mR, mRotation);
  //return;
  int i,j;
  FLOAT3 ci, cj;
  for( j=0; j<n_basis; j++ )
    for( i=0; i<n_basis; i++ )
    {
      cj = center[j];
      ci = center[i].rmul(mRotation);
      mR[ j*n_basis + i ] = ( (1-eta)*(1-eta)/(1+eta) ) * ( (1-eta)*(1-eta)/(1+eta) ) * 4*G_PI * fpoi0( vdot(&ci,&cj), eta*eta );
    }

}

