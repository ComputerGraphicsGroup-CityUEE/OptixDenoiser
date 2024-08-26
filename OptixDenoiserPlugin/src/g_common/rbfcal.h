// rbfcal.h: interface for the rbfcal class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RBFCAL_H__995C3379_003F_4F69_AC6A_024E3B696C4F__INCLUDED_)
#define AFX_RBFCAL_H__995C3379_003F_4F69_AC6A_024E3B696C4F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct point
{
  float x;
  float y;
  float z;
};

typedef struct point point;

class rbfcal  
{
public:
	rbfcal(int no_centre, float trange, float prange);
	virtual ~rbfcal();
public:
  float G_cal(int number, float l_theta, float l_phi);
private:
  void  calcentre();
  void  SphereHammersley2(float *result, int n, int p1);
  void  caldelta();
  float cal_theta(point in1, point in2);
public:
  int   m_no_centre;
  float m_trange, m_prange;
  float *m_theta, *m_phi;
  point *m_centre;
  float m_delta;
};

#endif // !defined(AFX_RBFCAL_H__995C3379_003F_4F69_AC6A_024E3B696C4F__INCLUDED_)
