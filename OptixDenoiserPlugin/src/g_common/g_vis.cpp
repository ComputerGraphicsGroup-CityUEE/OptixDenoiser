#include <stdlib.h>
#include <stdio.h>
#include <float.h>


#include <GL/glew.h>
#include <GL/glut.h>
#include "RenderTexture.h"

#include "g_vis.h"

#include "g_vector.h"
#include "g_common.h"
#include "g_cubemap.h"

GVis::GVis()
{
  glutCreateWindow( "g_cubemap" );
  glewInit();
  memset( this, 0, sizeof(GVis) );
}

GVis::~GVis()
{
  int i;
  for( i=0; i<6; i++ )
    delete vis_rt[i];

  if(glme_model)
    glmeDelete( glme_model );
}

void GVis::set_size( int cm_side )
{
  vis_cm_side = cm_side;
}

void GVis::set_obj( const char *spath )
{
  glme_model= glmeReadOBJ( spath );
  glmeLoadBuf( glme_model );
}

void GVis::prepare_render()
{
  int cm_side = vis_cm_side;

  int i;
  for( i=0; i<6; i++ )
  {
    vis_rt[i] = new RenderTexture( cm_side, cm_side );
    vis_rt[i]->Initialize( true, true, false, false, false, 8,8,8,0 );
  }

  glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
      gluPerspective( 
        90.0,  // FOV
        1.0,   // Aspect
        GL_EPSILON, // Near
        1000.0  // Far
      );
  glGetFloatv( GL_PROJECTION_MATRIX, vis_projection );
}

void GVis::GetCmModelView( 
  FLOAT3 v, FLOAT3 n, FLOAT3 t1, FLOAT3 t2, 
  float **cm_modelview
){
  FLOAT3 pos_x = t2;
  FLOAT3 pos_y = n;
  FLOAT3 pos_z = t1;

  FLOAT3 neg_x = pos_x*-1;
  FLOAT3 neg_y = pos_y*-1;
  FLOAT3 neg_z = pos_z*-1;

  FLOAT3 eye, center, up;
  FLOAT3 view_direction;


  int face_idx;
  for( face_idx=0; face_idx<6; face_idx++ )
  {
    switch( face_idx )
    {
      case POS_X:
        up     = pos_y;
        view_direction = pos_x;
        break;
      case NEG_X:
        up     = pos_y;
        view_direction = neg_x;
        break;

      case POS_Y:
        up     = neg_z;
        view_direction = pos_y;
        break;
      case NEG_Y:
        up     = pos_z;
        view_direction = neg_y;
        break;

      case POS_Z:
        up     = pos_y;
        view_direction = pos_z;
        break;
      case NEG_Z:
        up     = pos_y;
        view_direction = neg_z;
        break;
    }

    eye = v + n*GL_EPSILON;
    center = eye + view_direction;


    glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
        gluLookAt(
          eye.x, eye.y, eye.z,  
          center.x, center.y, center.z,
          up.x, up.y, up.z 
        );

    glGetFloatv( GL_MODELVIEW_MATRIX, cm_modelview[face_idx] );
  }

}

void GVis::render( FLOAT3 v, FLOAT3 n, FLOAT3 t1, FLOAT3 t2 )
{
  int cm_side = vis_cm_side;

  int i;

  float _cm_modelview[6][16];
  float *cm_modelview[6];
    for( i=0; i<6; i++ )
      cm_modelview[i] = _cm_modelview[i];

    GetCmModelView( v, n, t1, t2, cm_modelview );


    int cm_idx[] = { POS_Y, POS_X, POS_Z, NEG_X, NEG_Z, NEG_Y };

    for( i=0; i<6; i++ )
    {
      vis_rt[i]->BeginCapture();

      glEnable(GL_DEPTH_TEST);
      glClearColor(1.0, 1.0, 1.0, 1.0);
      glColor3f( 0,0,0 );

      glMatrixMode(GL_PROJECTION);
        glLoadMatrixf( vis_projection );
        glScalef( 1,-1,1 );
      glMatrixMode(GL_MODELVIEW);

      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      glLoadMatrixf( cm_modelview[cm_idx[i]] );

      glmeBufDraw( glme_model );

      vis_rt[i]->EndCapture();

    }
}

BYTE GVis::lookup( FLOAT3 v )
{
  int idx = cm_lookup(v, vis_cm_side);

  int face_idx = idx / ( vis_cm_side*vis_cm_side );

  int xy = idx % ( vis_cm_side*vis_cm_side );
  int x = xy%vis_cm_side;
  int y = xy/vis_cm_side;

  BYTE val[3];

  vis_rt[face_idx]->BeginCapture();

  glReadPixels( x,y, 1,1, GL_RGB, GL_UNSIGNED_BYTE, val );

  vis_rt[face_idx]->EndCapture();


  return val[0]/255;
}
/*
void GVis::vis_cm_debug()
{
  int i,j,k;

  BYTE *vis_data = (BYTE*)  malloc( vis_cm_side*vis_cm_side*6 * sizeof(BYTE)  );
  BYTE *t_data = vis_data;

  for( k=0; k<6; k++ )
  {
    vis_rt[k]->BeginCapture();

    for( j=0; j<vis_cm_side; j++ )
      for( i=0; i<vis_cm_side; i++ )
        glReadPixels( i,j, 1,1, GL_RED, GL_UNSIGNED_BYTE, t_data++ );

    vis_rt[k]->EndCapture();
  }

  cm_save( "vis_cm_debug.ppm", vis_data, vis_cm_side );

  free( vis_data );
}
*/