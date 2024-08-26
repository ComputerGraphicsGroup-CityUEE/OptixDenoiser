#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "g_common.h"
#include "g_pfm.h"

#include "g_mrz.h"


GMrz::GMrz()
{
  memset( this, 0, sizeof(GMrz) );
}

GMrz::GMrz( const GMrz &a )
{
  memset( this, 0, sizeof(GMrz) );
  load( a.row, a.col );
  memcpy( m, a.m, row*col*sizeof(float) );
}

GMrz::~GMrz()
{
  clear();
}

void GMrz::clear()
{
  SAFE_FREE(matrix);
  memset( this, 0, sizeof(GMrz) );
}


void GMrz::load( int number_of_row, int number_of_col )
{
  clear();

  row = number_of_row;
  col = number_of_col;

  matrix = (float**) malloc2d( row, col, sizeof(float) );
  m = matrix[0];
  memset( m, 0, row*col*sizeof(float) );
}

void GMrz::load( int number_of_row, int number_of_col, float *data )
{
  load( number_of_col, number_of_row );
  memcpy( m, data, row*col*sizeof(float) );
  *this = ~(*this);
}

void GMrz::load( const FLOAT3 &nx, const FLOAT3 &ny, const FLOAT3 &nz )
{
  identity(4);
  memcpy( matrix[0], &nx, sizeof(FLOAT3) );
  memcpy( matrix[1], &ny, sizeof(FLOAT3) );
  memcpy( matrix[2], &nz, sizeof(FLOAT3) );
}

void GMrz::identity( int n )
{
  load( n, n );

  int i;
  for( i=0; i<n; i++ )
    matrix[i][i] = 1;
}

void GMrz::load( const char *spath )
{
  GPath gp = parse_spath( spath );

  if( strcmp( gp.ename,"pfm" )==0 )
  {

    // int i, j;
    //GPf1 pf1;
    //  pf1.load( spath );
    //load( pf1.h, pf1.w );
    //for( j=0; j<pf1.h; j++ )
    //  for( i=0; i<pf1.w; i++ )
    //    matrix[i][j] = pf1.pm[j][i];

    char str[256];
    int w, h, j;
    GMrz vrow;
    FILE *f0 = fopen( spath, "rb" );
    fgets( str, 256, f0 );
    if( str[0]!='P' && str[1]!='f' )
    {
      printf( "[Error] GMrz::load, incorrect file format\n" );
      exit(-1);
    }
    while( fgets( str, 256, f0 ) && str[0] == '#' );
    sscanf(str, "%i %i", &w, &h);
    fgets( str, 256, f0 );
    // sscanf(str, "%f", &byte_ordering);
    load( h, w );
    vrow.load( 1, w );
    for( j=0; j<h; j++ )
    {
      fread( vrow.m, sizeof(float), w, f0 );
      setrow( h-1-j, vrow );
    }
    fclose( f0 );


  }else if( strcmp( gp.ename,"txt" )==0 )
  {
    char str[2048], *tstr, sval[256];

    {
      int w=0, h=0;
      FILE *f0 = fopen( spath, "rt" );
      while( fgets( str, 2048, f0 ) )
      {
        tstr = str;
        while( *tstr==' ' ) tstr++;
        w=0;
        while( sscanf( tstr, "%s", sval )>0 )
        {
          tstr+=strlen( sval )+1;
          while( *tstr==' ' ) tstr++;
          w++;
        }
        h++;
      }
      fclose(f0);
      load( h, w );
    }

    {
      int i, j;
      FILE *f0 = fopen( spath, "rt" );
      for( i=0; i<row; i++ )
        for( j=0; j<col; j++ )
          fscanf( f0, "%f", &matrix[j][i] );
      fclose(f0);
    }

  }else
  {
    printf( "[Error] GMrz::load, unsupported file format.\n" );
    exit(-1);
  }
}

GMrz GMrz::getrow( int row_index ) const
{
  const GMrz &a = *this;
  int i;
  
  GMrz vrow;
  vrow.load( 1, col );
  for( i=0; i<a.col; i++ )
    vrow.m[i] = a.matrix[i][row_index];
  return vrow;
}

void GMrz::setrow( int row_index, const GMrz &vrow )
{
  if( col!=vrow.col )
  {
    printf( "[Error] GMrz::setrow(), matrix dimension don't match.\n" );
    exit(-1);
  }

  const GMrz &a = *this;
  int i;
  for( i=0; i<a.col; i++ )
    a.matrix[i][row_index] = vrow.m[i];
}

void GMrz::save( const char *spath ) const
{
  const GMrz &a = *this;

  GPath gp = parse_spath( spath );

  if( strcmp( gp.ename,"pfm" )==0 )
  {
    int j;
    GMrz vrow;
    FILE *f0 = fopen( spath, "wb" );
      fprintf( f0, "Pf\n" );
	    fprintf( f0, "%i %i\n", a.col, a.row );
	    fprintf( f0, "%f\n", -1.f );
      for( j=a.row-1; j>=0; j-- )
      {
        vrow = getrow(j);
        fwrite( vrow.m, sizeof(float), a.col, f0 );
      }
    fclose( f0 );
  }else if( strcmp( gp.ename,"txt" )==0 )
  {
    int i, j;

    FILE *f0 = fopen( spath, "wt" );
      for( i=0; i<row; i++ )
      {
        for( j=0; j<col; j++ )
          fprintf( f0, "%f ", matrix[j][i] );
        fprintf( f0, "\n" );
      }
    fclose(f0);
  }else
  {
    printf( "[Error] GMrz::save, unsupported file format.\n" );
    exit(-1);
  }
}


GMrz GMrz::operator~() const
{
  const GMrz &a = *this;
  GMrz b;

  b.load( a.col, a.row );

  int i, j;

  for( j=0; j<a.col; j++ )
    for( i=0; i<a.row; i++ )
      b.matrix[i][j] = a.matrix[j][i];

  return b;
}

GMrz& GMrz::operator=( const GMrz &a )
{
  load( a.row, a.col );
  memcpy( m, a.m, row*col*sizeof(float) );

  return *this;
}

GMrz GMrz::operator+( const GMrz &a ) const
{
  if( row!=a.row || col!=a.col )
  {
    printf( "[Error] GMrz::operator+, matrix dimension don't match.\n" );
    exit(-1);
  }

  int i, n;
  const float *tm, *ta;
  float *tb;
  GMrz b;
    b.load( row, col );

    tm =   m;
    ta = a.m;
    tb = b.m;
    n = row * col;

  for( i=0; i<n; i++, tm++, ta++, tb++ )
    *tb = *tm + *ta;

  return b;
}

GMrz GMrz::operator-( const GMrz &a ) const
{
  if( row!=a.row || col!=a.col )
  {
    printf( "[Error] GMrz::operator-, matrix dimension don't match.\n" );
    exit(-1);
  }

  int i, n;
  const float *tm, *ta;
  float *tb;
  GMrz b;
    b.load( row, col );

    tm =   m;
    ta = a.m;
    tb = b.m;
    n = row * col;

  for( i=0; i<n; i++, tm++, ta++, tb++ )
    *tb = *tm - *ta;

  return b;
}

GMrz GMrz::operator*( const GMrz &b ) const
{
  const GMrz &a = *this;

  if( a.col!=b.row )
  {
    printf( "[Error] GMrz::operator*, matrix dimension don't match.\n" );
    exit(-1);
  }

  GMrz c, vrow;
  int i, j;

  c.load( a.row, b.col );
  for( i=0; i<c.row; i++ )
  {
    vrow = a.getrow(i);
    for( j=0; j<c.col; j++ )
      c.matrix[j][i] = vdot( vrow.m, b.matrix[j], col );
  }

  return c;
}

GMrz& GMrz::operator=( const float &a )
{
  float *ta = m;
  int n = row * col;
  int i;

  for( i=0; i<n; i++, ta++ )
    *ta = a;

  return *this;
}

GMrz GMrz::operator+( const float &a ) const
{
  int i, n;
  const float *tm;
  float *tb;
  GMrz b;
    b.load( row, col );

    tm =   m;
    tb = b.m;
    n = row * col;

  for( i=0; i<n; i++, tm++, tb++ )
    *tb = *tm + a;

  return b;
}

GMrz GMrz::operator-( const float &a ) const
{
  int i, n;
  const float *tm;
  float *tb;
  GMrz b;
    b.load( row, col );

    tm =   m;
    tb = b.m;
    n = row * col;

  for( i=0; i<n; i++, tm++, tb++ )
    *tb = *tm - a;

  return b;
}

GMrz GMrz::operator*( const float &a ) const
{
  int i, n;
  const float *tm;
  float *tb;
  GMrz b;
    b.load( row, col );

    tm =   m;
    tb = b.m;
    n = row * col;

  for( i=0; i<n; i++, tm++, tb++ )
    *tb = *tm * a;

  return b;
}

GMrz GMrz::operator/( const float &a ) const
{
  int i, n;
  const float *tm;
  float *tb;
  GMrz b;
    b.load( row, col );

    tm =   m;
    tb = b.m;
    n = row * col;

  for( i=0; i<n; i++, tm++, tb++ )
    *tb = *tm / a;

  return b;
}

float GMrz::det() const
{
  if( row == col )
    return det( matrix, row );
  else
  {
    printf( "[Error] : GMrz::det(), row and col is not the same.\n" ); 
    exit(-1);
  }
}

float GMrz::det( float **mm, int order )
{
  if(order<=1)
    return mm[0][0];

  float res=0;

  int i,j;
  int sgn = 1;
  float **cf = new float*[order-1];
  float **t_cf;

  for( j=0; j<order; j++, sgn=-sgn )
  {
    t_cf=cf;
    for( i=0; i<j; i++ )
     *t_cf++ = &mm[i][1];
    for( i=j+1; i<order; i++ )
     *t_cf++ = &mm[i][1];
    
    res += sgn * mm[j][0] * det( cf, order-1 );
  }

  delete[] cf;
  return res;
}

void GMrz::print( const char *prefix ) const
{
  int i, j;

  for( i=0; i<row; i++ )
  {
    for( j=0; j<col; j++ )
      printf( prefix, matrix[j][i] );
    printf( "\n" );
  }
  printf( "\n" );
}


void GMrz::print_info() const
{
  printf( "(row,col) %i %i\n", row, col );
}

FLOAT4 GMrz::operator*( const FLOAT3 &a ) const
{
  FLOAT4 v( a, 1 );
  return *this * v ;
}

FLOAT4 GMrz::operator*( const FLOAT4 &a ) const
{
  FLOAT4 v;

  v  = a.x * *( (FLOAT4*) matrix[0] );
  v += a.y * *( (FLOAT4*) matrix[1] );
  v += a.z * *( (FLOAT4*) matrix[2] );
  v += a.w * *( (FLOAT4*) matrix[3] );

  return v;
}

GMrz& GMrz::operator+=( const GMrz &a )
{
  if( row!=a.row || col!=a.col )
  {
    printf( "[Error] GMrz::operator+=, matrix dimension don't match.\n" );
    exit(-1);
  }

  int i;
  for( i=0; i<row*col; i++ )
    m[i] += a.m[i];
  return *this;
}

GMrz& GMrz::operator-=( const GMrz &a )
{
  if( row!=a.row || col!=a.col )
  {
    printf( "[Error] GMrz::operator-=, matrix dimension don't match.\n" );
    exit(-1);
  }

  int i;
  for( i=0; i<row*col; i++ )
    m[i] -= a.m[i];
  return *this;
}


FLOAT4 operator*( const FLOAT3 &a, const GMrz &mrz )
{
  FLOAT4 v( a, 1 );
  return v*mrz;
}

FLOAT4 operator*( const FLOAT4 &a, const GMrz &mrz )
{
  return FLOAT4( 
    vdot((float*)&a, mrz.matrix[0],4), 
    vdot((float*)&a, mrz.matrix[1],4), 
    vdot((float*)&a, mrz.matrix[2],4), 
    vdot((float*)&a, mrz.matrix[3],4) 
  );
}

float GMrz::norm() const
{
  GMrz u, s, v;
  svd(u,s,v);
  return s.matrix[0][0];
}

float GMrz::condition() const
{
  int idx = G_MIN(row,col)-1;
  GMrz u, s, v;
  svd(u,s,v);
  return s.matrix[0][0] / s.matrix[idx][idx];
}

GMrz GMrz::inverse() const
{
  GMrz u, s, v;
  svd(u,s,v);

  int i, n;
    n = G_MIN(row,col);
  for( i=0; i<n; i++ )
    s.matrix[i][i] = 1/s.matrix[i][i];

  return v * s * ~u;
}

GMrz operator+( const float &a, const GMrz &mrz ){ return mrz+a; }
GMrz operator*( const float &a, const GMrz &mrz ){ return mrz*a; }

GMrz operator-( const float &a, const GMrz &mrz )
{
  GMrz b;
    b.load( mrz.row, mrz.col );
  
  float *tm, *tb;
    tm = mrz.m;
    tb = b.m;
  
  int i, n;
    n = mrz.row * mrz.col;
  for( i=0; i<n; i++, tm++, tb++ )
    *tb = a - *tm;

  return b;
}

GMrz GMrz::operator-() const
{
  GMrz b;
    b.load( row, col );

  float *tm, *tb;
    tm = m;
    tb = b.m;

  int i, n;
    n = row * col;

  for( i=0; i<n; i++, tm++, tb++ )
    *tb = - *tm;
  return b;
}

#ifdef _M_AMD64
  #pragma comment (lib, "../../lib/lapack/lib/LAPACK.lib")  // Link with LAPACK
  #pragma comment (lib, "../../lib/lapack/lib/BLAS.lib")    // Link with BLAS
#else
  #pragma comment (lib, "../../lib/lapack/lib/LAPACK.lib")  // Link with LAPACK
  #pragma comment (lib, "../../lib/lapack/lib/BLAS.lib")    // Link with BLAS
#endif

#include "../../lib/lapack/include/f2c.h"
#include "../../lib/lapack/include/lapack.h"


void GMrz::svd( GMrz &u, GMrz &s, GMrz &v ) const
{
  u.load( row, row );
  s.load( row, col );
  v.load( col, col );

  integer M = row;
  integer N = col;
  
  integer LDA = M;
  integer LDU = M;
  integer LDVT = N;
  integer LWORK = max( M*N, max(3*min(M,N)+max(M,N),5*min(M,N)) );
  integer INFO;

  double *aa = (double*) malloc( M * N * sizeof(double) );
  double *uu = (double*) malloc( M * M * sizeof(double) );
  double *ss = (double*) malloc( min(M,N) * sizeof(double) );
  double *vv = (double*) malloc( N * N * sizeof(double) );
  double *wk = (double*) malloc( LWORK * sizeof(double) );

  int i, j;
  for( j=0; j<col; j++ )
    for( i=0; i<row; i++ )
      aa[j*row+i] = matrix[j][i];
  
  dgesvd_( 
    "A", "A", 
    &M, &N, aa, 
    &LDA, ss, 
    uu, &LDU, 
    vv, &LDVT, 
    wk, &LWORK, &INFO);

  for( j=0; j<row; j++ )
    for( i=0; i<row; i++ )
      u.matrix[j][i] = (float)uu[j*row+i];

  for( i=0; i<min(M,N); i++ )
    s.matrix[i][i] = (float)ss[i];

  for( j=0; j<col; j++ )
    for( i=0; i<col; i++ )
      v.matrix[i][j] = (float)vv[j*col+i];

  free(aa);
  free(uu);
  free(ss);
  free(vv);
  free(wk);
}

// http://docs.sun.com/source/819-0497/dsyevx.html
void GMrz::eig( int n_eig, GMrz &s, GMrz &v ) const
{
  if( col!=row )
  {
    printf( "GMrz::eig(), only applyable to symmetric matrix.\n" );
    exit(-1);
  }

  // absolute error tolerance for eigenvalue convergence determination
    double tol = 0.0;

  // input
    integer n;
    double *a;
    double vl, vu;
    integer il, iu;

  // system workspace
    integer lwork;
    double  *work;
    integer *iwork;

  // output
    integer nfound;
    double *eigenvalues;
    double *eigenvector;
    integer *ifail;
    integer info;

  n = row;
  vl = 0;
  vu = 0;
  il = G_MAX( n-n_eig+1, 1 );
  iu = n;
  a = (double*)  malloc( n * n * sizeof(double) );
  {
    float  *v0;
    double *v1;
    int i, l;
      l = n*n;
      v0 = m;
      v1 = a;
    for( i=0; i<l; i++, v0++, v1++ )
      *v1 = *v0;
  }


  lwork = max( 1, 8*n );
  work  = (double*)  malloc( lwork * sizeof(double) );
  iwork = (integer*) malloc( lwork * sizeof(integer) );

  eigenvalues = (double*)  malloc(     n * sizeof(double) );
  eigenvector = (double*)  malloc( n * n * sizeof(double) );

  ifail = (integer*) malloc( n * sizeof(integer) );

  dsyevx_(
    "V", "I", "U", 
    &n, a, &n, 
    &vl, &vu, &il, &iu, &tol, 
    &nfound, eigenvalues, eigenvector, &n, 
    work, &lwork, iwork, 
    ifail, &info
  );

  {
    s.load( n_eig, n_eig );
    v.load( n, n_eig );

    int i, j;
    double *v0;
    float  *v1;

    for( j=0; j<nfound; j++ )
      s.matrix[j][j] = (float)eigenvalues[nfound-j-1];

    for( j=0; j<nfound; j++ )
    {
      v0 = &eigenvector[ (nfound-j-1) *n];
      v1 = v.matrix[j];
      for( i=0; i<n; i++, v0++, v1++ )
        *v1 = (float)*v0;
    }
  }

  free(a );
  free(work);
  free(iwork);
  free(eigenvalues);
  free(eigenvector);
  free(ifail);
}

GMrz GMrz::eigs( int n_eig ) const
{
  GMrz a, u, s, v;
  const GMrz &c = *this;
  int i;

  if( row<col )
  {
    a = c*~c;
    a.eig( n_eig, s, u );
    for( i=0; i<s.col; i++ )
      if( s.matrix[i][i]>0 )
      s.matrix[i][i] = 1 / sqrtf( s.matrix[i][i] );
    v = ~c*u*s;
  }else
  {
    a = ~c*c;
    a.eig( n_eig, s, v );
    for( i=0; i<s.col; i++ )
      if( s.matrix[i][i]==0 )
        memset( v.matrix[i], 0, v.row*sizeof(float) );
  }

  return v;
}

void GMrz::svds( int n_eig, GMrz &u, GMrz &s, GMrz &v ) const
{
  GMrz a, ss;
  const GMrz &c = *this;
  int i;

  if( row<col )
  {
    a = c*~c;
    a.eig( n_eig, ss, u );

    for( i=0; i<ss.col; i++ )
      if( ss.matrix[i][i]<FLT_EPSILON )
        ss.matrix[i][i] = 0;
      
    s = ss;
    for( i=0; i<s.col; i++ )
      if( s.matrix[i][i]>0 )
        s.matrix[i][i] = sqrtf( s.matrix[i][i] );

    for( i=0; i<ss.col; i++ )
      if( ss.matrix[i][i]>0 )
        ss.matrix[i][i] = 1/sqrtf( ss.matrix[i][i] );

    v = ~c*u*ss;
  }else
  {
    a = ~c*c;
    a.eig( n_eig, ss, v );

    for( i=0; i<ss.col; i++ )
      if( ss.matrix[i][i]<FLT_EPSILON )
        ss.matrix[i][i] = 0;
      
    s = ss;
    for( i=0; i<s.col; i++ )
      if( s.matrix[i][i]>0 )
        s.matrix[i][i] = sqrtf( s.matrix[i][i] );

    for( i=0; i<ss.col; i++ )
      if( ss.matrix[i][i]>0 )
        ss.matrix[i][i] = 1/sqrtf( ss.matrix[i][i] );

    u = c*v*ss;
  }
}
