#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <float.h>
#include <math.h>

#include "g_common.h"
#include "g_angle.h"


GAngle::GAngle()
{
  memset( this, 0, sizeof(GAngle) );
}

GAngle::~GAngle()
{
  SAFE_FREE( v_src );
}

void GAngle::load( const char *spath )
{
  SAFE_FREE( v_src );

  {
    FILE *f0 = fopen( spath, "rb" );
    if( f0 == NULL )
    {
      printf( "[Error] GAngle::load(), %s not found.\n", spath );
      exit(-1);
    }
    fclose(f0);
  }

  {
    char str[256];
    FILE *f0 = fopen( spath, "rb" );
      n_src = 0;
      while( fgets( str, 256, f0 ) )
        n_src++;
    fclose(f0);
  }

  v_src = (FLOAT3*) malloc( n_src * sizeof(FLOAT3) );
  {
    int i;
    char str[256];
    float theta, phi;

    FILE *f0 = fopen( spath, "rb" );
      for( i=0; i<n_src; i++ )
      {
        fgets( str, 256, f0 );
        sscanf( str, "%f %f", &theta, &phi );
        Angle2Float3( theta, phi, &v_src[i] );
      }
    fclose(f0);
  }

}

int GAngle::nearest_index( FLOAT3 l )
{
  int   i, imin;
  float d, dmin;

  dmin = FLT_MAX;
  for( i=0; i<n_src; i++ )
  {
    d = vdot( &l, &v_src[i] );
    d = G_CLAMP( d, -1+FLT_EPSILON, 1-FLT_EPSILON );
    d = acosf(d);
    
    if( dmin>d )
    {
      dmin = d;
      imin = i;
    }
  }

  return imin;
}
