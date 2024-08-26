#ifndef SAVE_SCREEN_H
#define SAVE_SCREEN_H
//#include <GL/glut.h>

void save_screen( const char *spath );
void save_screen2( const char *spath, int x, int y, int w, int h );
void save_screen3( unsigned char *topdown_pixel, int x, int y, int w, int h, GLenum format=GL_RGB, int n_channel=3 );
void save_screen4( float *topdown_pixel, int x, int y, int w, int h );

#endif
