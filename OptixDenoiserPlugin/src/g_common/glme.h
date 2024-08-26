#ifndef GLME_H
#define GLME_H


#include "glm.h"

typedef struct _GLMEmodel {

  GLMmodel* glm;
  float *vertices;
  float *normals;
  float *tangents1;
  float *tangents2;

  int n_triangles;
  GLuint arb_vertex_id;
  GLuint arb_normal_id;

} GLMEmodel;

GLMEmodel* glmeReadOBJ( const char* spath );
GLMEmodel* glmeRide( GLMmodel *glm_model );
GLvoid glmeDraw( GLMEmodel* model );
GLuint glmeList( GLMEmodel* model );
GLvoid glmeDelete( GLMEmodel* model );
GLvoid glmeLoadBuf( GLMEmodel* model );
GLvoid glmeBufDraw( GLMEmodel* model );

#endif
