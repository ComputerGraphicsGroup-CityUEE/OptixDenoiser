// dct2class.h: interface for the dct2class class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DCT2CLASS_H__3D56AEDA_F3C0_4DD4_866A_C2E54C005132__INCLUDED_)
#define AFX_DCT2CLASS_H__3D56AEDA_F3C0_4DD4_866A_C2E54C005132__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef M_PI
#define M_PI 3.141592654
#endif

class dct2class  
{
public:
	dct2class(int dim);
	virtual ~dct2class();
public:
	void ForwardDCT(double *input,double *output);
	void InverseDCT(double *input,double *output);
//private:
public:
	int	m_dim;
	int	m_imgsize;
	double **m_C;
	double **m_Ct;
	double **m_Temp;

};

#endif // !defined(AFX_DCT2CLASS_H__3D56AEDA_F3C0_4DD4_866A_C2E54C005132__INCLUDED_)
