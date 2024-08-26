#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//#include "YlmCal.h"
#include "ylmcalv.h"


#include "SphereHammersleyParam.h"
#include "SHBasis2.h"

SHBasis2::SHBasis2( int basis_num, GParam::P_TYPE domain )
{

  m_domain = domain;

  if( m_domain == GParam::FULL_SPHERE )
    bOrthonormal = true; 
  else
    bOrthonormal = false; 

  m_type = BASIS_SH;
  
  if( fmod( sqrt(basis_num), 1) != 0 )
  {
    printf( "Error : Number of Basis of SH should square root to an integer.\n" );
    exit(0);
  }
  n_basis = basis_num;



}

void SHBasis2::GetSamples( float *pSample, const FLOAT3 *tL ) const
{
  float theta, phi;
  Float32Angle( &theta, &phi, tL );

  int lmax = (int) sqrt(n_basis);


  ylmcalv ylm(lmax);
  ylm.getylmvector( pSample, theta, phi );

  //YlmCal ylm;
  //for( int l=0; l<lmax; l++ )
  //  for( int m=-l; m<=l; m++ )
  //    pSample[l*l+m+l] = (float)ylm.Ylm( l, m, cosf(theta), phi );

}

/*
void InitCoeffArr(int l_cnt);

  int lmax = (int) sqrt(n_basis);
  InitCoeffArr( lmax+1 );
void RotateEulCC(float *CC, int l_max, float eulerAlpha, float eulerBeta, float eulerGamma);

void SHBasis2::SetRot( const float *mRotation )
{
  const float *m = mRotation;

  eb = acos( m[5] );

  if( fabs(eb)>1e-7 )
  {
    ea = atan2(  m[6] , m[4] );
    eg = atan2( m[9] , -m[1] );

  printf( "a %f %f %f\n", ea, eb, eg );
  }else
  {
    ea = atan2(m[2], m[0]);
    eg = 0;
  printf( "b %f %f %f\n", ea, eb, eg );
  }




}


void SHBasis2::GetRotatedCoef( const float *c0, float *c1 )
{
  int lmax = (int) sqrt(n_basis);
  memcpy( c1, c0, n_basis*sizeof(float) );

  static float x =0;
  RotateEulCC( c1, lmax, ea, eb, eg  );

  x+=.01;
}











#define FALSE  0
#define TRUE   1
#define M_SQRT2         1.41421356237309504880
#define FTAB_MAX 50


#define MAX(a,b)          ((a>b)? a : b)
#define MIN(a,b)          ((a<b)? a : b)
#define NEG(x) ((x & 0x01) ? (-1) : (1))
#define C(l,m)   CC[Idx[l] + m + l]




int   *Idx=NULL;
//int Idx[1024];

// Init C1, C2, C3, C4 for fast calculation
// Allocate memory for all arrays
void InitCoeffArr(int l_cnt)
{
  int m, l, l_max=l_cnt-1, l_m;
  float rpsqrt2;

  if (Idx) free(Idx);

  Idx=(int*)calloc(l_cnt,sizeof(int));

  for (m=0 ; m<=l_max ; m++)
    Idx[m] = m*m;
}

float factorial(int x)
{
  // The factorial table will be build on the fly.
  static float facttable[FTAB_MAX]; // store the 1st 50 factorial
  static char first=TRUE;
  int i;
  float acc;

  if (first)
  {
    first = FALSE;
    for (i=0 ; i<FTAB_MAX ; i++)
      facttable[i] = 0;  // init to invalid value
    facttable[0] = facttable[1] = 1;
  }

  if (x<FTAB_MAX && facttable[x]!=0)
    return facttable[x];

  // otherwise evaluate it.
  acc = 1;
  for (i=x ; i>0 ; i--)
    if (i>=FTAB_MAX || facttable[i]==0)
      acc *= i;
    else  // we can stop here
    {
      acc *= facttable[i];
      break;
    }
  if (x<FTAB_MAX)
    facttable[x] = acc;  // update the table
  return acc;
}

static float EvalDLMB(int l, int mp, int m, float beta)
{
  float c = sqrt(factorial(l + mp) * factorial(l - mp) * factorial(l + m) * factorial(l - m));
  float sum = 0.0f;
  float coshbeta = cos(beta / 2.0f);
  float sinhbeta = sin(beta / 2.0f);
  int beginK = MAX(0, m - mp);
  int endK = MIN(l - mp, l + m);
  int sign = ((beginK + mp - m) & 0x01) ? -1 : 1;       // odd then -1

  for (int k = beginK; k <= endK; k++)
  {

    sum += sign
      * pow(coshbeta, 2 * l + m - mp - 2 * k)
      * pow(sinhbeta, 2 * k + mp - m) / factorial(k) / factorial(l + m - k) / factorial(l - mp - k) / factorial(mp - m + k);

    sign = -sign;
  }

  return c * sum;
}
static double RotationVector(int l_cnt, double alpha, double beta, double gamma, int mp, int m)
{
  double val = 0;

  // override arguments
  // mp = 2, m = 1;

  if ((mp > 0) && (m > 0))      //1
    val = EvalDLMB(l_cnt, mp, m, beta) * cos(m * gamma + mp * alpha) +
      NEG(mp) * EvalDLMB(l_cnt, -mp, m, beta) * cos(m * gamma - mp * alpha);

  else if ((mp == 0) && (m > 0))        //2
    val = EvalDLMB(l_cnt, 0, m, beta) * M_SQRT2 * cos(m * gamma);

  else if ((mp < 0) && (m > 0)) //3
    val =
      NEG(mp + 1) * EvalDLMB(l_cnt, mp, m,
                             beta) * sin(m * gamma + mp * alpha) +
      EvalDLMB(l_cnt, -mp, m, beta) * sin(m * gamma - mp * alpha);

  else if ((mp > 0) && (m == 0))        //4
    val = EvalDLMB(l_cnt, mp, 0, beta) * M_SQRT2 * cos(m * alpha);

  else if ((mp == 0) && (m == 0))       //5
    val = EvalDLMB(l_cnt, 0, 0, beta);

  else if ((mp < 0) && (m == 0))        //6
    val = NEG(mp + 1) * EvalDLMB(l_cnt, mp, 0, beta) * M_SQRT2 * sin(mp * alpha);

  else if ((mp > 0) && (m < 0)) //7
    val =
      NEG(m) * EvalDLMB(l_cnt, mp, m,
                        beta) * sin(m * gamma + mp * alpha) + NEG(m +
                                                                  mp) *
      EvalDLMB(l_cnt, -mp, m, beta) * sin(m * gamma - mp * alpha);

  else if ((mp == 0) && (m < 0))        //8
    val = NEG(m) * EvalDLMB(l_cnt, 0, m, beta) * M_SQRT2 * cos(m * gamma);

  else if ((mp < 0) && (m < 0)) //9
    val =
      NEG(m + mp) * EvalDLMB(l_cnt, mp, m,
                             beta) * cos(m * gamma + mp * alpha) + NEG(m +
                                                                       1)
      * EvalDLMB(l_cnt, -mp, m, beta) * cos(m * gamma - mp * alpha);

  else
  {
    fputs("sanity check: RotationVector(), m_prime, m out of range\n", stderr);
    fprintf(stderr, "m_prime = %d \t m = %d\n", mp, m);
    return (1);                 // FAIL
  }

#ifdef DEBUG
  printf("RotationVector() val = %f\n", val);
#endif

  return val;
}

void RotateEulCC(float *CC, int l_max, float eulerAlpha, float eulerBeta, float eulerGamma)
{
  int l, m, mp, k, len;
  float tmpCC[256];

  {
    float sum;

    // For each (l,m) pair
    len = 1;
    for (l = 0; l <= l_max; l++)
    {
      // tmpCC = reduced equation
      for (m = -l, k = 0; m <= l; m++)
      {
        sum = 0.0;
        for (mp = -l; mp <= l; mp++)
          sum += C(l, mp) * RotationVector(l, eulerAlpha, eulerBeta, eulerGamma, mp, m);
        tmpCC[k++] = sum;
      }

      // C(l,m) = tmpCC
      memcpy(&(C(l, -l)), tmpCC, len * sizeof(float));
      len += 2;
    }

  }
}
*/