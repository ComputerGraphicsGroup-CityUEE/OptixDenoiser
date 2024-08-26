#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <GL/glew.h>

#include "glme.h"

#include "g_vector.h"
#include "g_common.h"

#define	M_PI		3.14159265358979323846

void prepare_tangents( GLMEmodel* model );

GLMEmodel* glmeReadOBJ( const char* spath )
{
  GLMmodel* glm_model = glmReadOBJ( spath );
  return glmeRide( glm_model );
}

GLMEmodel* glmeRide( GLMmodel *glm_model )
{
  assert(glm_model);

  
  GLMEmodel *model = (GLMEmodel*) malloc( sizeof(GLMEmodel) );
  memset( model, 0, sizeof(GLMEmodel) );

  model->glm = glm_model;
  model->vertices = (float*) malloc( (model->glm->numvertices+1) * 3*sizeof(float) );
  model->normals = (float*) malloc( (model->glm->numvertices+1) * 3*sizeof(float) );
  model->tangents1 = (float*) malloc( (model->glm->numvertices+1) * 3*sizeof(float) );
  model->tangents2 = (float*) malloc( (model->glm->numvertices+1) * 3*sizeof(float) );

  prepare_tangents( model );

  return model;
}

GLvoid glmeLoadBuf( GLMEmodel* model )
{
  GLuint i;
  GLMgroup* group;
  GLMtriangle* triangle;
  
  assert(model);
  assert(model->glm);
  assert(model->glm->vertices);
  

  model->n_triangles = 0;

  group = model->glm->groups;
  while(group)
  {
    model->n_triangles += group->numtriangles;
    group = group->next;
  };

  float *V, *tV;
    V = (float*) malloc( model->n_triangles * 3 * 3 * sizeof(float) );
    tV = V;

  float *N, *tN;
    N = (float*) malloc( model->n_triangles * 3 * 3 * sizeof(float) );
    tN = N;
  
  
  group = model->glm->groups;
  while (group) 
  {
    for( i=0; i<group->numtriangles; i++, tV+=9, tN+=9 ) 
    {
      triangle = &model->glm->triangles[group->triangles[i]];
      
      memcpy( &tV[0], &model->glm->vertices[3 * triangle->vindices[0]] , 3 * sizeof(float) );
      memcpy( &tV[3], &model->glm->vertices[3 * triangle->vindices[1]] , 3 * sizeof(float) );
      memcpy( &tV[6], &model->glm->vertices[3 * triangle->vindices[2]] , 3 * sizeof(float) );

      memcpy( &tN[0], &model->glm->normals[3 * triangle->nindices[0]] , 3 * sizeof(float) );
      memcpy( &tN[3], &model->glm->normals[3 * triangle->nindices[1]] , 3 * sizeof(float) );
      memcpy( &tN[6], &model->glm->normals[3 * triangle->nindices[2]] , 3 * sizeof(float) );

    }

    group = group->next;
  }

  glGenBuffersARB( 1, &model->arb_vertex_id );
  glBindBufferARB( GL_ARRAY_BUFFER_ARB, model->arb_vertex_id );
  glBufferDataARB( GL_ARRAY_BUFFER_ARB, model->n_triangles*3*3*sizeof(float), V, GL_STATIC_DRAW_ARB );

  glGenBuffersARB( 1, &model->arb_normal_id );
  glBindBufferARB( GL_ARRAY_BUFFER_ARB, model->arb_normal_id );
  glBufferDataARB( GL_ARRAY_BUFFER_ARB, model->n_triangles*3*3*sizeof(float), N, GL_STATIC_DRAW_ARB );

  free(V);
  free(N);
}

GLvoid glmeBufDraw( GLMEmodel* model )
{
  glEnableClientState(GL_VERTEX_ARRAY);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, model->arb_vertex_id);
  glVertexPointer(3, GL_FLOAT, 0, 0);

  glEnableClientState(GL_NORMAL_ARRAY);
  glBindBufferARB(GL_ARRAY_BUFFER_ARB, model->arb_normal_id);
  glNormalPointer( GL_FLOAT, 0, 0);

  glDrawArrays( GL_TRIANGLES, 0, model->n_triangles*3 );

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
}

GLvoid glmeDraw( GLMEmodel* model )
{
  static GLuint i;
  static GLMgroup* group;
  static GLMtriangle* triangle;
  static GLMmaterial* material;
  
  assert(model);
  assert(model->glm);
  assert(model->glm->vertices);
  
  
  group = model->glm->groups;
  while (group) 
  {
    glBegin(GL_TRIANGLES);

      for( i=0; i<group->numtriangles; i++ ) 
      {
        triangle = &model->glm->triangles[group->triangles[i]];
      
        glNormal3fv( &model->glm->normals[3 * triangle->nindices[0]] );
        glTexCoord3fv( &model->tangents1[3 * triangle->vindices[0]] );
        glVertex3fv( &model->glm->vertices[3 * triangle->vindices[0]] );
      
        glNormal3fv( &model->glm->normals[3 * triangle->nindices[1]] );
        glTexCoord3fv( &model->tangents1[3 * triangle->vindices[0]] );
        glVertex3fv( &model->glm->vertices[3 * triangle->vindices[1]] );
      
        glNormal3fv( &model->glm->normals[3 * triangle->nindices[2]] );
        glTexCoord3fv( &model->tangents1[3 * triangle->vindices[0]] );
        glVertex3fv( &model->glm->vertices[3 * triangle->vindices[2]] );
      
      }

    glEnd();
    
    group = group->next;
  }
}

GLuint glmeList( GLMEmodel* model )
{
  GLuint list;
  
  list = glGenLists(1);
  glNewList(list, GL_COMPILE);
  glmeDraw( model );
  glEndList();
  
  return list;
}

GLvoid glmeDelete( GLMEmodel* model )
{
  glmDelete( model->glm );
  free( model->vertices );
  free( model->normals );
  free( model->tangents1 );
  free( model->tangents2 );

  if(model->arb_vertex_id)
    glDeleteBuffersARB( 1, &model->arb_vertex_id );
  if(model->arb_normal_id)
    glDeleteBuffersARB( 1, &model->arb_normal_id );

  free( model );
}

void prepare_tangents( GLMEmodel* model )
{

  GLMmodel* glm_model = model->glm;
  float *vertices = model->vertices;
  float *normals = model->normals;
  float *tangents1 = model->tangents1;
  float *tangents2  = model->tangents2;


  bool  *isdonecaltangent;
  isdonecaltangent = new bool [glm_model->numvertices+1];
  memset( isdonecaltangent, 0, (glm_model->numvertices+1)*sizeof(bool) );
  
  
  
  unsigned int i;
  int v_idx0, v_idx1, v_idx2;
  int n_idx0, n_idx1, n_idx2;
  FLOAT3 v0,v1,v2;
  FLOAT3 n0,n1,n2;
  FLOAT3 tt,t1,t2;

  FLOAT3 *V,*N,*T1,*T2;
    V = (FLOAT3*)vertices;
    N = (FLOAT3*)normals;
    T1 = (FLOAT3*)tangents1;
    T2 = (FLOAT3*)tangents2;

  for( i=0; i<glm_model->numtriangles; i++ )
  {
    v_idx0 = glm_model->triangles[i].vindices[0];
    v_idx1 = glm_model->triangles[i].vindices[1];
    v_idx2 = glm_model->triangles[i].vindices[2];

    n_idx0 = glm_model->triangles[i].nindices[0];
    n_idx1 = glm_model->triangles[i].nindices[1];
    n_idx2 = glm_model->triangles[i].nindices[2];

    v0 = *( (FLOAT3*) &glm_model->vertices[3*v_idx0] );
    v1 = *( (FLOAT3*) &glm_model->vertices[3*v_idx1] );
    v2 = *( (FLOAT3*) &glm_model->vertices[3*v_idx2] );

    n0 = *( (FLOAT3*) &glm_model->normals[3*n_idx0] );
    n1 = *( (FLOAT3*) &glm_model->normals[3*n_idx1] );
    n2 = *( (FLOAT3*) &glm_model->normals[3*n_idx2] );



    if( !isdonecaltangent[v_idx0] )
    {


      tt = v1 - v0;
      vnormalize( &tt );

      t1 = vcross( tt, n0 );
      vnormalize( &t1 );

      t2 = vcross( n0, t1 );
      vnormalize( &t2 );


      V[v_idx0] = v0;
      N[v_idx0] = n0;
      T1[v_idx0] = t1;
      T2[v_idx0] = t2;

      isdonecaltangent[v_idx0] = true;
    }
    if( !isdonecaltangent[v_idx1] )
    {


      tt = v2 - v1;
      vnormalize( &tt );

      t1 = vcross( tt, n1 );
      vnormalize( &t1 );

      t2 = vcross( n1, t1 );
      vnormalize( &t2 );

      V[v_idx1] = v1;
      N[v_idx1] = n1;
      T1[v_idx1] = t1;
      T2[v_idx1] = t2;

      isdonecaltangent[v_idx1] = true;
    }
    if( !isdonecaltangent[v_idx2] )
    {


      tt = v0 - v2;
      vnormalize( &tt );

      t1 = vcross( tt, n2 );
      vnormalize( &t1 );

      t2 = vcross( n2, t1 );
      vnormalize( &t2 );

      V[v_idx2] = v2;
      N[v_idx2] = n2;
      T1[v_idx2] = t1;
      T2[v_idx2] = t2;

      isdonecaltangent[v_idx2] = true;
    }
  }

  for( i=1; i<glm_model->numvertices+1; i++ )
  {
    if (!isdonecaltangent[i])
    {
      printf( "[Error]: Tangent has not been calculated\n");
      printf( "index = %i\n", i);
      exit(-1);
    }
  }

  delete []isdonecaltangent;
}

