#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "g_buf.h"

GBuf::GBuf()
{
  m_stream = NULL;

  m_fsize = 0;
  m_bsize = 0;
  m_fpos = 0;

  t_fsize = 0;
  t_fpos = 0;

  m_buf = NULL;
  t_buf = NULL;
  t_bsize = 0;
}

void GBuf::init( FILE *stream, fpos_t init_fpos, size_t fragment_size, size_t buffer_size )
{
  m_stream = stream;

  m_fsize = fragment_size;
  m_bsize = buffer_size;
  m_fpos = init_fpos;

  t_fsize = 0;
  t_fpos = init_fpos;

  m_buf = (unsigned char*) malloc( buffer_size * sizeof(unsigned char) );
  t_buf = m_buf;
  t_bsize = 0;
}

GBuf::~GBuf()
{
  if(m_buf!=NULL)
  {
    free(m_buf);
    m_buf=NULL;
  }
}

void GBuf::write( const void *src, size_t size, size_t count )
{
  if( t_fsize+size*count <= m_fsize )
  {
    t_fsize += size*count;
    if( count*size >= m_bsize )
    {
      flush();
      fsetpos( m_stream, &t_fpos );
      t_fpos += size*fwrite( src, size, count, m_stream );
    }else if( t_bsize + size*count >= m_bsize )
    {
      const unsigned char *t_src = (const unsigned char *)src;
      size_t tmp = size*count-(m_bsize-t_bsize);
      t_src += write2buf( t_src, 1, m_bsize-t_bsize );
      flush();
      write2buf( t_src, 1, tmp );
    }else
    {
      write2buf( src, size, count );
    }
  }else
  {
    printf( "[Error] : attempted to write data beyond defined GBuf, operation discarded.\n" );
  }
}

size_t GBuf::write2buf( const void *src, size_t size, size_t count )
{
  int tmp_size = size*count;
  if( tmp_size<=0 ) return 0;

  memcpy( t_buf, src, tmp_size );
  t_buf += tmp_size;
  t_bsize +=  tmp_size;

  return tmp_size;
}

void GBuf::flush()
{
  if(t_bsize==0) return;
  fsetpos( m_stream, &t_fpos );
  t_fpos += fwrite( m_buf, sizeof(unsigned char), t_bsize, m_stream );

  t_buf = m_buf;
  t_bsize = 0;
}

bool GBuf::assert1()
{
  if( t_fsize == m_fsize )
    return true;
  return false;
}


void GBuf::init_all(
  GBuf *gBuf, int num_of_buf,
  FILE *stream, fpos_t total_size, size_t total_buf_size_mb
){
  if( total_size%num_of_buf )
  {
    printf( "[ERROR] - num_of_buf must divid total_size\n" );
    exit(-1);
  }

  printf( "Allocating harddisk space... " );
    fpos_t last_fpos;
      last_fpos = total_size-1;
      fsetpos( stream, &last_fpos );

      unsigned char xxx = 0;
      fwrite( &xxx, sizeof(unsigned char), 1, stream );
  printf( "done.\n" );


  size_t fragment_size = (size_t) ( total_size/num_of_buf );
  size_t buffer_size = (size_t) ( 1024*1024*total_buf_size_mb/num_of_buf );
         buffer_size = buffer_size>fragment_size ? fragment_size : buffer_size;
  fpos_t init_fpos = 0;

  printf( "Totally %.2fMB File Fragment Buffer were assigned.\n", 1.0*buffer_size*num_of_buf/1024/1024 );

  for( int i=0; i<num_of_buf; i++, init_fpos+=fragment_size )
    gBuf[i].init( stream, init_fpos, fragment_size, buffer_size );
}

void GBuf::flush_all(
  GBuf *gBuf, int num_of_buf
){
  for( int i=0; i<num_of_buf; i++ )
    gBuf[i].flush();
}

bool GBuf::assert_all( GBuf *gBuf, int num_of_buf )
{
  for( int i=0; i<num_of_buf; i++ )
  {
    if( !gBuf[i].assert1() )
      return false;
  }
  return true;
}
