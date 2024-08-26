#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include "g_vector.h"
#include "g_common.h"
#include "g_wan.h"

GWan* gwanLoad( const char *spath_pts, const char *spath_cs )
{
  int n_light, n_light2;
  float energy_light;
  float *tp_light, *tp;
  float *rgb_light, *rgb;
  float *xyz_light, *xyz;

  int i;
  char str[256];

  FILE *f0 = fopen( spath_pts, "rb" );
    if( f0 == NULL )
    {
      printf( "[Error] gwanLoad(), file \"%s\" not found.\n", spath_pts );
      exit(-1);
    }

    fgets( str, 256, f0 );
    sscanf( str, "%i", &n_light );


  FILE *f1 = fopen( spath_cs , "rb" );
    if( f1 == NULL )
    {
      printf( "[Error] gwanLoad(), file \"%s\" not found.\n", spath_cs );
      exit(-1);
    }
    n_light2;
    fgets( str, 256, f1 );
    sscanf( str, "%i", &n_light2 );
    if( n_light2 != n_light )
    {
      printf( "[Error] load_light_source(), .pts and .cs is not consistent.\n" );
      exit(-1);
    }
    fgets( str, 256, f1 );
    sscanf( str, "%f", &energy_light );


    tp_light  = (float*) malloc( n_light * 2*sizeof(float) );

    rgb_light  = (float*) malloc( n_light * 3*sizeof(float) );

    xyz_light = (float*) malloc( n_light * 3*sizeof(float) );


  for(
    i=0, tp = tp_light, rgb = rgb_light, xyz = xyz_light; 
    i<n_light; 
    i++, tp+=2, rgb+=3, xyz+=3
  ){
    fgets( str, 256, f0 );
    sscanf( str, "%f %f", &tp[1], &tp[0] );
    tp[1] = -tp[1];
    Angle2Float3( tp[0], tp[1], (FLOAT3*)xyz );
    fgets( str, 256, f1 );
    sscanf( str, "%f %f %f", &rgb[0], &rgb[1], &rgb[2] );
  }

  fclose(f0);
  fclose(f1);

  GWan *wan = (GWan*) malloc( sizeof(GWan) );
    strcpy( wan->spath_pts, spath_pts );
    strcpy( wan->spath_cs, spath_cs );
    wan->n_light      = n_light;
    wan->energy_light = energy_light;
    wan->tp_light     = tp_light;
    wan->rgb_light    = rgb_light;
    wan->xyz_light    = xyz_light;

  return wan;
}

void gwanDelete( GWan *wan )
{
  if( wan )
  {
    SAFE_FREE( wan->tp_light );
    SAFE_FREE( wan->rgb_light  );
    SAFE_FREE( wan->xyz_light  );
    SAFE_FREE( wan );
  }
}

GLuint gwanList( GWan *wan, float color_scale )
{
  GLuint list;

  int n_light = wan->n_light;

  float val, val_max, val_min, val_range;

  FLOAT3 *rgb, *xyz;
  int i;
    

  rgb = (FLOAT3*) wan->rgb_light;
  for( 
    i=0, val_max = -FLT_MAX, val_min = FLT_MAX; 
    i<n_light; 
    i++, rgb++
  ){
    val = sqrtf( vdot( rgb, rgb ) );

    if( val > val_max )
      val_max = val;

    if( val < val_min )
      val_min = val;
  }

  val_range = val_max - val_min;


  list = glGenLists(1);

    glNewList(list, GL_COMPILE);

      glBegin( GL_POINTS );

        rgb = (FLOAT3*) wan->rgb_light;
        xyz = (FLOAT3*) wan->xyz_light;
        for( i=0; i<n_light; i++, rgb++, xyz++ )
        {
          FLOAT3 tmp;
          tmp = *rgb*color_scale;
          glColor3fv(  (float*)&tmp );
          glVertex3fv( (float*)xyz );
        }
      glEnd();

    glEndList();

  return list;
}


