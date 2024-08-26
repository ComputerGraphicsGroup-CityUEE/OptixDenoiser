#ifndef YLMCALV_H
#define YLMCALV_H

#ifndef	M_PI
#define	M_PI		3.14159265358979323846
#endif

#include "g_vector.h"

class ylmcalv
{
public:
  ylmcalv();
	ylmcalv( int in_shdeg );
	virtual ~ylmcalv();

	void getylmvector( float *ylmvec, const float tt, const float pp );
	void getylmvector( float *ylmvec, FLOAT3 l );

private:
	double factorial(int a);
  int    getindex(int l, int m);
	void   N_cal();
  void   Q_cal( float x );

private:
  int shdeg;
  int numw;
  
  double *N_val;
  double *Q_val;
  float  *ylm_val;

};

#endif
