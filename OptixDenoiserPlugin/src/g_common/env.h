//
// env.h
//
// An object to represent the environment map (cubemap)
// and parse its corresponding description file. (.env)
//

#ifndef __ENV_H
#define __ENV_H

#include <GL/gl.h>
#include "g_pfm.h"

class Environ
{
  public:
    Environ();
    ~Environ();

    void LoadEnv( const char *spath );
    void LoadEnv( char cm_path_list[6][256] );
    void LoadEnv( const GLuint env_tex[6] );

    void save_ftmap( const char *spath, int n_side );
    void save_tpmap( const char *spath, int n_side );
    void save_ppm( const char *spath, unsigned char r=0, unsigned char g=64, unsigned char b=128 );

    void draw( float eye_distance ); 
    GLuint GL_envbox;   // OpenGL Display Call List for drawing cubemap box.



    GPfm pfm[6];
  private:

    void LoadTexture();
    void ConstructEnvBox();

    GLuint pEnvTex[6];   // OpenGL texture ID
    GLuint own_tex;
};


#endif

