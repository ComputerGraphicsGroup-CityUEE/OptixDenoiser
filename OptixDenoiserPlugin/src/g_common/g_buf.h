//////////////////////////////////////////////////////////////////
//
//
// Class GBuf is designed specially for writting huge file
// that source data are available in a non-contiguous order.
// For example, let say we are going to pack corresponding 
// pixel of a set of images into a contiguous form of a single
// output file.  Then, every pixel of a single image file are
// non-contiguous.  To do that without GBuf, writting every
// single pixel would have to do one fseek and one fwrite.
// The overhead of doing so can easily stack up to consume
// huge resources.  And getting all data onto memory are 
// obviously not practical as all data can be too large 
// for physical memory (including virtual one).  While increasing
// the number of file descriptor is usually upper bounded by
// the system. And so... And so...
//
// Therefore, we have GBuf here.
//
// GBuf is just a buffer for contiguous file fragment.  If 
// you define a single GBuf covering whole output file, that 
// would make it a equivalent of file stream buffer.  Of 
// course, you can have any number of file fragment buffer.  
// In that sense, huge file writting can be buffered even source 
// data are available in a non-contiguous order.
//
// For the example discussed above, we can give every pixel a 
// file fragment buffer.  And so, to a single image, pixel
// data goes to corresponding file fragment buffer instead
// of output file.  File fragment buffer will keep your data
// to write awaiting in memory until it's full.
//
//
// Tze-Yui Ho, ma_hty@hotmail.com, Copyright(c), Sept 2004
//
//
//////////////////////////////////////////////////////////////////


/*
#include <stdio.h>

#include "g_buf.h"

void main()
{

  char ch[] = { 'a', 'b', 'c', 'd', 'e' };

  FILE *f0 = fopen( "src.txt", "wb" );

    GBuf gBuf[4];

    GBuf::init_all( gBuf, 4, f0, 40, .2f );

      gBuf[1].write( ch, sizeof(char), 5);
      gBuf[0].write( ch, sizeof(char), 4);

      gBuf[2].write( ch, sizeof(char), 5);
      gBuf[1].write( ch, sizeof(char), 5);
      gBuf[0].write( ch, sizeof(char), 2);

      gBuf[3].write( ch, sizeof(char), 5);
      gBuf[0].write( ch, sizeof(char), 3);
      gBuf[2].write( ch, sizeof(char), 5);

      gBuf[3].write( ch, sizeof(char), 5);

      gBuf[0].write( ch, sizeof(char), 1);

    GBuf::flush_all( gBuf, 4 );

  fclose(f0);

  if( GBuf::assert_all( gBuf, 4 ) )
    printf( "all file fragments are fullly filled.\n" );
  else
    printf( "some file fragments are not fullly filled.\n" );
}
*/


#ifndef G_BUF_H
#define G_BUF_H

#include <stdio.h>

class GBuf
{
  public:

    GBuf();
    ~GBuf();

    void init( FILE *stream, fpos_t init_fpos, size_t fragment_size, size_t buffer_size );
    void write( const void *src, size_t size, size_t count );
    void flush();
    bool assert1();

    static void init_all(
      GBuf *gBuf, int num_of_buf,
      FILE *stream, fpos_t total_size, size_t total_buf_size_mb
    );
    static void flush_all( GBuf *gBuf, int num_of_buf );

    static bool assert_all( GBuf *gBuf, int num_of_buf );

  private:

    FILE *m_stream;

    size_t m_fsize;
    fpos_t m_fpos;
    size_t m_bsize;
    unsigned char *m_buf;

    size_t t_fsize;
    fpos_t t_fpos;
    size_t t_bsize;
    unsigned char *t_buf;

    size_t write2buf( const void *src, size_t size, size_t count );
};

#endif G_BUF_H
