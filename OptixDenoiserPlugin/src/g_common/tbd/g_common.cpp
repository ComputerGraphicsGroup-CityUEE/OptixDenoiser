#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include <immintrin.h>


#include "g_common.h"


//combination, n(n-1)...(n-r+1)/r(r-1)...1
//c(n,r) = c(n,n-r)
double nCr(int n, int r)
{
  double ans = 1;
  int il = n-r;
  for(int i=1, j=r+1; i<=il; i++, j++)
    ans *= ((double)j)/i;
  return ans;
}

int g_quo( int x, int n )
{
  return (x + n-1)/n;
}


// mR is taken to be OpenGL version of 4x4 Matrix
// that is assuming row vector.
//
// Singularity at attitude = +- PI/2
//
// heading : [-pi  , pi  ]
// attitude: [-pi/2, pi/2]
// bank    : [-pi  , pi  ]
//
// http://martinb.com/maths/geometry/rotations/euler/index.htm
//
void Matrix2Euler( const float *mR, float *heading, float *attitude, float *bank )
{
  if( mR[1] > 0.998 )   // singularity at north pole
  {
    *heading = atan2f( mR[8], mR[10]);
    *attitude = G_PI/2;
    *bank = 0;
  }else if( mR[1] < -0.998 )   // singularity at south pole
  {
    *heading = atan2f( mR[8], mR[10]);
    *attitude = -G_PI/2;
    *bank = 0;
  }else
  {
    *heading = atan2f( -mR[2], mR[0] );
    *bank = atan2f( -mR[9], mR[5] );
    *attitude = asinf( mR[1] );
 }
}

// mR is taken to be OpenGL version of 4x4 Matrix, column major matrix
// that is assuming row vector.
// heading: x; attitude: z; bank: y.
// mR = R(y)*R(z)*R(x)
void Euler2Matrix( float *mR, float heading, float attitude, float bank )
{
  float ch = cosf(heading);    
  float sh = sinf(heading);    
  float ca = cosf(attitude);    
  float sa = sinf(attitude);    
  float cb = cosf(bank);    
  float sb = sinf(bank);

  mR[0] = ch * ca;
  mR[4] = sh*sb - ch*sa*cb;
  mR[8] = ch*sa*sb + sh*cb;
  mR[12] = 0;

  mR[1] = sa;
  mR[5] = ca*cb;
  mR[9] = -ca*sb;
  mR[13] = 0;

  mR[2] = -sh*ca;
  mR[6] = sh*sa*cb + ch*sb;
  mR[10] = -sh*sa*sb + ch*cb;
  mR[14] = 0;

  mR[3] = 0;
  mR[7] = 0;
  mR[11] = 0;
  mR[15] = 1;
}

 void AxisAngle2Matrix( float *m, float angle, float x, float y, float z  )
 {
   float c, s, t;
   
   c = cosf( angle );
   s = sinf( angle );
   t = 1 - c;
   
   memset( m, 0, 16*sizeof(float) );
   
   m[0]  = c + x*x*t;
   m[1]  = x*y*t + z*s;
   m[2]  = x*z*t - y*s;
   
   m[4]  = x*y*t - z*s;
   m[5]  = c + y*y*t;
   m[6]  = y*z*t + x*s;
   
   m[8]  = x*z*t + y*s;
   m[9]  = y*z*t - x*s;
   m[10] = c + z*z*t;
   
   m[15] = 1;

   // float3 axis;
   // float angle;
   // axis = float3( ay.z-az.y, az.x-ax.z, ax.y-ay.x )/2;
   // angle = asin( clamp( length(axis),0,1) )/G_PI;
   // if( ax.x+ay.y+az.z-1 < 0 )
   //   angle = 1-angle;
}

GPath parse_spath( const char *spath0 )
{
  GPath gp;

  char spath[256];
  strcpy( spath, spath0 );
  int slen = (int) strlen( spath );
  int i;
  for( i=0; i<slen; i++ )
    if( spath[i] == '\\' ) 
      spath[i]='/';


  const char *t_fname = strrchr( spath, '/' );

  int dlen, flen;
  if( t_fname )
  {
    dlen = int(strlen(spath) - strlen(t_fname)+1);
    t_fname++;
  }else
  {
    dlen = 0;
    t_fname = spath;
  }
  memcpy( gp.dname, spath, dlen*sizeof(char) );
  gp.dname[dlen] = '\0';

  if( slen == dlen )
  {
    gp.fname[0] = '\0';
    gp.ename[0] = '\0';
    return gp;
  }

  const char *t_ename = strrchr( t_fname, '.' );

  if( t_ename )
  {
    flen = int(strlen(t_fname) - strlen(t_ename));
    t_ename++;

    if( strcmp( t_ename, "" ) == 0 )
    {
      printf( "[Error] : parse path error, invalid path \"%s\"  .\n", spath );
      exit(-1);
    }
  }else
  {
    flen = int( strlen(t_fname) );
    t_ename = "";
  }

  strcpy( gp.ename, t_ename );

  memcpy( gp.fname, t_fname, flen*sizeof(char) );
  gp.fname[flen] = '\0';

  if( strcmp( gp.fname, "" ) == 0 )
  {
    printf( "[Error] : parse path error, invalid path \"%s\"  .\n", spath );
    exit(-1);
  }

  //if( strcmp(dname, "" )==0 )
  //  strcpy( dname, "./");

  return gp;
}

void gen_zmap( int *zmap, int n_dim )
{
  int *t_zmap = zmap;

  int s_zrow, t_zrow;
  int x=0, y=0;    //(x,y)=(0,0)
  int tx=1, ty=-1; //(tx,ty)=(0,0)

  for( s_zrow=0; s_zrow<n_dim; s_zrow++ )
  {
    for( t_zrow=0; t_zrow<s_zrow; t_zrow++, x+=tx, y+=ty )
      *t_zmap++ = y*n_dim + x;
      *t_zmap++ = y*n_dim + x;

    (s_zrow%2)? y+=1 : x+=1;
    tx=-tx; ty=-ty;
  }

  for( s_zrow=n_dim-2, x+=tx, y+=ty; s_zrow>=0; s_zrow-- )
  {
    for( t_zrow=0; t_zrow<s_zrow; t_zrow++, x+=tx, y+=ty )
      *t_zmap++ = y*n_dim + x;
      *t_zmap++ = y*n_dim + x;

    (s_zrow%2)? x+=1 : y+=1;
    tx=-tx; ty=-ty;
  }

}

float g_psnr( float mse, float peak )
{
  return 10*logf( peak*peak/mse )/logf(10);
}

float g_fps( void (*func)(void), int n_frame )
{
  clock_t start, finish;
  int i;
  float fps;

  if( n_frame==0 )
  {
    for( start=finish=clock(), n_frame=0; finish-start<CLOCKS_PER_SEC*.1; finish=clock(), n_frame++ )
    {
      func();
    }
    n_frame = g_max( 4, n_frame*20 );
  }

  printf( "Performing benchmark for %i frames, please wait... ", n_frame );
    start = clock();
    for( i=0; i<n_frame; i++ )
    {
      func();
    }
    finish = clock();
    printf( "done\n" );

  fps = float(n_frame)/(finish-start)*CLOCKS_PER_SEC;
  return fps;
}


float Secant( float x0, float x1, float (*f)(float) )
{
  if( x0==x1 )
    return x1;
  
  float x2 = x1 - f(x1)*(x1-x0)/(f(x1)-f(x0));
  return Secant( x1, x2, f );
}

float Bisect( float x0, float x1, float (*f)(float) )
{
  float y, x;

  int s0, s1;
  s0 = G_SIGN(f(x0));
  s1 = G_SIGN(f(x1));
  if( s0 == s1 )
  {
    printf( "[Error] Bisect, both end points are of the same sign %f %f.\n", x0, x1 );
    exit(-1);
  }

  while( fabs(x0-x1)>.00001 )
  {
    x = (x0+x1)/2;
    y = f(x);

    if( s0 == G_SIGN(y) )
      x0 = x;
    else
      x1 = x;
  }

  return x;
}

void swapbyte( void *src, int n )
{
  int i,j,n2;
  unsigned char a;
  unsigned char *s0;
  s0 = (unsigned char*)src;

  n2 = n/2;
  for( i=0, j=n-1; i<n2; i++, j-- )
  {
    a = s0[i];
    s0[i] = s0[j];
    s0[j] = a;
  }
}

void* freadall( const char *spath )
{
  if( !fexist(spath) )
  {
    printf( "[Error] : freadall(), \"%s\" not found.\n", spath );
    exit(-1);
  }

  unsigned char *dat;
  int pend;

  FILE *f0 = fopen( spath, "rb" );
    fseek( f0, 0, SEEK_END );
    pend = ftell(f0);
    fseek( f0, 0, SEEK_SET );
    dat = (unsigned char*) malloc( (pend+1)*sizeof(unsigned char) );
    fread( dat, sizeof(unsigned char), pend, f0 );
    dat[pend]=0;
  fclose(f0);

  return dat;
}


void bubble_sort( const float *val, int *idx, int n, bool ascending )
{
  int i,j;

  for( i=0; i<n; i++ )
    idx[i] = i;

  for( i=0; i<n-1; i++ )
    for( j=0; j<n-1-i; j++ )
      if( val[idx[j+1]] < val[idx[j]] )
        g_swap( idx[j], idx[j+1] );

  if( !ascending )
    for( i=0; i<n/2; i++ )
      g_swap( idx[i], idx[n-1-i] );
}

void rank_sort( const float *val, int *idx, int n, bool ascending )
{
  int *rank;
    rank = (int*) malloc( n * sizeof(int) );
    memset(rank, 0, n * sizeof(int) );

  int i, j;

  for( j=0; j<n; j++ )
    for( i=0; i<n; i++ )
      rank[j] += ( val[j] > val[i] );

  for( i=0; i<n; i++ )
    idx[rank[i]] = i;

  if( !ascending )
    for( i=0; i<n/2; i++ )
      g_swap( idx[i], idx[n-1-i] );

  free(rank);
}


void partial_quicksort( const float *val, int *idxm, int n, int m, bool ascending )
{
  if(n==0 || m==0)
    return;

  int i, *idx;

  idx = (int*) malloc( n*sizeof(int) );

  for( i=0; i<n; i++ )
    idx[i] = i;

  {
    GStack<int> gs;
    int low, high, l, h;
    float sv;

    gs.push( 0, n-1 );
    while( gs.ns )
    {
      gs.pop( high, low );
      sv = val[idx[high]];
      for( l=low, h=low; h<high; h++ )
        if( (val[idx[h]] <= sv) == ascending )
          g_swap( idx[l++], idx[h] );
        g_swap( idx[l], idx[h] );

      if( low < l-1 )
        gs.push( low, l-1  );
      if( l+1 < high && l+1 < m )
        gs.push( l+1, high );
    }
  }

  for( i=0; i<m; i++ )
    idxm[i] = idx[i];

  free(idx);
}

void* malloc2d( int w, int h, int size )
{
  int j;
  void **a = (void**) malloc( h*sizeof(void*) + w*h*size );
  for( j=0; j<h; j++ )
    a[j] = ((char *)(a+h)) + j*w*size;
  return a;

  //int j;
  //void **a = (void**) malloc( h*sizeof(void*) );
  //for( j=0; j<h; j++ )
  //  a[j] = malloc( w*size );
  //return a;
}

void* malloc3d( int w, int h, int d, int size )
{
  int j, k;
  void ***a = (void***) malloc( d*sizeof(void**) + h*d*sizeof(void*) + w*h*d*size );
  for( k=0; k<d; k++ )
    a[k] = ((void**)(a+d)) + k*h;
  for( k=0; k<d; k++ )
    for( j=0; j<h; j++ )
      a[k][j] = ((char*)(a+d+h*d)) + (k*h+j)*w*size;
  return a;
}

GPermutation::GPermutation(){ memset( this, 0, sizeof(GPermutation) ); } 
GPermutation::~GPermutation(){ SAFE_FREE(buf); SAFE_FREE(p); } 
void GPermutation::load( int *dat, int n_dat ) 
{ 
  SAFE_FREE(buf); 
  SAFE_FREE(p); 
  n = n_dat; 
  buf = (int*) malloc( n * sizeof(int) ); 
  p = (int*) malloc( n * sizeof(int) ); 
  
  for( i=0; i<n; i++ ) 
  { 
    buf[i] = dat[i]; 
    p[i] = n-i; 
  } 
  i = n-1; 
} 

bool GPermutation::next() 
{ 
  if( i>0 ) 
  { 
    p[i]--;  i--;  j = n-1; 
    do{  g_swap(buf[i], buf[j]);  j--;  i++;  }while( j>i ); 
    i = n-1; 
    while( !p[i] ){  p[i] = n - i;  i--;  } 
    return true; 
  }else 
    return false; 
} 

char* replace_char( char *str, char a, char b )
{
  int i, l;
  for( i=0, l=int(strlen( str )); i<l; i++ )
    if( str[i] == a ) 
      str[i] = b;
  return str;
}

bool getbit( unsigned char c, int i )
{
  // i==[0..7], return bit 0 to bit 7

  if ( i>7 || i<0 )
  {
    printf( "[Error] getbit(): i=%i\n", i );
    exit(-1);
  }

  return (c>>i&1)>0;
}

void setbit( unsigned char &c, int i, bool value )
{

// i==[0..7], return bit 0 to bit 7
  if ( i>7 || i<0 )
  {
    printf( "[Error] setbit(): i=%i\n", i );
    exit(-1);
  }
  c = c - (c & 1<<i);
  if( value==true )
    c += 1<<i;
}

void random_index( int *idx, int n )
{
  int i, j, *tmp, n_tmp, pos;

  tmp = (int*) malloc( n*sizeof(int) );

  n_tmp = n;
  for( i=0; i<n; i++ )
    tmp[i] = i;

  for( j=0; j<n; j++ )
  {
    pos = ( rand()*(RAND_MAX+1)+rand() ) % n_tmp;
    idx[j] = tmp[pos];

    n_tmp = n_tmp-1;
    for( i=pos; i<n_tmp; i++ )
      tmp[i] = tmp[i+1];
  }

  free(tmp);
}

void random_index( int *idx, int n, int m )
{
  int i, j, *tmp, n_tmp, pos;

  tmp = (int*) malloc( n*sizeof(int) );

  n_tmp = n;
  for( i=0; i<n; i++ )
    tmp[i] = i;

  for( j=0; j<m; j++ )
  {
    pos = ( rand()*(RAND_MAX+1)+rand() ) % n_tmp;
    idx[j] = tmp[pos];

    n_tmp = n_tmp-1;
    for( i=pos; i<n_tmp; i++ )
      tmp[i] = tmp[i+1];
  }

  free(tmp);
}

bool g_ferr( const float &f )
{
  if( (*((unsigned int*)&f) & 0x7f800000) == 0x7f800000 )
    return true;
  else
    return false;
}

int ftoi( float x, int xmax )
{
  return g_clamp( g_round(x*xmax), 0, xmax );
}

int two_power_ubound(int n0)
{
  unsigned int n1 = n0<0?-n0:n0;
  if( (n1&(n1-1))==0 )
    return n1;
  else
    return 1 << sizeof(n1)*8 - _lzcnt_u32(n1);
}

#include <sys/stat.h>  
bool dir_exist( const char *spath )
{
  struct stat st;
  return stat(spath, &st) == 0;
}

int fsize( const char *spath )
{
  if( !fexist(spath) )
  {
    printf( "[Error] : freadall(), \"%s\" not found.\n", spath );
    exit(-1);
  }
  int pend;
  FILE *f0 = fopen( spath, "rb" );
    fseek( f0, 0, SEEK_END );
    pend = ftell(f0);
  fclose(f0);
  return pend;
}

bool fexist( const char *spath )
{
  FILE *f0 = fopen( spath, "rb" );
  if( f0==NULL )
    return false;
  fclose(f0);
  return true;
}


size_t fsize( const wchar_t *spath )
{
  if( !fexist(spath) )
  {
    printf( L"[Error] : freadall(), \"%ws\" not found.\n", spath );
    exit(-1);
  }
  size_t pend;
  FILE *f0 = _wfopen( spath, L"rb" );
    fseek( f0, 0, SEEK_END );
    pend = ftell(f0);
  fclose(f0);
  return pend;
}

bool fexist( const wchar_t *spath )
{
  FILE *f0 = _wfopen( spath, L"rb" );
  if( f0==NULL )
    return false;
  fclose(f0);
  return true;
}
void* freadall( const wchar_t *spath )
{
  if( !fexist(spath) )
  {
    printf( L"[Error] : freadall(), \"%s\" not found.\n", spath );
    exit(-1);
  }

  unsigned char *dat;
  int pend;

  FILE *f0 = _wfopen( spath, L"rb" );
    fseek( f0, 0, SEEK_END );
    pend = ftell(f0);
    fseek( f0, 0, SEEK_SET );
    dat = (unsigned char*) malloc( (pend+1)*sizeof(unsigned char) );
    fread( dat, sizeof(unsigned char), pend, f0 );
    dat[pend]=0;
  fclose(f0);

  return dat;
}

wGPath wparse_spath( std::wstring spath0 )
{
  wGPath gp;
  std::wstring spath;

  spath = spath0;
  int slen = spath.length();  
  int i;
  for( i=0; i<slen; i++ )
    if( spath[i] == '\\' ) 
      spath[i]='/';

  const wchar_t *t_fname = wcsrchr( spath.c_str(), L'/' );

  int dlen, flen;
  if( t_fname )
  {
    dlen = int( spath.length() - wcslen(t_fname) +1 );
    t_fname++;
  }else
  {
    dlen = 0;
    t_fname = spath.c_str();
  }
  memcpy( gp.dname, spath.c_str(), dlen*sizeof(wchar_t) );
  gp.dname[dlen] = '\0';

  if( slen == dlen )
  {
    gp.fname[0] = '\0';
    gp.ename[0] = '\0';
    return gp;
  }

  const wchar_t *t_ename = wcsrchr( t_fname, L'.' );

  if( t_ename )
  {
    flen = int(wcslen(t_fname) - wcslen(t_ename));
    t_ename++;

    if( wcscmp( t_ename, L"" ) == 0 )
    {
      printf( L"[Error] : parse path error, invalid path \"%s\"  .\n", spath );
      exit(-1);
    }
  }else
  {
    printf( L"%s\n", t_fname );
    flen = int( wcslen(t_fname) );
    t_ename = L"";
  }

  wcscpy( gp.ename, t_ename );

  memcpy( gp.fname, t_fname, flen*sizeof(wchar_t) );
  gp.fname[flen] = '\0';

  if( wcscmp( gp.fname, L"" ) == 0 )
  {
    printf( L"[Error] : parse path error, invalid path \"%s\"  .\n", spath );
    exit(-1);
  }

  return gp;
}








#define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')

float fast_atof32( const char *p )
{
  return (float)fast_atof(p);
}

double fast_atof( const char *p )
{
    int frac;
    double sign, value, scale;

    // Skip leading white space, if any.

    while (white_space(*p) ) {
        p += 1;
    }

    // Get sign, if any.

    sign = 1.0;
    if (*p == '-') {
        sign = -1.0;
        p += 1;

    } else if (*p == '+') {
        p += 1;
    }

    // Get digits before decimal point or exponent, if any.

    for (value = 0.0; valid_digit(*p); p += 1) {
        value = value * 10.0 + (*p - '0');
    }

    // Get digits after decimal point, if any.

    if (*p == '.') {
        double pow10 = 10.0;
        p += 1;
        while (valid_digit(*p)) {
            value += (*p - '0') / pow10;
            pow10 *= 10.0;
            p += 1;
        }
    }

    // Handle exponent, if any.

    frac = 0;
    scale = 1.0;
    if ((*p == 'e') || (*p == 'E')) {
        unsigned int expon;

        // Get sign of exponent, if any.

        p += 1;
        if (*p == '-') {
            frac = 1;
            p += 1;

        } else if (*p == '+') {
            p += 1;
        }

        // Get digits of exponent, if any.

        for (expon = 0; valid_digit(*p); p += 1) {
            expon = expon * 10 + (*p - '0');
        }
        if (expon > 308) expon = 308;

        // Calculate scaling factor.

        while (expon >= 50) { scale *= 1E50; expon -= 50; }
        while (expon >=  8) { scale *= 1E8;  expon -=  8; }
        while (expon >   0) { scale *= 10.0; expon -=  1; }
    }

    // Return signed and scaled floating point result.

    return sign * (frac ? (value / scale) : (value * scale));
}

void extract_double( double x, unsigned __int64 &sign_bit, unsigned __int64 &exponent, unsigned __int64 &fraction )
{
  unsigned __int64 val = (*((unsigned __int64*)&x));
  sign_bit  = val & 0x8000000000000000uL;
  sign_bit = (sign_bit>>63);
  exponent = (val & 0x7FF0000000000000UL)>>52;
  fraction = val & 0x000FFFFFFFFFFFFFuL;
}

double get_double( unsigned __int64 sign_bit, unsigned __int64 exponent, unsigned __int64 fraction )
{
  unsigned __int64 val = (sign_bit<<63) | (exponent<<52) | fraction;
  return *((double*)&val);
}

