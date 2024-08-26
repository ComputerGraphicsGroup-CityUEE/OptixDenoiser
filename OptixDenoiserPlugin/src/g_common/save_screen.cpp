#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#  include <windows.h>
#include <GL/gl.h>

void save_screen( const char *spath )
{

  GLint vp[4];
  glGetIntegerv( GL_VIEWPORT, vp );
  
  int x,y, w,h;
    x = vp[0];
    y = vp[1];
    w = vp[2];
    h = vp[3];

  int j;

  unsigned char *bottomup_pixel = (unsigned char *) malloc( w*h*3*sizeof(unsigned char) );
  unsigned char *topdown_pixel = (unsigned char *) malloc( w*h*3*sizeof(unsigned char) );


  //Byte alignment (that is, no alignment)
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  glReadPixels( x, y, w, h, GL_RGB, GL_UNSIGNED_BYTE, bottomup_pixel);
  for( j=0; j<h; j++ )
    memcpy( &topdown_pixel[j*w*3], &bottomup_pixel[(h-j-1)*w*3], w*3*sizeof(unsigned char) );
  
  
  FILE *f0 = fopen( spath, "wb" );
  if( f0==NULL )
  {
    printf( "[Error] : SaveScreen(), Cannot open %s for writting.\n", spath );
    exit(-1);
  }

  fprintf( f0, "P6\n%d %d\n255\n", w, h);
  fwrite( topdown_pixel, sizeof(unsigned char), w*h*3, f0);
  fclose( f0 );

  free(bottomup_pixel);
  free(topdown_pixel);
}

void save_screen2( const char *spath, int x, int y, int w, int h )
{

  int j;

  unsigned char *bottomup_pixel = (unsigned char *) malloc( w*h*3*sizeof(unsigned char) );
  unsigned char *topdown_pixel = (unsigned char *) malloc( w*h*3*sizeof(unsigned char) );


  //Byte alignment (that is, no alignment)
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  glReadPixels( x, y, w, h, GL_RGB, GL_UNSIGNED_BYTE, bottomup_pixel);
  for( j=0; j<h; j++ )
    memcpy( &topdown_pixel[j*w*3], &bottomup_pixel[(h-j-1)*w*3], w*3*sizeof(unsigned char) );
  
  
  FILE *f0 = fopen( spath, "wb" );
  if( f0==NULL )
  {
    printf( "[Error] : SaveScreen(), Cannot open %s for writting.\n", spath );
    exit(-1);
  }

  fprintf( f0, "P6\n%d %d\n255\n", w, h);
    //InvertBuf( pixel, h, w*3*sizeof(unsigned char) );  // invert the image

  fwrite( topdown_pixel, sizeof(unsigned char), w*h*3, f0);
  fclose( f0 );

  free(bottomup_pixel);
  free(topdown_pixel);
}

void save_screen3( unsigned char *topdown_pixel, int x, int y, int w, int h, GLenum format, int n_channel )
{

  int j;

  unsigned char *bottomup_pixel = (unsigned char *) malloc( w*h*n_channel*sizeof(unsigned char) );


  //Byte alignment (that is, no alignment)
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  glReadPixels( x, y, w, h, format, GL_UNSIGNED_BYTE, bottomup_pixel);
  for( j=0; j<h; j++ )
    memcpy( &topdown_pixel[j*w*n_channel], &bottomup_pixel[(h-j-1)*w*n_channel], w*n_channel*sizeof(unsigned char) );
  
  free(bottomup_pixel);

}

void save_screen4( float *topdown_pixel, int x, int y, int w, int h )
{
  int j;

  float *bottomup_pixel = (float *) malloc( w*h*3*sizeof(float) );


  //Byte alignment (that is, no alignment)
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  glReadPixels( x, y, w, h, GL_RGB, GL_FLOAT, bottomup_pixel);
  for( j=0; j<h; j++ )
    memcpy( &topdown_pixel[j*w*3], &bottomup_pixel[(h-j-1)*w*3], w*3*sizeof(float) );
  
  free(bottomup_pixel);

}
