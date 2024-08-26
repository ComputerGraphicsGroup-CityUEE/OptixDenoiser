#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "g_common.h"
#include "g_vector.h"
#include "g_param.h"

void GParam::rotate( float *mRot )
{
  FLOAT4 *tL = GetSamplesPt();
  FLOAT3 l1;
  
  for( int i=0; i<n_sample; i++, tL++ )
  {
    FLOAT3 *l0 = (FLOAT3*)tL;
    Mul_R3xM33( mRot, l0, &l1 );
    *l0 = l1;
  }
}

void GParam::save_tpmap( const char *spath )
{
  GPath gp = parse_spath( spath );


  FLOAT4 *tL;
    tL = GetSamplesPt();

  float *tp_angles, *tp;
    tp_angles = (float*) malloc( n_sample * 2*sizeof(float) );
    tp = tp_angles;

  int i;

  for( i=0; i<n_sample; i++, tL++, tp+=2 )
    Float32Angle( tp, tp+1, (FLOAT3*)tL );
  
  if( strcmp( gp.ename, "txt" )==0 )
  {
    tp = tp_angles;
    FILE *f0 = fopen( spath, "wt" );
      for( i=0; i<n_sample; i++, tp+=2 )
        fprintf( f0, "%f %f\n", tp[0], tp[1] );
    fclose(f0);
  }else
  {
    FILE *f0 = fopen( spath, "wb" );
      fwrite( tp_angles, sizeof(float), n_sample * 2, f0 );
    fclose(f0);
  }
  
  free(tp_angles);
}

void GParam::save_xfile( const char *spath )
{

  
  FILE *f0 = fopen( spath, "wt" );

  fprintf( f0, "%s\n", "xof 0303txt 0032" );
  fprintf( f0, "%s\n", "Frame {" );
  fprintf( f0, "%s\n", "  Mesh {" );

  fprintf( f0, "%i;\n", n_sample );

  int i;
  FLOAT4 *tL = GetSamplesPt();
  for( i=0; i<n_sample-1; i++, tL++ )
    fprintf( f0, "%f;%f;%f;,\n", tL->x, tL->y, -tL->z );
    fprintf( f0, "%f;%f;%f;;\n", tL->x, tL->y, -tL->z );

  fprintf( f0, "%i;\n", n_sample-2 );
  for( i=0; i<n_sample-3; i++ )
    fprintf( f0, "3;%i,%i,%i;,\n", i, i+1, i+2 );
    fprintf( f0, "3;%i,%i,%i;;\n", i, i+1, i+2 );

  fprintf( f0, "}\n" );
  fprintf( f0, "}\n" );

  
  fclose(f0);
}

void GParam::get_dir( FLOAT3 *dir )
{
  int i;

  FLOAT4 *l = GetSamplesPt();
  for( i=0; i<n_sample; i++ )
    dir[i] = *( (FLOAT3*) &l[i] );
}

void GParam::get_solid_angle( float *sld )
{
  int i;

  FLOAT4 *l = GetSamplesPt();
  for( i=0; i<n_sample; i++ )
    sld[i] = l[i].w;
}
