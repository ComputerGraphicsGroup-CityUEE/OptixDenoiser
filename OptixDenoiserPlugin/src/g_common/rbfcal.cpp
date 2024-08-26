// rbfcal.cpp: implementation of the rbfcal class.
//
//////////////////////////////////////////////////////////////////////

#include <iostream>
#include <math.h>
//#include "pcvalues.h"
#define	M_PI		3.14159265358979323846
#include "rbfcal.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//-------------------------------------------------------
// Input: 
//    no_center: number of center used for rbf
//    trange   : theta range of centre
//    prange   : phi range of centre
rbfcal::rbfcal(int no_centre, float trange, float prange)
{
  m_no_centre = no_centre;
  m_trange    = trange;
  m_prange    = prange;
  m_theta     = new float [m_no_centre];
  m_phi       = new float [m_no_centre];
  m_centre    = new point [m_no_centre];
  // calculate centre position in spherical domain
  calcentre();
  // calculate the spread (delta) based on the centre position
  caldelta();
  //printf("Average delta among all min Delta %0.12lf\n", m_delta);

/*  
  //debug
  FILE *fpw;
  char filename[100];
  int  i;
  sprintf(filename, "centre%02d.txt", m_no_centre);
  fpw = fopen(filename, "wt");
  for (i=0; i<m_no_centre; i++)
    fprintf(fpw, "%15f  %15f\n", m_theta[i], m_phi[i]);
  fprintf(fpw, "%0.12lf\n", m_delta);
  fclose(fpw);
  exit(0);
*/
}

rbfcal::~rbfcal()
{
  delete[] m_theta;
  delete[] m_phi;
  delete[] m_centre;
}

//---------------------------------------------------------
// Input:
//   number  : which number of weight
//   l_theta : lighting theta in radian
//   l_phi   : lighting phi in radian
//
// Output:
//   result  : the basis value in float format
float rbfcal::G_cal(int number, float l_theta, float l_phi)
{
  point light;
  float result;

  light.x  =   sin(l_theta) * cos(l_phi);
  light.y  =   cos(l_theta);
  light.z  = - sin(l_theta) * sin(l_phi);

  result = cal_theta(m_centre[number], light);
  result = result * result / (2 * m_delta * m_delta);
  result = exp(-result);

  return result;
}


// This program is used to generate the centres of radial basis function using
// the Hammersley point set, base 2.
//
// Reference:
// Tien-Tsin Wong, Wai-Shing Luk and Pheng-Ann Heng, "Sampling with 
// Hammersley and Halton points", Journal of Graphic Tools, Vol.2, No.2, 
// 1997, pp9-24. 
//
// 24 Sept 2003
//
// Kwok-Hung Choy, Tien-Tsin Wong
//
// All Rights Reserved
// The Chinese University of Hong Kong
//
void rbfcal::calcentre()
{
  float   *centre;
  float   *theta, *phi;
  float   magnitude;
  float   temp1, temp2;
  int     i;
  int     n, m;

  n = m_no_centre;                // total no. of centre
  m = 0;

  // Since there may be some constraints to bound the center positions and reject
  // some generated points, the program logic is as below. Starting from 
  // n = no_center, it tries to collect enough number of points satisfying the 
  // constraints, If in this iteration there are no enough points, all generated
  // points are removed. Then n++ and regenerate the whole set of points. The
  // iteration stops until sufficient are collected.
  while (true)
  {
    // allocate sufficient buffer to hold current no of centers 
    centre = new float[3 * n];
    theta  = new float[n];
    phi    = new float[n];

    SphereHammersley2(centre, n, 3);

    for (i = 0; i < n; i++)
    {
      theta[i] = (acos(centre[i * 3 + 2]) * 180) / M_PI;
      magnitude = sqrt(centre[i * 3] * centre[i * 3] + centre[i * 3 + 1] * centre[i * 3 + 1]);
      phi[i] = (acos(centre[i * 3] / magnitude) * 180) / M_PI;
      if (centre[i * 3] < 0 && centre[i * 3 + 1] < 0)
        phi[i] = 360 - phi[i];
      if (centre[i * 3] > 0 && centre[i * 3 + 1] < 0)
        phi[i] = 360 - phi[i];

      if (theta[i] < m_trange && phi[i] < m_prange) 
      {
        theta[m] = theta[i];
        phi[m]   = phi[i];
        m++;          // increase count & record if constraint setting
      }
    }

    if (m == m_no_centre)   
      break;          // sufficient no of centers
    else  
    {                 // insufficient no of centers
      n++;            // increase no of center by 1
      m = 0;          // regenerate the whole new set of points
    }
    delete[]centre;
    delete[]theta;
    delete[]phi;
  }  

  
  // Output each center in (theta, phi) in degree
  for (i = 0; i < m_no_centre; i++)
  {
    m_theta[i] = theta[i];
    m_phi[i]   = phi[i];
  }
  for (i=0; i < m_no_centre; i++)
  {
    temp1 = m_theta[i] * M_PI / 180.0;
    temp2 = m_phi[i]   * M_PI / 180.0;
    m_centre[i].x =   sin(temp1) * cos(temp2);
    m_centre[i].y =   cos(temp1);
    m_centre[i].z = - sin(temp1) * sin(temp2);
  }
  delete[] theta;
  delete[] phi;
}

void rbfcal::SphereHammersley2(float *result, int n, int p1)
{
  float   a, p, ip, t, st, phi, phirad;
  int     k, kk, pos;

  for (k = 0, pos = 0; k < n; k++)
  {
    t = 0;
    ip = 1.0 / p1;              // recipical of p1
    for (p = ip, kk = k; kk; p *= ip, kk /= p1) // kk = (int)(kk/p1)
      if ((a = kk % p1))
        t += a * p;
    t = 2.0 * t - 1.0;          // map from [0,1] to [-1,1]

    phi = (k + 0.5) / n;
    phirad = phi * 2.0 * M_PI;  // map to [0, 2 pi)

    st = sqrt(1.0 - t * t);
    result[pos++] = st * cos(phirad);
    result[pos++] = st * sin(phirad);
    result[pos++] = t;
  }
}


void rbfcal::caldelta()
{
  int     i,j;
  float   minDelta;
  float   tempDelta;  
  float   *smallest_delta;
  float   average_minDelta;

  minDelta       = 9999999;
  smallest_delta = (float *)malloc(sizeof(float) * m_no_centre);
  if (smallest_delta==NULL)
  {
    fprintf (stderr, "[calspread]: no memory\n");
    exit(1);
  }
  for (i=0; i < m_no_centre; i++)
  {
    minDelta = 9999999;
    for (j=0; j < m_no_centre; j++)
    {
      if (i != j) 
      {
        tempDelta = cal_theta(m_centre[i],m_centre[j]);
        if (tempDelta < minDelta)
          minDelta = tempDelta;
      }
    }
    smallest_delta[i] = minDelta;        
  }

  average_minDelta = 0;  
  for(i=0; i<m_no_centre; i++)
    average_minDelta += smallest_delta[i];

  average_minDelta /= (float)m_no_centre;

  // put detla value into m_delta
  m_delta = average_minDelta;
  
  free(smallest_delta);
}

//calculate the theta, geodestic, between 2 points on a sphere
float rbfcal::cal_theta(point in1, point in2)
{
  float ret;
  ret = in1.x * in2.x + in1.y * in2.y + in1.z * in2.z; // dot product
  ret = (float)acos(ret);
  return ret;
}
