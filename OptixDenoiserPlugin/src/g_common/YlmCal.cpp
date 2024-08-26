// YlmCal.cpp: implementation of the YlmCal class.
//
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include "YlmCal.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

YlmCal::YlmCal()
{
}

YlmCal::~YlmCal()
{
}

double YlmCal::factorial(int a)
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
	case 27: return 1088886945041835216768000000.0;
	case 28: return 304888344611713860501504000000.0;
	case 29: return 8841761993739701954543616000000.0;
	default: 
      double res = 8841761993739701954543616000000.0;
      for( int i=30; i<=a; i++ )
        res *= i;
      return res;
	}
}

double YlmCal::N(int l, int m)
{
	double up;
	double down;
	if (m<0)
		m*=-1;
	up=(2*l+1)*factorial(l-m);
	down=2.0*M_PI*factorial(l+m);
	return sqrt(up/down);
}

double YlmCal::Q(int l, int m, float x)
{
	
	if ((l==0)&(m==0))
		return 1.0;
	else
	{
		if (m<0)
			m*=-1;
		if (l==m)
			return (double)(1-2*m)*sqrt(1.0-x*x)*Q(m-1,m-1,x);
		else if (l==m+1)
			return x*(double)(2*m+1)*Q(m,m,x);
		else
			return (double)(x*(2*l-1))/(double)(l-m)*Q(l-1,m,x) - (double)(l+m-1)/(double)(l-m)*Q(l-2,m,x);
	}
}

double YlmCal::Ylm(int l, int m, float cost, float p)
{
	if (m>0)
		return N(l,m)*Q(l,m,cost)*cos(m*p);
	else if (m==0)
		return N(l,0)*Q(l,0,cost)/sqrt(2);
	else
	{
		m*=-1;
		return N(l,m)*Q(l,m,cost)*sin(m*p);
	}
}