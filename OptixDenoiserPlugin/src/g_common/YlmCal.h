// YlmCal.h: interface for the YlmCal class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_YLMCAL_H__A0083F55_1833_453B_8BA4_F2F46CE4CA91__INCLUDED_)
#define AFX_YLMCAL_H__A0083F55_1833_453B_8BA4_F2F46CE4CA91__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef	M_PI
#define	M_PI	3.1415926
#endif

class YlmCal  
{
public:
	YlmCal();
	virtual ~YlmCal();
public:
	double factorial(int a);
	double N(int l, int m);
	double Q(int l, int m, float x);
	double Ylm(int l, int m, float cost, float p);
};

#endif // !defined(AFX_YLMCAL_H__A0083F55_1833_453B_8BA4_F2F46CE4CA91__INCLUDED_)
