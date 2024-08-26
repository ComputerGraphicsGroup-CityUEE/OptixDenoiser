// dct2class.cpp: implementation of the dct2class class.
//
//////////////////////////////////////////////////////////////////////
#define	M_PI		3.14159265358979323846

#include <math.h>
#include "dctf2class.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

dctf2class::dctf2class(int dim)
{
  m_dim=dim;
  m_imgsize=m_dim*m_dim;
  
  int i;
  int j;
  m_C=new float* [m_dim];
  m_Ct=new float* [m_dim];
  m_Temp=new float *[m_dim];
  for (i=0;i<m_dim;i++)
  {
    m_C[i]=new float [m_dim];
    m_Ct[i]=new float [m_dim];
    m_Temp[i]=new float [m_dim];
  }
  
  for (j=0;j<m_dim;j++) 
  {
    m_C[0][j]	= 1.0f / sqrtf((float)m_dim);
    m_Ct[j][0]	= m_C[0][j];
  }
  for (i=1;i<m_dim;i++)
  {
    for (j=0;j<m_dim;j++)
    {
      m_C[i][j]	= sqrtf(2.0f / m_dim)*\
        cosf( ((float)M_PI) *(2*j+1)*i / (2.0f*m_dim));
      m_Ct[j][i] = m_C[i][j];
    }
  }
}

dctf2class::~dctf2class()
{
  int i;
  for (i=0;i<m_dim;i++)
  {
    delete[] m_C[i];
    delete[] m_Ct[i];
    delete[] m_Temp[i];
  }
  delete[] m_C;
  delete[] m_Ct;
  delete[] m_Temp;
}

/*
 * The Forward DCT routine implements the matrix function:
 *
 *                     DCT = m_C * pixels * m_Ct
 */
void dctf2class::ForwardDCT(float *input,float *output)
{//only do DCT within m_dim*m_dim range
  int i;
  int j;
  int k;
  
  /*  MatrixMultiply( m_Temp, input, m_Ct ); */
  for ( i = 0 ; i < m_dim ; i++ ) {
    for ( j = 0 ; j < m_dim ; j++ ) {
      m_Temp[ i ][ j ] = 0.0;
      for ( k = 0 ; k < m_dim ; k++ )
        m_Temp[ i ][ j ] += input[ i*m_dim+k ] *
        m_Ct[ k ][ j ];
    }
  }
  
  /*  MatrixMultiply( output, m_C, m_Temp ); */
  for ( i = 0 ; i < m_dim ; i++ ) {
    for ( j = 0 ; j < m_dim ; j++ ) {
      output[ i*m_dim+j ] = 0.0;
      for ( k = 0 ; k < m_dim ; k++ )
        output[ i*m_dim+j ] += m_C[ i ][ k ] * m_Temp[ k][j ];
    }
  }
}


/*
* The Inverse DCT routine implements the matrix function:
*
*                     pixels = m_Ct * DCT * m_C
*/
void dctf2class::InverseDCT(float *input,float *output )
{
  int i;
  int j;
  int k;
  
  /*  MatrixMultiply( m_Temp, input, m_C ); */
  for ( i = 0 ; i < m_dim ; i++ ) {
    for ( j = 0 ; j < m_dim ; j++ ) {
      m_Temp[ i ][ j ] = 0.0;
      for ( k = 0 ; k < m_dim ; k++ )
        m_Temp[ i ][ j ] += input[ i*m_dim+k ] * m_C[ k ][ j ];
    }
  }
  
  /*  MatrixMultiply( output, m_Ct, m_Temp ); */
  for ( i = 0 ; i < m_dim ; i++ ) {
    for ( j = 0 ; j < m_dim ; j++ ) {
      output[i*m_dim+j] = 0.0;
      for ( k = 0 ; k < m_dim ; k++ )
        output[i*m_dim+j] += m_Ct[ i ][ k ] * m_Temp[ k ][ j ];
    }
  }
}