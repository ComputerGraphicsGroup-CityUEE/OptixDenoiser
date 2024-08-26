#include <iostream>
#include <float.h>
#include <math.h>
#include "ylmcalv.h"

ylmcalv::ylmcalv():shdeg(5)
{
  printf( "[DEFAULT]: SH order = %d\n", shdeg );
  numw = shdeg*shdeg;
  N_val   = new double [numw];
  Q_val   = new double [numw];
  ylm_val = new float [numw];
}

ylmcalv::ylmcalv( int in_shdeg ):
shdeg( in_shdeg )
{
  numw = shdeg*shdeg;
  N_val   = new double [numw];
  Q_val   = new double [numw];
  ylm_val = new float [numw];
}

ylmcalv::~ylmcalv()
{
  delete[] N_val;
  delete[] Q_val;
  delete[] ylm_val;
}

double ylmcalv::factorial(int a)
{
	switch (a)
	{
	case 0:	return 1.0;
	case 1: return 1.0;
	case 2: return 2.0;
	case 3:	return 6.0;
	case 4: return 24.0;
	case 5: return 120.0;
	case 6:	return 720.0;
	case 7: return 5040.0;
	case 8: return 40320.0;
	case 9:	return 362880.0;
	case 10: return 3628800.0;
	case 11: return 39916800.0;
	case 12: return 479001600.0;
	case 13: return 6227020800.0;
	case 14: return 87178291200.0;
	case 15: return 1307674368000.0;
	case 16: return 20922789888000.0;
	case 17: return 355687428096000.0;
	case 18: return 6402373705728000.0;
	case 19: return 121645100408832000.0;
	case 20: return 2342902008176640000.0;
	case 21: return 51090942171709440000.0;
	case 22: return 1124000727777607680000.0;
	case 23: return 25852016738884976640000.0;
	case 24: return 620448401733239439360000.0;
	case 25: return 15511210043330985984000000.0;
	case 26: return 403291461126605635584000000.0;
	case 27: return 10888869450418352160768000000.0;
	case 28: return 304888344611713860501504000000.0;
	case 29: return 8841761993739701954543616000000.0;
	default:
    double res = 8841761993739701954543616000000.0;
    for( int i=30; i<=a; i++ )
      res *= i;
    return res;
	}
}

int ylmcalv::getindex(int l, int m)
{
  return (l*l+l+m);
}

void ylmcalv::N_cal()
{
  int l, m;
  int mm;
  int index;
  for ( l=0; l<shdeg; l++ )
  {
    for ( m=-l; m<=l; m++ )
    {
      index = getindex(l,m);
      if (m<0)
      {
        mm=-m;
      }
      else
      {
        mm=m;
      }
      N_val[index] =    ( (2.0*l+1) * factorial(l-mm) ) 
                      / ( 2.0*M_PI * factorial(l+mm) );
      N_val[index] = sqrt( N_val[index] );
    }
  }
}


void ylmcalv::Q_cal( float x )
{
  int l, m;
  int mm;
  int index;
  for ( l=0; l<shdeg; l++ )
  {
    for ( m=-l; m<=l; m++ )
    {
      index = getindex(l,m);
      if ( (l==0) && (m==0) )
      {
        Q_val[index] = 1.0;
      }
      else
      {
        if (m<0)
        {
          mm=-m;
        }
        else
        {
          mm=m;
        }
        if (l==mm)
        {
          Q_val[index] = (1.0-2*mm) * sqrt(1.0-x*x) * Q_val[getindex(mm-1,mm-1)];
        }
        else if (l==mm+1)
        {
          Q_val[index] = x * (2.0*mm+1) * Q_val[getindex(mm,mm)];
        }
        else
        {
          Q_val[index] =    x * (2.0*l-1) * Q_val[getindex(l-1,mm)]
                          - (l+mm-1.0) * Q_val[getindex(l-2,mm)];
          Q_val[index] /= (l-mm);
        }
      }
    }
  }

}

// get sh basis vector
// Input: tt: theta, pp: phi
void ylmcalv::getylmvector( float *ylmvec, const float tt, const float pp )
{
  float costt;

  costt = cosf( tt );

  N_cal();
  Q_cal( costt );

  int l, m;
  int index;
  for ( l=0; l<shdeg; l++ )
  {
    for ( m=-l; m<=l; m++ )
    {
      
      index = getindex(l,m);
      if (m>0)
      {
        ylm_val[index] = float( N_val[index]*Q_val[index]*cos(m*pp) );
      }
      else if (m==0)
      {
        ylm_val[index] = float( N_val[getindex(l,0)]*Q_val[getindex(l,0)]*0.707106781 );
      }
      else
      {
        ylm_val[index] = float( N_val[index]*Q_val[getindex(l,-m)]*sin(-m*pp) );
      }
    }
  }
  memcpy( ylmvec, ylm_val,  sizeof(float)*numw );
}


void ylmcalv::getylmvector( float *ylmvec, FLOAT3 l )
{
  float theta, phi;
  Float32Angle( &theta, &phi, &l );
  getylmvector( ylmvec, theta, phi );
}
