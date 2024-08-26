#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>
#include <float.h>



#include "las2.h"
extern "C" void svds(float *U, float *S, float *V, int nRow, int nCol, int K);

//#include "lpsvd.h"

#include "g_vector.h"


#define SAFE_FREE(p)  { if(p){ free(p);  p=NULL;} }
#define SAFE_DELETE(p){ if(p){ delete(p); p=NULL;} }
#include "g_mrx.h"

char *GMrx::TXT_FORMAT=NULL;


GMrx::GMrx()
{
  memset( this, 0, sizeof(GMrx) );
}


GMrx::GMrx( int row, int col )
{
  this->row = row;
  this->col = col;
  m = NULL;
  matrix = NULL;
  mem_alloc();
}

GMrx::GMrx( int row, int col, float a )
{
  this->row = row;
  this->col = col;
  m = NULL;
  matrix = NULL;
  mem_alloc();

  float *tM = m;
  for( int j=0; j<row; j++ )
    for( int i=0; i<col; i++, tM++ )
      *tM = a; 
}

void GMrx::mem_alloc()
{
  SAFE_FREE(m);
  SAFE_FREE(matrix);
  m = (float*) malloc( row*col * sizeof(float) );
  matrix = (float**) malloc( row * sizeof(float*) );

  for( int j=0; j<row; j++ )
    matrix[j] = &m[j*col];
}

void GMrx::mem_set(
  const float *pSrc, int src_width, int src_height,
  int x, int y, int m_width, int m_height
){
  this->row = m_height;
  this->col = m_width;
  mem_alloc();

  const float *tSrc = &pSrc[ y*src_width + x ];
  float *tM = this->m;
  
  for( int j=0; j<m_height; j++, tSrc+=src_width, tM+=m_width )
    memcpy( tM, tSrc, m_width*sizeof(float) );

}

void GMrx::load( const float *pSrc, int src_width, int src_height )
{
  this->row = src_height;
  this->col = src_width;
  mem_alloc();

  memcpy( m, pSrc, src_width*src_height * sizeof(float) );
}

void GMrx::load( const double *src, int w, int h )
{
  load( w, h );

  int i,l;
  l = row * col;

  float  *tm   = m;
  const double *tsrc = src;
  for( i=0; i<l; i++, tm++, tsrc++ )
    *tm = (float) *tsrc;

}

void GMrx::load( int src_width, int src_height )
{
  this->row = src_height;
  this->col = src_width;
  mem_alloc();
  memset( m, 0, col*row * sizeof(float) );
}

void GMrx::transpose()
{
  transpose(this);
}
    
void GMrx::transpose( const GMrx *A )
{
  if( this==A )
  {
    GMrx tmpA;
    tmpA.clone(this);
    this->transpose(&tmpA);
    return;
  }

  col = A->row;
  row = A->col;
  mem_alloc();

  if( A->row==1 || A->col==1 )
  {
    memcpy( m, A->m, A->row*A->col * sizeof(float) );
  }else
  {
    int i,j;
    const float *tA;
    float *tM, *tM0;

    for( j=0, tA=A->m, tM=m; j<A->row; j++, tM++ )
      for(  i=0, tM0=tM; i<A->col; i++, tA++, tM0+=A->row )
        *tM0 = *tA;
  }
}

void GMrx::clone( const GMrx *A )
{
  if( A==this ) return;
  col = A->col;
  row = A->row;
  mem_alloc();

  memcpy( m, A->m, A->row*A->col * sizeof(float) );
  //memcpy( matrix, A->matrix, A->row * sizeof(float*) );
}

void GMrx::mul( const GMrx *A, const GMrx *B )
{
  if( A->col!=B->row )
  {
    printf( "[ERROR] Matrix Mul error...\n" );
    A->size();
    B->size();
    exit(0);
  }

  if( this==A || this==B )
    if( this==A && this==B )
    {
      GMrx tmpA, tmpB;
      tmpA.clone(A);
      tmpB.clone(B);
      this->mul( &tmpA, &tmpB );
      return;
    }else if( this==A )
    {
      GMrx tmpA;
      tmpA.clone(A);
      this->mul( &tmpA, B );
      return;
    }else if( this==B )
    {
      GMrx tmpB;
      tmpB.clone(B);
      this->mul( A, &tmpB );
      return;
    }

  int k_len =  A->col;  // use B->col if you wanted to...

  if( k_len>8 )
  {
    GMrx *Bt = new GMrx();
    Bt->transpose(B);
    AxBt( A, Bt );
    SAFE_DELETE(Bt);
  }else
  {
    col = B->col;
    row = A->row;
    mem_alloc();

    //int k_len = A->col;  // use B->col if you wanted to...
    memset( m, 0, col*row * sizeof(float) );

    float *tM;
    const float *tA, *tA0, *tB, *tB0;
    int i,j,k;

    for( j=0, tA=A->m, tM=m; j<row; j++, tA+=A->col )
      for( i=0, tB=B->m; i<col; i++, tM++, tB++ )
        for( k=0, tA0=tA, tB0 = tB; k<k_len; k++, tA0++, tB0+=B->col )
          *tM += *tA0 * *tB0;
    }
}

void GMrx::AxBt( const GMrx *A, const GMrx *B )
{
  if( A->col!=B->col )
  {
    printf( "[ERROR] Matrix Mul error...\n" );
    exit(0);
  }

  if( this==A || this==B )
    if( this==A && this==B )
    {
      GMrx tmpA, tmpB;
      tmpA.clone(A);
      tmpB.clone(B);
      this->AxBt( &tmpA, &tmpB );
      return;
    }else if( this==A )
    {
      GMrx tmpA;
      tmpA.clone(A);
      this->AxBt( &tmpA, B );
      return;
    }else if( this==B )
    {
      GMrx tmpB;
      tmpB.clone(B);
      this->AxBt( A, &tmpB );
      return;
    }

  col = B->row;
  row = A->row;
  mem_alloc();

  int k_len = A->col;  // use B->col if you wanted to...
  memset( m, 0, col*row * sizeof(float) );

  float *tM;
  const float *tA, *tA0, *tB, *tB0;
  int i,j,k;

  for( j=0, tA=A->m, tM=m; j<row; j++, tA+=A->col )
    for( i=0, tB=B->m; i<col; i++, tM++, tB+=B->col )
      for( k=0, tA0=tA, tB0 = tB; k<k_len; k++, tA0++, tB0++ )
        *tM += *tA0 * *tB0;
}

GMrx::~GMrx()
{
  SAFE_FREE(m);
  SAFE_FREE(matrix);
}

void GMrx::xsvds( GMrx *U, float *S, GMrx *V, int K ) const
{

  int i,j;
  float *tM;
  float score = 0;
  for( j=0, tM=m; j<row; j++ )
    for( i=0; i<col; i++, tM++ )
      score += fabsf(*tM);

  if( score<FLT_EPSILON )
  {
    U->load( K, row );
    memset( S, 0, K * sizeof(float) );
    U->load( K, col );
    return;
  }

  float *U0 = (float*) malloc( row*col * sizeof(float) );
  float *S0 = (float*) malloc( K * sizeof(float) );
  float *V0 = (float*) malloc( K*col * sizeof(float) );

    memcpy( U0, m, row*col * sizeof(float) );
    svds( U0, S0, V0, row, col, K );

  U->mem_set( U0, col, row, 0,0, K,row );
  memcpy( S, S0, K*sizeof(float) );
  V->load( V0, K,col );

  SAFE_FREE(U0);
  SAFE_FREE(S0);
  SAFE_FREE(V0);



}

void GMrx::inverse( const GMrx *A )
{
  int i,j;

  float *U = (float*) malloc( A->row*A->col * sizeof(float) );
  float *V = (float*) malloc( A->col*A->col * sizeof(float) );
  float *S = (float*) malloc( A->col * sizeof(float) );


#ifdef LPSVD_H
  lpsvd( A->m, U, S, V, A->row, A->col );
#else
  memcpy( U, A->m, A->row*A->col * sizeof(float) );
  svds( U, S, V, A->row, A->col, A->col );
#endif


  float *tV = V;
  for( j=0; j<A->col; j++ )
    for( i=0; i<A->col; i++, tV++ )
      *tV *=1 / S[i];

  GMrx gU; gU.load( U, A->col, A->row );
  GMrx gVS; gVS.load( V, A->col, A->col );

  this->AxBt( &gVS, &gU );

  SAFE_FREE(U);
  SAFE_FREE(S);
  SAFE_FREE(V);
}

void GMrx::save( FILE *f0, const char *mode ) const
{
  if( strcmp( mode, "txt" )==0 )
  {
    fprintf( f0, "G0 txt\n%i %i\n", col, row );

    float *tM = m;
    for( int j=0; j<row; j++ )
    {
      for( int i=0; i<col; i++, tM++ )
      {
        if( TXT_FORMAT==NULL )
          fprintf( f0, "%8.5f ", *tM );
        else
          fprintf( f0, TXT_FORMAT, *tM );
      }
      fprintf( f0, "\n" );
    }
  }else if( strcmp( mode, "bin" )==0)
  {
    fprintf( f0, "G0 bin\n%i %i\n", col, row );
    fwrite( m, sizeof(float), col*row, f0 );
  }else
  {
    printf( "[Error] : GMrx::Save, unsupported mode %s.\n", mode );
    exit(-1);
  }
}

bool GMrx::_load( FILE *f0, const char *mode )
{
  char str[256]="";
  char identify_str[256], type_str[256];

  
  fgets( str, 256, f0 );
  sscanf(str, "%s %s", &identify_str, &type_str);

  if( mode!=NULL )
    strcpy( type_str, mode );

  int count = 0;

  if( strcmp( "G0", identify_str )==0 )
  {
    if( strcmp( "txt", type_str )==0 )
    {
      fgets( str, 256, f0 );
      sscanf(str, "%i %i", &col, &row);
      mem_alloc();

      float *tM = m;
      for( int i=0; i<col*row; i++, tM++ )
        count += fscanf( f0, "%f", tM );
      fgets( str, 256, f0 );

      if( count==col*row )
        return true;

    }else if( strcmp( "bin", type_str )==0 )
    {
      fgets( str, 256, f0 );
      sscanf(str, "%i %i", &col, &row);
      mem_alloc();

      count=fread( m, sizeof(float), col*row, f0);

      if( count==col*row )
        return true;
    }
  }else if( strcmp( "Pf", identify_str )==0 )
  {
      fgets( str, 256, f0 );
      sscanf(str, "%i %i", &col, &row);
      fgets( str, 256, f0 );
      mem_alloc();

      count=fread( m, sizeof(float), col*row, f0);

      flip_vertical();

      if( count==col*row )
        return true;
  }


  printf( "[Error] : mrx file maybe corrupted.\n" );
  exit(-1);
}

void GMrx::add( const GMrx *A, const GMrx *B )
{
  if( A->col!=B->col || A->row!=B->row)
  {
    printf( "[ERROR] Matrix add error...\n" );
    exit(0);
  }

  if( this!=A && this!=B )
  {
    col = A->col;
    row = B->row;
    mem_alloc();
  }
  
  float *tM;
  const float *tA, *tB;

  int t, tlen = row*col;
  for( t=0, tA=A->m, tB=B->m, tM=m; t<tlen; t++, tA++, tB++, tM++ )
      *tM = *tA + *tB;
}

void GMrx::sub( const GMrx *A, const GMrx *B )
{
  if( A->col!=B->col || A->row!=B->row)
  {
    printf( "[ERROR] Matrix add error...\n" );
    exit(0);
  }

  if( this!=A && this!=B )
  {
    col = A->col;
    row = B->row;
    mem_alloc();
  }

  float *tM;
  const float *tA, *tB;

  int t, tlen = row*col;
  for( t=0, tA=A->m, tB=B->m, tM=m; t<tlen; t++, tA++, tB++, tM++ )
      *tM = *tA - *tB;
}

void GMrx::scalar_mul( float a )
{
  float *tM;

  int t, tlen = row*col;
  for( t=0, tM=m; t<tlen; t++, tM++ )
    *tM *= a;
}

void GMrx::scalar_add( float a )
{
  float *tm;

  int t, tlen = row*col;
  for( t=0, tm=m; t<tlen; t++, tm++ )
    *tm += a;
}

void GMrx::scalar_assign( float a )
{
  float *tm;

  int t, tlen = row*col;
  for( t=0, tm=m; t<tlen; t++, tm++ )
    *tm = a;
}


void GMrx::interleave( const GMrx *p_mrx, int num_of_mrx, float *pData )
{
  float *tData = pData;

  int tcol = p_mrx[0].col;
  int trow = p_mrx[0].row;

  float **t_m = (float**) malloc( num_of_mrx * sizeof(float*) );
  for( int mrx_idx=0; mrx_idx<num_of_mrx; mrx_idx++ )
  {
    if( tcol!=p_mrx[mrx_idx].col || trow!=p_mrx[mrx_idx].row  )
    {
      printf( "[Error] : interleave(), matrix dimension do not match.\n" );
      exit(0);
    }
    t_m[mrx_idx] = p_mrx[mrx_idx].m;
  }

  int t, tlen = trow*tcol;
  for( t=0; t<tlen; t++ )
    for( int mrx_idx=0; mrx_idx<num_of_mrx; mrx_idx++ )
      *tData++ = *(t_m[mrx_idx]++);

  free(t_m);
}

void GMrx::interleave( const GMrx *p_mrx, int num_of_mrx, GMrx *mrx )
{
  if( mrx->row != p_mrx[0].row || mrx->col !=p_mrx[0].col*num_of_mrx )
    mrx->load( p_mrx[0].col*num_of_mrx, p_mrx[0].row );
  interleave( p_mrx, num_of_mrx, mrx->m );
}

    
void GMrx::split( GMrx *p_mrx, int num_of_mrx, const float *pData, int tcol, int trow )
{
  const float *tData = pData;

  float **t_m = (float**) malloc( num_of_mrx * sizeof(float*) );
  for( int mrx_idx=0; mrx_idx<num_of_mrx; mrx_idx++ )
  {
    p_mrx[mrx_idx].col = tcol;
    p_mrx[mrx_idx].row = trow;
    p_mrx[mrx_idx].mem_alloc();
    t_m[mrx_idx] = p_mrx[mrx_idx].m;
  }

  int t, tlen = trow*tcol;
  for( t=0; t<tlen; t++ )
    for( int mrx_idx=0; mrx_idx<num_of_mrx; mrx_idx++ )
      *(t_m[mrx_idx]++) = *tData++;

  free(t_m);
}

void GMrx::getblk( float *pBlk, int x, int y, int blkw, int blkh ) const
{
  const float *tM = &m[ y*col + x ];
  float *tBlk = pBlk;
  
  for( int j=0; j<blkh; j++, tM+=col, tBlk+=blkw )
    memcpy( tBlk, tM, blkw*sizeof(float) );
}

void GMrx::setblk( const float *pBlk, int x, int y, int blkw, int blkh )
{
  float *tM = &m[ y*col + x ];
  const float *tBlk = pBlk;
  
  for( int j=0; j<blkh; j++, tM+=col, tBlk+=blkw )
    memcpy( tM, tBlk, blkw*sizeof(float) );
}

void GMrx::size() const
{
  printf( "[%i, %i]\n" , row, col );
}

float GMrx::sum() const
{
  float *tM;
  float res = 0;

  int t, tlen = row*col;
  for( t=0, tM=m; t<tlen; t++, tM++ )
      res += *tM;

  return res;
}

void GMrx::block_join( const GMrx *p_mrx, int blk_num_w, int blk_num_h, GMrx *mrx )
{
  int blk_num_total = blk_num_w*blk_num_h;
  int blk_w = p_mrx[0].col;
  int blk_h = p_mrx[0].row;

  int i,j;

  for( i=0; i<blk_num_total; i++ )
    if( p_mrx[i].col != blk_w || p_mrx[i].row != blk_h )
    {
      printf( "[Error] - GMrx join error, not all matrix to join are of the same row number and col number.\n" );
      exit(-1);
    }

  mrx->load( blk_num_w*blk_w, blk_num_h*blk_h );

  const GMrx *t_mrx = p_mrx;
  for( j=0; j<blk_num_h; j++ )
    for( i=0; i<blk_num_w; i++, t_mrx++ )
      mrx->setblk( t_mrx->m, i*blk_w, j*blk_h, blk_w, blk_h );
}

void GMrx::block_split( GMrx *p_mrx, int blk_num_w, int blk_num_h, const GMrx *mrx )
{
  if( mrx->col%blk_num_w != 0 || mrx->row%blk_num_h != 0 )
  {
    printf( "[Error] - GMrx split error, the row number and col number of matrix to be splited is not divisible by number of block specified in the coressponding dimension.\n" );
    exit(-1);
  }
  int i,j;

  int blk_num_total = blk_num_w*blk_num_h;
  int blk_w = mrx->col / blk_num_w;
  int blk_h = mrx->row / blk_num_h;

  float *tmp = (float*) malloc( blk_w*blk_h * sizeof(float) );

  GMrx *t_mrx = p_mrx;
  for( j=0; j<blk_num_h; j++ )
    for( i=0; i<blk_num_w; i++, t_mrx++ )
    {
      mrx->getblk( tmp, i*blk_w, j*blk_h, blk_w, blk_h );
      t_mrx->load( tmp, blk_w, blk_h );
    }

  free(tmp);
}

void GMrx::channel_split( GMrx *p_mrx, int n_blkw, int n_blkh, const GMrx *mrx )
{
  if( mrx->col%n_blkw != 0 || mrx->row%n_blkh != 0 )
  {
    printf( "[Error] - GMrx split error, the row number and col number of matrix to be splited is not divisible by number of block specified in the coressponding dimension.\n" );
    exit(-1);
  }
  int i,j;
  int u,v;

  int blkw = mrx->col / n_blkw;
  int blkh = mrx->row / n_blkh;

  for( v=0; v<blkh; v++ )
    for( u=0; u<blkw; u++ )
    {
      p_mrx[v*blkw+u].load( n_blkw, n_blkh );
      for( j=0; j<n_blkh; j++ )
        for( i=0; i<n_blkw; i++)
        {
          p_mrx[v*blkw+u].matrix[j][i] = mrx->matrix[ j*blkh+v ][ i*blkw+u ];
        }
    }


}

void GMrx::channel_join( const GMrx *p_mrx, int blkw, int blkh, GMrx *mrx )
{
  int i,j;
  int u,v;

  int n_blkw = p_mrx[0].col;
  int n_blkh = p_mrx[0].row;

  mrx->load( blkw*n_blkw, blkh*n_blkh );

  for( v=0; v<blkh; v++ )
    for( u=0; u<blkw; u++ )
    {
      for( j=0; j<n_blkh; j++ )
        for( i=0; i<n_blkw; i++)
        {
          mrx->matrix[ j*blkh+v ][ i*blkw+u ] = p_mrx[v*blkw+u].matrix[j][i];
        }
    }
}


void GMrx::split( GMrx *p_mrx, int num_of_mrx, const GMrx *mrx )
{
  if( mrx->col%num_of_mrx != 0 )
  {
    printf( "[Error] : inter_split error, number of column is not divisable by number to split.\n" );
    exit(-1);
  }
  split( p_mrx, num_of_mrx, mrx->m, mrx->col/num_of_mrx, mrx->row );
}

void GMrx::save( const char *spath, const char *mode ) const
{
  if( strcmp( mode, "txt" )==0 )
  {
    FILE *f0 = fopen( spath, "wt" );
    save( f0, mode );
    fclose(f0);
  }else if( strcmp( mode, "bin" )==0)
  {
    FILE *f0 = fopen( spath, "wb" );
    save( f0, mode );
    fclose(f0);
  }else if( strcmp( mode, "pfm" )==0)
  {
    save_pfm( spath );
  }else
  {
    printf( "[Error] : GMrx::save, unsupported mode %s.\n", mode );
    exit(-1);
  }
}

void GMrx::save_all( const char *spath, const GMrx *p_mrx, int num_of_mrx, const char *mode, const char *s_comment )
{
  if( strcmp( mode, "txt" )==0 )
  {
    FILE *f0 = fopen( spath, "wt" );
    fprintf( f0, "G1 %s %i\n", mode, num_of_mrx );

    if( s_comment!= NULL )
      fprintf( f0, "%s", s_comment );

    for( int i=0; i<num_of_mrx; i++ )
      p_mrx[i].save(f0, mode);
    fclose(f0);

  }else if( strcmp( mode, "bin" )==0)
  {

    FILE *f0 = fopen( spath, "wb" );
    fprintf( f0, "G1 %s %i\n", mode, num_of_mrx );

    if( s_comment!= NULL )
      fprintf( f0, "%s", s_comment );

    for( int i=0; i<num_of_mrx; i++ )
      p_mrx[i].save(f0, mode);
    fclose(f0);

  }else
  {
    printf( "[Error] : GMrx::Save, unsupported mode %s.\n", mode );
    exit(-1);
  }

}

void GMrx::load_all( const char *spath, GMrx *p_mrx, int num_of_mrx, const char *sid, const char *mode )
{
  char str[256]="", str_len;
  char str0[256];

  if( strcmp( sid, "G1" )==0 )
  {

    int mrx_count = 0;

    FILE *f0 = fopen( spath, "rb" );

      fgets( str, 256, f0 );

      fgets( str, 256, f0 );
      while( str[0]=='#' )
      {
        printf( str );
        fgets( str, 256, f0 );
      }

      sscanf( str, "%s", str0 );
      if( strcmp( str0, "PMD" )==0 )
      {
        printf( str );

        fgets( str, 256, f0 );
        sscanf( str, "%s", str0 );
        while( strcmp( str0, "/PMD" )!=0 )
        {
          printf( str );
          fgets( str, 256, f0 );
          sscanf( str, "%s", str0 );
        };
        printf( str );

      }else
      {
        str_len = strlen(str);
        fseek( f0, -str_len, SEEK_CUR );
      }

      for( int i=0; i<num_of_mrx; i++ )
        if( p_mrx[i]._load( f0, mode ) ) mrx_count++;

      fclose(f0);

      if( mrx_count != num_of_mrx )
      {
        printf( "[Error] : GMrx::load_all, the number of matrix read (%i) is not the same as specified (%i).\n", mrx_count, num_of_mrx );
        exit(-1);
      }

      return;

  }


  printf( "[Error] : GMrx::load_all, mrg file maybe corrupted.\n" );
  exit(-1);
}

void GMrx::load_all( const char *spath, GMrx **pp_mrx, int *num_of_mrx )
{
  char str[256]="", identify_str[256], type_str[256];

  FILE *f0 = fopen( spath, "rb" );
    if( f0==NULL )
    {
      printf( "[Error] : mrg file \"%s\" not found.\n", spath );
      exit(-1);
    }
    fgets( str, 256, f0 );
    sscanf( str, "%s %s %i", identify_str, type_str, num_of_mrx );
  fclose(f0);

  *pp_mrx = new GMrx[*num_of_mrx];
    
  load_all( spath, *pp_mrx, *num_of_mrx, identify_str, type_str );
}

void GMrx::load_all( const char *spath, GMrx *p_mrx, int num_of_mrx )
{
  char str[256]="", identify_str[256], type_str[256];

  FILE *f0 = fopen( spath, "rb" );
    if( f0==NULL )
    {
      printf( "[Error] : mrg file \"%s\" not found.\n", spath );
      exit(-1);
    }
    fgets( str, 256, f0 );

    int tmp_num_of_mrx;
    sscanf( str, "%s %s %i", identify_str, type_str, &tmp_num_of_mrx );

    if( tmp_num_of_mrx != num_of_mrx )
    {
      printf( "[Warning] : GMrx::load_all, the number of matrix specified in the header (%i) is not the same as specified (%i).\n", tmp_num_of_mrx, num_of_mrx );
      //exit(-1);
    }
    
    
    
    fclose(f0);

  load_all( spath, p_mrx, num_of_mrx, identify_str, type_str );
}

void GMrx::load( const char *spath )
{
  char str[256]="", identify_str[256], type_str[256];
  
  FILE *f0 = fopen( spath, "rb" );
    if( f0==NULL )
    {
      printf( "[Error] : mrx file \"%s\" not found.\n", spath );
      exit(-1);
    }
    fgets( str, 256, f0 );
    sscanf( str, "%s %s", identify_str, type_str );
  fclose(f0);

  if( strcmp( "G0", identify_str )==0 )
  {
    if( strcmp( "txt", type_str ) ==0 )
    {
      FILE *f0 = fopen( spath, "rb" );
        _load( f0, "txt" );
      fclose(f0);
      return;
    }else if( strcmp( "bin", type_str ) ==0 )
    {
      FILE *f0 = fopen( spath, "rb" );
        _load( f0, "bin" );
      fclose(f0);
      return;
    }
  }

  if( strcmp( "Pf", identify_str )==0 )
  {
    FILE *f0 = fopen( spath, "rb" );
      _load( f0, "pf" );
    fclose(f0);
    return;
  }

  printf( "[Error] : mrx file maybe corrupted.\n" );
  exit(-1);

}

void GMrx::print( const char *format ) const
{
  const char *default_format = "%8.5f ";
  if( format==NULL )
     format = default_format;
  
  float *tM = m;
  for( int j=0; j<row; j++ )
  {
    for( int i=0; i<col; i++, tM++ )
      printf( format, *tM );
    printf( "\n" );
  }
}

void GMrx::save_pfm( const char *spath ) const
{
  GMrx t;
    t.clone(this);
    t.flip_vertical();

  FILE *f0 = fopen( spath, "wb" );
    fprintf(f0, "Pf\n");
	fprintf(f0, "%i %i\n", col, row );
	fprintf(f0, "%f\n", -1 );
    fwrite( t.m, sizeof(float), col*row, f0 );
  fclose(f0);
}

float GMrx::det() const
{
  if( row == col )
    return det( matrix, row );
  else
  {
    printf( "[Error] : GMrx::det(), row and col is not the same.\n" ); 
    exit(0);
  }
}


float GMrx::det( float **mm, int order )
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

  delete(cf);
  return res;
}

void fill_partitions
(
  float t0, float t1,  // boundaries values in destination scale
  float s0, float s1,  // source and destination scale factors
  float *p, // weighting of each partitions
  int   *q, // quantized index of source
  int  &np  // number of partition
){
  float s01, s10;
    s01 = s1/s0;
    s10 = s0/s1;

  t0 *= s10;
  t1 *= s10;

  int f0, c0;
    f0 = (int)floor(t0);
    c0 = (int)ceil(t0);

  int f1, c1;
    f1 = (int)floor(t1);
    c1 = (int)ceil(t1);

  np=0;


  if( c0-t0 > FLT_EPSILON )
  {
    p[np] = c0-t0;
    q[np] = f0;
    np++;
  }

  int i;
  for( i=c0; i<f1; i++, np++ )
  {
    p[np] = 1;
    q[np] = i;
  }

  if( t1-f1 > FLT_EPSILON )
  {
    p[np] = t1-f1;
    q[np] = f1;
    np++;
  }

  float sp=0;
  for( i=0; i<np; i++ )
    sp+=p[i];
  for( i=0; i<np; i++ )
    p[i]/=sp;


}

void GMrx::scale( GMrx *a, int a_col, int a_row )
{
  a->load( a_col, a_row );

  int w0,h0, w1,h1;
    w0 = col;
    h0 = row;
    w1 = a->col;
    h1 = a->row;

  int max_px, max_py;
    max_px = w0 / w1  + 2;
    max_py = row / a->row  + 2;

  int    nx,  ny;
  float *px, *py;
  int   *qx, *qy;
    qx = new int[max_px]; 
    qy = new int[max_py];
    px = new float[max_px];
    py = new float[max_py];

  int i,j;
  for( j=0; j<h1; j++ )
  {
    fill_partitions( (float)j, (float)j+1, (float)h0, (float)h1, py, qy, ny );
    for( i=0; i<w1; i++ )
    {
      fill_partitions( (float)i, (float)i+1, (float)w0, (float)w1, px, qx, nx );

      int x,y;
      for( y=0; y<ny; y++ )
        for( x=0; x<nx; x++ )
          a->matrix[j][i] += py[y]*px[x]  *  matrix[ qy[y] ][ qx[x] ];
    }
  }

  delete[] px;
  delete[] qx;
  delete[] py;
  delete[] qy;

}

void GMrx::flip_vertical()
{
  float tmp;
  for( int j=0; j<row/2; j++ )
    for( int i=0; i<col; i++ )
    {
      int pos0 = j*col+i;
      int pos1 = (row-1-j)*col+i;
      tmp = m[pos0];
      m[pos0] = m[pos1];
      m[pos1] = tmp;
    }
}

void GMrx::flip_horizontal()
{
  float tmp;
  for( int j=0; j<row; j++ )
    for( int i=0; i<col/2; i++ )
    {
      int pos0 = j*col+i;
      int pos1 = j*col+(row-1-i);
      tmp = m[pos0];
      m[pos0] = m[pos1];
      m[pos1] = tmp;
    }
}



float GMrx::gmin() const
{
  float tmin, *tm;
    tmin = *m;
    tm = m;

  int t, tl;
    tl = col*row;

  for( t=0; t<tl; t++, tm++)
    if( tmin>*tm )
      tmin = *tm;

  return tmin;
}

float GMrx::gmax() const
{
  float tmax, *tm;
    tmax = *m;
    tm = m;

  int t, tl;
    tl = col*row;

  for( t=0; t<tl; t++, tm++)
    if( tmax<*tm )
      tmax = *tm;

  return tmax;
}

float GMrx::mean() const
{
  float tmean, *tm;
    tmean = 0;
    tm = m;

  int t, tl;
    tl = col*row;

  for( t=0; t<tl; t++, tm++)
	tmean += *tm;

  tmean/=tl;

  return tmean;
}


void GMrx::round()
{
  float *tm;
    tm = m;

  int t, tl;
    tl = col*row;

  for( t=0; t<tl; t++, tm++)
  {
    *tm = (float)((int)( *tm>0 ? *tm+.5 : *tm-.5 ));
  }
}

void GMrx::clamp(float a, float b)
{
  float *tm;
    tm = m;

  int t, tl;
    tl = col*row;

  for( t=0; t<tl; t++, tm++)
  {
    if( *tm > b )
      *tm=b;
    if( *tm < a )
      *tm=a;
  }
}


float GMrx::variance() const
{
  float tmean, tvar, *tm, tmp;
    tmean = this->mean();
    tvar = 0;
    tm = m;

  int t, tl;
    tl = col*row;

  for( t=0; t<tl; t++, tm++)
  {
    tmp = *tm-tmean;
	tvar += tmp*tmp/tl;
  }

  return tvar;
}

float GMrx::condition( float *s ) const
{
  if( row!=col )
  {
    printf( "[Error] GMrx::condition, the matrix is not square matrix\n" );
    exit(-1);
  }

  float *U = (float*) malloc( row*col * sizeof(float) );
  float *V = (float*) malloc( col*col * sizeof(float) );
  float *S = (float*) malloc( col * sizeof(float) );

#ifdef LPSVD_H
  lpsvd( m, U, S, V, row, col );
#else
  memcpy( U, m, row*col * sizeof(float) );
  svds( U, S, V, row, col, col );
#endif


  float s_max, s_min, condition_number;
    s_max = S[0];
    s_min = S[col-1];
    condition_number = s_max / s_min;

  memcpy( s, S, col * sizeof(float) );

  SAFE_FREE(U);
  SAFE_FREE(S);
  SAFE_FREE(V);

  return condition_number;
}

float GMrx::norm() const 
{
  float *U = (float*) malloc( row*col * sizeof(float) );
  float *V = (float*) malloc( col*col * sizeof(float) );
  float *S = (float*) malloc( col * sizeof(float) );

#ifdef LPSVD_H
  lpsvd( m, U, S, V, row, col );
#else
  memcpy( U, m, row*col * sizeof(float) );
  svds( U, S, V, row, col, col );
#endif

  float s_max = S[0];

  ////////////////////////////////////////////
  // The first element of SVDS should be
  // the largest value already, locating maximum
  // value like the following is just for debuging 
  // purpose only.
  for( int i=0; i<col; i++ )
    if( s_max<S[i] )
      s_max = S[i];
  //
  ////////////////////////////////////////////

  SAFE_FREE(U);
  SAFE_FREE(S);
  SAFE_FREE(V);

  return s_max;
}

#define ALPHA_GAUSSIAN  1.423f
#define ALPHA_LAPLACIAN 1.232f
void GMrx::bitalloc( const GMrx *p_mrx, int n_mrx, float bitrate, float *assignbit, float *stepsize )
{

  float minium_bitrate = 0;

  int i;
  
  int n_src, n_element;
    n_src = n_mrx;
  
    n_element = 0;
    for( i=0; i<n_src; i++ )
      n_element += p_mrx[i].col * p_mrx[i].row;
  
  
  float check_bit_rate;
  
  float *nf, *bf, *df, part3;
    nf = (float*) malloc( n_src*sizeof(float));
    bf = (float*) malloc( n_src*sizeof(float));
    df = (float*) malloc( n_src*sizeof(float));
  
  
  //---------- Subband normalized frequency factor ----------
    df[0] = ALPHA_GAUSSIAN;
    for( i=1; i<n_src; i++ )
      df[i] = ALPHA_LAPLACIAN;
  
    part3 = 0;
    for( i=0; i<n_src; i++ )
    {
      nf[i]  = ((float)p_mrx[i].col * p_mrx[i].row) / n_element;
      bf[i]  = df[i] * p_mrx[i].variance();
      part3 += 0.5f * nf[i] * logf(bf[i])/logf(2);
    }


  //---------- Bit Allocation ----------
    check_bit_rate = 0;
    for( i=0; i<n_src; i++ )
    {
      assignbit[i] = bitrate + 0.5f*logf(bf[i])/logf(2) - part3;

      if( assignbit[i]<minium_bitrate)
        assignbit[i]=minium_bitrate;

      check_bit_rate += assignbit[i];
      stepsize[i] = sqrtf( 12 * bf[i] * powf( 2,-2*assignbit[i] ) );
    }
  
  
  //printf( "check_bit_rate = %.4f\n", check_bit_rate );
  
  free(nf);
  free(bf);
  free(df);
  
}


void GMrx::identity( int n )
{
  load( n,n );

  for( int i=0; i<n; i++ )
    matrix[i][i] = 1;
}


void GMrx::toDouble( double *a ) const
{
  int i, l;
  l = col * row;

  double *ta = a;
  float  *tm = m;

  for( i=0; i<l; i++, ta++, tm++ )
    *ta = *tm;
}

void GMrx::toDoublet( double *a ) const
{
  double *ta = a;
  int i, j;

  for( i=0; i<col; i++ )
    for( j=0; j<row; j++, ta++ )
      *ta = matrix[j][i];
}

