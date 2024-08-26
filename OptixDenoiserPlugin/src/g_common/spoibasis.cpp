#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <float.h>

#include "g_common.h"
#include "spoibasis.h"
#include "poibasis.h"
#include "g_mrx.h"

SPOIBasis::SPOIBasis( int basis_num, GParam::P_TYPE domain)
{
  if( domain!=GParam::FULL_SPHERE )
  {
    printf( "[Error] MPOIBasis, not implemented yet.\n" );
    exit(-1);
  }

  m_type = BASIS_SPOI;
  m_domain = domain;
  n_basis = basis_num;
  GMrx A, B, invA, E;
  int i;

  switch( basis_num )
  {
    case 20:
      n_level=1;
      lpoi = (POIBasis**) malloc( n_level * sizeof(GBasis*) );
      lpoi[0] = new POIBasis(20, .6f);
    break;

    case 80:
      n_level=2;
      lpoi = (POIBasis**) malloc( n_level * sizeof(GBasis*) );
      lpoi[0] = new POIBasis(20, 0.87f);

      lpoi[1] = new POIBasis( lpoi[0], &POIBasis(80, 0.87f) );


      for( i=0; i<n_level-1; i++ )
      {
        A.load( lpoi[i]->n_basis, lpoi[i]->n_basis );
        B.load( lpoi[i+1]->n_basis, lpoi[i]->n_basis );

        GBasis::GetCorelation( A.m, lpoi[i], lpoi[i], lpoi[i]->GetParam(DENSE) );
        GBasis::GetCorelation( B.m, lpoi[i+1], lpoi[i], lpoi[i]->GetParam(DENSE) );
        invA.inverse(&A);
        E.mul( &invA, &B );
        nEt[i].transpose(&E);
        nEt[i].scalar_mul(-1);
      }
    break;

    case 320:
      n_level=3;
      lpoi = (POIBasis**) malloc( n_level * sizeof(GBasis*) );
      lpoi[0] = new POIBasis(20, 0.87f);
      lpoi[1] = new POIBasis( &POIBasis(20, 0.87f), &POIBasis(80, 0.87f) );
      lpoi[2] = new POIBasis( &POIBasis(80, 0.87f), &POIBasis(320, 0.87f) );


      for( i=0; i<n_level-1; i++ )
      {
        A.load( lpoi[i]->n_basis, lpoi[i]->n_basis );
        B.load( lpoi[i+1]->n_basis, lpoi[i]->n_basis );

        GBasis::GetCorelation( A.m, lpoi[i], lpoi[i], lpoi[i]->GetParam(DENSE) );
        GBasis::GetCorelation( B.m, lpoi[i+1], lpoi[i], lpoi[i]->GetParam(DENSE) );
        invA.inverse(&A);
        E.mul( &invA, &B );
        nEt[i].transpose(&E);
        nEt[i].scalar_mul(-1);
      }
    break;

    case 100:
      n_level=2;
      lpoi = (POIBasis**) malloc( n_level * sizeof(GBasis*) );
      lpoi[0] = new POIBasis(20, .6f);
      lpoi[1] = new POIBasis(80, .6f);


      for( i=0; i<n_level-1; i++ )
      {
        A.load( lpoi[i]->n_basis, lpoi[i]->n_basis );
        B.load( lpoi[i+1]->n_basis, lpoi[i]->n_basis );

        GBasis::GetCorelation( A.m, lpoi[i], lpoi[i], lpoi[i]->GetParam(DENSE) );
        GBasis::GetCorelation( B.m, lpoi[i+1], lpoi[i], lpoi[i]->GetParam(DENSE) );
        invA.inverse(&A);
        E.mul( &invA, &B );
        nEt[i].transpose(&E);
        nEt[i].scalar_mul(-1);
      }
    break;

    case 420:
      n_level=3;
      lpoi = (POIBasis**) malloc( n_level * sizeof(GBasis*) );
      lpoi[0] = new POIBasis(20, .6f);
      lpoi[1] = new POIBasis(80, .6f);
      lpoi[2] = new POIBasis(320, .6f);


      for( i=0; i<n_level-1; i++ )
      {
        A.load( lpoi[i]->n_basis, lpoi[i]->n_basis );
        B.load( lpoi[i+1]->n_basis, lpoi[i]->n_basis );

        GBasis::GetCorelation( A.m, lpoi[i], lpoi[i], lpoi[i]->GetParam(DENSE) );
        GBasis::GetCorelation( B.m, lpoi[i+1], lpoi[i], lpoi[i]->GetParam(DENSE) );
        invA.inverse(&A);
        E.mul( &invA, &B );
        nEt[i].transpose(&E);
        nEt[i].scalar_mul(-1);
      }
    break;

    case 1700:
      n_level=4;
      lpoi = (POIBasis**) malloc( n_level * sizeof(GBasis*) );
      lpoi[0] = new POIBasis(20, .6f);
      lpoi[1] = new POIBasis(80, .6f);
      lpoi[2] = new POIBasis(320, .6f);
      lpoi[3] = new POIBasis(1280, .6f);


      for( i=0; i<n_level-1; i++ )
      {
        A.load( lpoi[i]->n_basis, lpoi[i]->n_basis );
        B.load( lpoi[i+1]->n_basis, lpoi[i]->n_basis );

        GBasis::GetCorelation( A.m, lpoi[i], lpoi[i], lpoi[i]->GetParam(DENSE) );
        GBasis::GetCorelation( B.m, lpoi[i+1], lpoi[i], lpoi[i]->GetParam(DENSE) );
        invA.inverse(&A);
        E.mul( &invA, &B );
        nEt[i].transpose(&E);
        nEt[i].scalar_mul(-1);
      }
    break;


    default:
      printf( "[Error] SPOIBasis, basis_num can only be 20, 80, 320.\n" );
      exit(-1);
    break;
  }
}

SPOIBasis::~SPOIBasis()
{
  int i;
  for( i=0; i<n_level; i++ )
    delete lpoi[i];
  free(lpoi);
}



void SPOIBasis::GetSamples( float *pSample, const FLOAT3 *tL ) const 
{
  int i,m;
  GMrx v0, v1, vc, vd;
  int n0, n1;

  m=0;
  lpoi[0]->GetSamples( pSample, tL );
  m += lpoi[0]->n_basis;

  for( i=0; i<n_level-1; i++ )
  {
    n0 = lpoi[i]->n_basis;
    n1 = lpoi[i+1]->n_basis;

    v0.load( 1, n0 );
    v1.load( 1, n1 );

    lpoi[i]->GetSamples( v0.m, tL );
    lpoi[i+1]->GetSamples( v1.m, tL );

    vc.mul( &nEt[i], &v0 );
    vd.add( &vc, &v1 );

    memcpy( &pSample[m], vd.m, vd.row*sizeof(float) );
    m += n1;
  }
}

void SPOIBasis::GetRotationMatrix( float *mR, const float *mRotation )
{
  GBasis::GetRotationMatrix( mR, mRotation);
  return;

  int k;
  int n0, n1, N;
  POIBasis *p0, *p1;
  float eta0, eta1;
  GMrx A0, A1, m, B0, Bt0;
  GMrx T;

  GMrx eA, Ae, eAe;
  GMrx eB, Be;

    N = 0;
    m.load( n_basis, n_basis );

    p1 = lpoi[0];
    n1 = p1->n_basis;
    A1.load( n1, n1 );
    p1->GetRotationMatrix( A1.m, mRotation );
    m.setblk( A1.m, N,N, n1, n1 );
    N+=n1;

  for( k=0; k<n_level-1; k++ )
  {
    p0 = lpoi[k];
    p1 = lpoi[k+1];
   
    n0 = p0->n_basis;
    n1 = p1->n_basis;
    eta0 = p0->eta;
    eta1 = p1->eta;

    A0.load( n0, n0);
    p0->GetRotationMatrix( A0.m, mRotation );

    A1.load( n1, n1);
    p1->GetRotationMatrix( A1.m, mRotation );

    B0.load( n1, n0 );
    {
      int i,j;
      FLOAT3 ci, cj;
      for( i=0; i<n1; i++ )
      {
        ci = p1->center[i].rmul(mRotation);
        for( j=0; j<n0; j++ )
        {
          cj = p0->center[j];
          B0.matrix[j][i] = 
            4*G_PI * 
            ( (1-eta0)*(1-eta0)/(1+eta0) ) * 
            ( (1-eta1)*(1-eta1)/(1+eta1) ) * 
            POIBasis::fpoi0( vdot(&ci,&cj), eta0*eta1 );
        }
      }
    }
/*
    Bt0.load( n0, n1 );
    {
      int i,j;
      FLOAT3 ci, cj;
      for( j=0; j<n1; j++ )
        for( i=0; i<n0; i++ )
        {
          cj = p1->center[j];
          ci = p0->center[i].rmul(mRotation);
          Bt0.matrix[j][i] = 
            4*G_PI * 
            ( (1-eta0)*(1-eta0)/(1+eta0) ) * 
            ( (1-eta1)*(1-eta1)/(1+eta1) ) * 
            POIBasis::fpoi0( vdot(&ci,&cj), eta0*eta1 );
        }
    }
*/

    Bt0.load( n1, n0 );
    {
      int i,j;
      FLOAT3 ci, cj;
      for( j=0; j<n0; j++ )
      {
        cj = p0->center[j].rmul(mRotation);
        for( i=0; i<n1; i++ )
        {
          ci = p1->center[i];
          Bt0.matrix[j][i] = 
            4*G_PI * 
            ( (1-eta0)*(1-eta0)/(1+eta0) ) * 
            ( (1-eta1)*(1-eta1)/(1+eta1) ) * 
            POIBasis::fpoi0( vdot(&ci,&cj), eta0*eta1 );
        }
      }
    }
    Bt0.transpose(&Bt0);

    Ae.AxBt( &A0, &nEt[k] );
    eA.mul( &nEt[k], &A0 );

    T.add( &Ae, &B0 );
    m.setblk( T.m, N,0, T.col,T.row );

    T.add( &eA, &Bt0 );
    m.setblk( T.m, 0,N, T.col,T.row );

    eAe.mul( &nEt[k], &A0 );
    eAe.AxBt( &eAe, &nEt[k] );
    eB.mul( &nEt[k], &B0 );
    Be.AxBt( &Bt0, &nEt[k] );

    T.add( &eAe, &eB );
    T.add( &T, &Be );
    T.add( &T, &A1 );
    m.setblk( T.m, N,N, T.col,T.row );

    N+=n1;
  }

  //m.save( "m.pfm", "pfm" );

  memcpy( mR, m.m, n_basis * n_basis * sizeof(float) );
}

