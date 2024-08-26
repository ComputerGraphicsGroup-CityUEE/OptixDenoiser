//
// env.cpp
//
// An object to represent the environment map (cubemap)
// and parse its corresponding description file. (.env)
//
// modified by Philip Fu, 8/21/2002 10:11AM
// modified by TT 21 Apr 2003
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include "env.h"

#include "g_common.h"
#include "g_vector.h"
#include "g_pmd.h"


enum CUBE_MAP_FACE
{
  POS_X, NEG_X,
  POS_Y, NEG_Y,
  POS_Z, NEG_Z
};


Environ::Environ()
{
  memset( this, 0, sizeof(Environ) );
}

Environ::~Environ()
{
  if(pEnvTex!=NULL && own_tex )
    glDeleteTextures( 6, pEnvTex );
  if( GL_envbox!=NULL )
    glDeleteLists( GL_envbox, 1 );
}

void Environ::LoadEnv( const char *spath )
{

  int i;
  char tpath[256];
  char cm_path_list[6][256];
  int rotate[6];

  GPmd pmd;
    pmd.load( spath );
    pmd.query( "FRONT" , "%s %i", cm_path_list[POS_Z], &rotate[POS_Z] );
    pmd.query( "LEFT"  , "%s %i", cm_path_list[POS_X], &rotate[POS_X] );
    pmd.query( "BACK"  , "%s %i", cm_path_list[NEG_Z], &rotate[NEG_Z] );
    pmd.query( "RIGHT" , "%s %i", cm_path_list[NEG_X], &rotate[NEG_X] );
    pmd.query( "TOP"   , "%s %i", cm_path_list[POS_Y], &rotate[POS_Y] );
    pmd.query( "BOTTOM", "%s %i", cm_path_list[NEG_Y], &rotate[NEG_Y] );


  GPath* gp = parse_path( spath );
    for( i=0; i<6; i++ )
    {
      sprintf( tpath, "%s%s", gp->dname, cm_path_list[i] );
      printf("reading [%s] ... ", tpath);

        pfm[i].load( tpath );
        pfm[i].rotate(rotate[i]);

      printf("done.\n");
    }

  LoadTexture();
}

void Environ::LoadEnv( const GLuint env_tex[6] )
{
  int i;
  for( i=0; i<6; i++ )
    pEnvTex[i] = env_tex[i];
  if( GL_envbox==NULL )
    ConstructEnvBox();
}

void Environ::LoadEnv( char cm_path_list[6][256] )
{
  int i;
  for( i=0; i<6; i++ )
    pfm[i].load(  cm_path_list[i]  );

  LoadTexture();
}

void Environ::LoadTexture()
{

  own_tex = true;

  if( pEnvTex[0]==NULL )
    glGenTextures(6, pEnvTex);
  if( GL_envbox==NULL )
    ConstructEnvBox();



  GPfm tmp;

  for( int i=0; i<6; i++ ) 
  {
    glBindTexture(GL_TEXTURE_2D, pEnvTex[i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      tmp.load( pfm[i].w, pfm[i].h, pfm[i].fm );
      tmp.flip_vertical();

      glTexImage2D(
        GL_TEXTURE_2D, 0, 
        GL_RGB, tmp.w, tmp.h, 0, 
        GL_RGB, GL_FLOAT, 
        tmp.fm
      );
  }
}




// 
// ConstructEnvBox
//
// Construct the display list of the environment bounding box.
//
// AFFECT:
//   G_envbox
//

void Environ::ConstructEnvBox()
{
  // Create the display list of the bounding box
  // for Internal View


  GLuint env_list = glGenLists(1);
  glNewList(env_list, GL_COMPILE);

  glEnable(GL_TEXTURE_2D);
  //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

  glPushMatrix();

    // -Z side
    glBindTexture(GL_TEXTURE_2D, pEnvTex[NEG_Z]);
      glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);  glVertex3f( -1, -1, -1 );
        glTexCoord2f(0.0, 1.0);  glVertex3f( -1,  1, -1 );
        glTexCoord2f(1.0, 1.0);  glVertex3f(  1,  1, -1 );
        glTexCoord2f(1.0, 0.0);  glVertex3f(  1, -1, -1 );
      glEnd();
      glRotatef( 90, 0,1,0 );


    // -X side
    glBindTexture(GL_TEXTURE_2D, pEnvTex[NEG_X]);
      glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);  glVertex3f( -1, -1, -1 );
        glTexCoord2f(0.0, 1.0);  glVertex3f( -1,  1, -1 );
        glTexCoord2f(1.0, 1.0);  glVertex3f(  1,  1, -1 );
        glTexCoord2f(1.0, 0.0);  glVertex3f(  1, -1, -1 );
      glEnd();
      glRotatef( 90, 0,1,0 );

    // +Z side
    glBindTexture(GL_TEXTURE_2D, pEnvTex[POS_Z]);
      glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);  glVertex3f( -1, -1, -1 );
        glTexCoord2f(0.0, 1.0);  glVertex3f( -1,  1, -1 );
        glTexCoord2f(1.0, 1.0);  glVertex3f(  1,  1, -1 );
        glTexCoord2f(1.0, 0.0);  glVertex3f(  1, -1, -1 );
      glEnd();
      glRotatef( 90, 0,1,0 );

    // +X side
    glBindTexture(GL_TEXTURE_2D, pEnvTex[POS_X]);
      glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);  glVertex3f( -1, -1, -1 );
        glTexCoord2f(0.0, 1.0);  glVertex3f( -1,  1, -1 );
        glTexCoord2f(1.0, 1.0);  glVertex3f(  1,  1, -1 );
        glTexCoord2f(1.0, 0.0);  glVertex3f(  1, -1, -1 );
      glEnd();
      glRotatef( -90, 0,1,0 );
      glRotatef( 90, 1,0,0 );

    // +Y side
    glBindTexture(GL_TEXTURE_2D, pEnvTex[POS_Y]);
      glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);  glVertex3f( -1, -1, -1 );
        glTexCoord2f(0.0, 1.0);  glVertex3f( -1,  1, -1 );
        glTexCoord2f(1.0, 1.0);  glVertex3f(  1,  1, -1 );
        glTexCoord2f(1.0, 0.0);  glVertex3f(  1, -1, -1 );
      glEnd();
      glRotatef( 180, 1,0,0 );

    // -Y side
    glBindTexture(GL_TEXTURE_2D, pEnvTex[NEG_Y]);
      glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);  glVertex3f( -1, -1, -1 );
        glTexCoord2f(0.0, 1.0);  glVertex3f( -1,  1, -1 );
        glTexCoord2f(1.0, 1.0);  glVertex3f(  1,  1, -1 );
        glTexCoord2f(1.0, 0.0);  glVertex3f(  1, -1, -1 );
      glEnd();

    glPopMatrix();
  glDisable(GL_TEXTURE_2D);

  glEndList();

  GL_envbox = env_list;
}



void Environ::save_ftmap( const char *spath, int n_side )
{
  int i;
  GPfm tmp[6];
    for( i=0; i<6; i++ )
      pfm[i].scale( tmp[i], n_side, n_side );

  FILE *f0 = fopen( spath, "wb" );
    fwrite( tmp[POS_Y].fm, sizeof(float), n_side*n_side*3, f0 );
    fwrite( tmp[POS_X].fm, sizeof(float), n_side*n_side*3, f0 );
    fwrite( tmp[POS_Z].fm, sizeof(float), n_side*n_side*3, f0 );
    fwrite( tmp[NEG_X].fm, sizeof(float), n_side*n_side*3, f0 );
    fwrite( tmp[NEG_Z].fm, sizeof(float), n_side*n_side*3, f0 );
    fwrite( tmp[NEG_Y].fm, sizeof(float), n_side*n_side*3, f0 );
  fclose(f0);

}

void Environ::save_tpmap( const char *spath, int n_side )
{
  printf( "Preparing Theta-Phi Map... " );

  //FLOAT3 texcord[4] = { {0,1}, {1,1}, {1,0}, {0,0} };

  FLOAT3 v[8] = 
  {
    FLOAT3(  1,  1, -1 ),  // v0
    FLOAT3( -1,  1, -1 ),  // v1
    FLOAT3( -1,  1,  1 ),  // v2
    FLOAT3(  1,  1,  1 ),  // v3
    FLOAT3(  1, -1, -1 ),  // v4
    FLOAT3( -1, -1, -1 ),  // v5
    FLOAT3( -1, -1,  1 ),  // v6
    FLOAT3(  1, -1,  1 ),  // v7
  };

  int vidx[6][4] = 
  {
    { 0, 3, 7, 4 },  // +x face
    { 2, 1, 5, 6 },  // -x face
    { 0, 1, 2, 3 },  // +y face
    { 7, 6, 5, 4 },  // -y face
    { 3, 2, 6, 7 },  // +z face
    { 1, 0, 4, 5 },  // -z face
  };
  

  FLOAT3 o,x,y;
  FLOAT3 dx,dy;
  FLOAT3 Sij;
  int i,j,k;
  float *tp;

  float **tp_face;
  {
    tp_face = (float**) malloc( 6*sizeof(float*) + 6*n_side * n_side * 2 * sizeof(float) );
    for(i=0; i<6; i++ )
      tp_face[i] = ((float*)(tp_face + 6)) + i*n_side * n_side * 2;
  }

  for( k=0; k<6; k++ )
  {
    tp = tp_face[k];
    o = v[vidx[k][0]];
    x = v[vidx[k][1]];
    y = v[vidx[k][3]];

    dx = (x-o)/(float)n_side;
    dy = (y-o)/(float)n_side;
    for( j=0; j<n_side; j++ )
      for( i=0; i<n_side; i++, tp+=2)
      {
        Sij = o + dx*(i+.5f) + dy*(j+.5f);
        Float32Angle( tp, tp+1, &Sij );
      }
  }


  float *tp_data;
    tp_data = (float*) malloc( n_side * 6*n_side * 2 * sizeof(float) );
    tp = tp_data;
      memcpy( tp, tp_face[POS_Y], n_side * n_side * 2*sizeof(float) );  tp += n_side * n_side * 2;
      memcpy( tp, tp_face[POS_X], n_side * n_side * 2*sizeof(float) );  tp += n_side * n_side * 2;
      memcpy( tp, tp_face[POS_Z], n_side * n_side * 2*sizeof(float) );  tp += n_side * n_side * 2;
      memcpy( tp, tp_face[NEG_X], n_side * n_side * 2*sizeof(float) );  tp += n_side * n_side * 2;
      memcpy( tp, tp_face[NEG_Z], n_side * n_side * 2*sizeof(float) );  tp += n_side * n_side * 2;
      memcpy( tp, tp_face[NEG_Y], n_side * n_side * 2*sizeof(float) );  tp += n_side * n_side * 2;

  FILE *f0 = fopen( spath, "wb" );
    fwrite( tp_data, sizeof(float), 6 * n_side * n_side * 2, f0 );
  fclose(f0);

  free( tp_data );
  free( tp_face );


  printf( "Done\n" );
}


void Environ::save_ppm( const char *spath, unsigned char r, unsigned char g, unsigned char b )
{
  int n = pfm[0].w;

  GPfm tmp;
    tmp.load( n*4, n*3, r/255.0f, g/255.0f, b/255.0f );

    tmp.draw( pfm[0],   0,   n );
    tmp.draw( pfm[1], 2*n,   n );
    tmp.draw( pfm[2],   n,   0 );
    tmp.draw( pfm[3],   n, 2*n );
    tmp.draw( pfm[4],   n,   n );
    tmp.draw( pfm[5], 3*n,   n );

  tmp.save( spath, "ppm" );
}


void Environ::draw( float eye_distance )
{
  glPushMatrix();
    glScalef( eye_distance, eye_distance, eye_distance );
      glEnable(GL_TEXTURE_2D);
        glCallList( GL_envbox );
      glDisable(GL_TEXTURE_2D);
  glPopMatrix();
}


