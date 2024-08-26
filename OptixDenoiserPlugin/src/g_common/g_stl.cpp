#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

#include "g_common.h"
#include "g_file.h"

#include "g_stl.h"

GStl::GStl(){  memset(this, 0, sizeof(GStl) );  }
GStl::~GStl(){  clear();  }

void GStl::clear()
{
  if( faces )
    free(faces);
  memset(this, 0, sizeof(GStl) );
}

void GStl::save_stl( const char *spath ) const
{
  char stl_header[80] = {0};
  strcpy( stl_header, "Converted using a software written by ma_hty@hotmail.com" );
  int i;
  short attr_size;

  attr_size = 0;
  FILE *f0 = fopen( spath, "wb" );
  fwrite( stl_header, sizeof(char), 80, f0 );
  fwrite( &nfaces, sizeof(int), 1, f0 );
  for( i=0; i<nfaces; i++ )
  {
    fwrite( &faces[4*i], 4*sizeof(FLOAT3), 1, f0 );
    fwrite( &attr_size, sizeof(short), 1, f0 );
  }
  fclose(f0);
}

void GStl::save_stl_ascii( const char *spath ) const
{
  GPath gp = parse_spath( spath );

  int i;
  FILE *f0 = fopen( spath, "wt" );
  fprintf( f0, "solid %s\n", gp.fname );
  for( i=0; i<nfaces; i++ )
  {
    fprintf( f0, "facet normal %f %f %f\n", faces[4*i+0].x, faces[4*i+0].y, faces[4*i+0].z );
    fprintf( f0, "  outer loop\n" );
    fprintf( f0, "    vertex %f %f %f\n", faces[4*i+1].x, faces[4*i+1].y, faces[4*i+1].z );
    fprintf( f0, "    vertex %f %f %f\n", faces[4*i+2].x, faces[4*i+2].y, faces[4*i+2].z );
    fprintf( f0, "    vertex %f %f %f\n", faces[4*i+3].x, faces[4*i+3].y, faces[4*i+3].z );
    fprintf( f0, "  outer endloop\n" );
    fprintf( f0, "endfacet\n" );
    fprintf( f0, "\n" );
  }
  fprintf( f0, "endsolid %s\n", gp.fname );

  fclose(f0);
}


bool readable_char_only( const char *str, int n )
{
  int i;
  for( i=0; i<n; i++ )
  {
    if(str[i]<0)
      return false;
  }
  return true;
}



void GStl::load_stl( const char *spath )
{
  clear();
  if(!fexist(spath))
  {
    printf( "[Error] GStl::load_stl(), file %s not found\n", spath );
    exit(0);
  }
  char str[256];
  FILE *f0 = fopen( spath, "rb" );
    fread( str, sizeof(char), 256, f0 );
  fclose(f0);

  int n = *((int*)&str[80]);
  int file_size = fsize(spath);

  if( memcmp(str,"solid",5*sizeof(char))==0 )
  {
    if( file_size!=80+4+n*(4*3*4+2) )
      load_stl_ascii( spath );
    else if( readable_char_only(&str[80],256-80) )
      load_stl_ascii( spath );
    else
      load_stl_binary( spath );
  }else
    load_stl_binary( spath );
}

void GStl::load_stl_binary( const char *spath )
{
  clear();
  strcpy( stl_path, spath );

  vmin = FLT_MAX;
  vmax = -FLT_MAX;

  char stl_header[80];
  int i;
  short attr_size;

  FILE *f0 = fopen( spath, "rb" );
  fread( stl_header, sizeof(char), 80, f0 );
  fread( &nfaces, sizeof(int), 1, f0 );
  faces = (FLOAT3*) malloc( nfaces * 4 * sizeof(FLOAT3) );

  for( i=0; i<nfaces; i++ )
  {
    fread( &faces[4*i], 4*sizeof(FLOAT3), 1, f0 );
    fread( &attr_size, sizeof(short), 1, f0 );

    FLOAT3 *v = &faces[4*i+1];
    FLOAT3 fn=vcross( v[1]-v[0],  v[2]-v[0] );
    if( fn.norm()>10*FLT_EPSILON )
      faces[4*i+0] = vnormalize(fn);

    vmin = FLOAT3( g_min(v[0].x,vmin.x), g_min(v[0].y,vmin.y), g_min(v[0].z,vmin.z) );
    vmin = FLOAT3( g_min(v[1].x,vmin.x), g_min(v[1].y,vmin.y), g_min(v[1].z,vmin.z) );
    vmin = FLOAT3( g_min(v[2].x,vmin.x), g_min(v[2].y,vmin.y), g_min(v[2].z,vmin.z) );
    vmax = FLOAT3( g_max(v[0].x,vmax.x), g_max(v[0].y,vmax.y), g_max(v[0].z,vmax.z) );
    vmax = FLOAT3( g_max(v[1].x,vmax.x), g_max(v[1].y,vmax.y), g_max(v[1].z,vmax.z) );
    vmax = FLOAT3( g_max(v[2].x,vmax.x), g_max(v[2].y,vmax.y), g_max(v[2].z,vmax.z) );

  }
  fclose(f0);
}

void GStl::load_stl_ascii( const char *spath )
{
  clear();
  strcpy( stl_path, spath );

  const int triangle_per_temp = 16384;

  char *line;
  int cmd, i, iblk, vi;
  FLOAT3 v[3], fn;
  GFile f0;

  vmin = FLT_MAX;
  vmax = -FLT_MAX;
  f0.load( spath );
  faces = (FLOAT3*) malloc( triangle_per_temp * 4 * sizeof(FLOAT3) );
  for( i=0, nfaces=0, iblk=0, vi=0, line=f0.getline(); line=f0.getline(); i++ )
  {
    char *pch;
    {
      cmd=0;
      pch = strtok(line," \t");
      if( pch && strcmp(pch,"facet")==0 ) cmd=2;
      if( pch && strcmp(pch,"vertex")==0 ) cmd=3;
      if( pch && strcmp(pch,"endfacet")==0 ) cmd=4;
    }

    {
      switch(cmd)
      {
        case 2:
          pch = strtok(NULL," \t");
          pch = strtok(NULL," \t");  fn.x = pch? fast_atof32(pch) : 0;
          pch = strtok(NULL," \t");  fn.y = pch? fast_atof32(pch) : 0;
          pch = strtok(NULL," \t");  fn.z = pch? fast_atof32(pch) : 0;
          // pch = strtok(NULL," \t");  sscanf(pch,"%f",&fn.x);
          // pch = strtok(NULL," \t");  sscanf(pch,"%f",&fn.y);
          // pch = strtok(NULL," \t");  sscanf(pch,"%f",&fn.z);
          vi=0;
        break;
        case 3:
          if(vi<3)
          {
            pch = strtok(NULL," \t");  v[vi].x = pch? fast_atof32(pch) : 0;
            pch = strtok(NULL," \t");  v[vi].y = pch? fast_atof32(pch) : 0;
            pch = strtok(NULL," \t");  v[vi].z = pch? fast_atof32(pch) : 0;
            // pch = strtok(NULL," \t");   sscanf(pch,"%f",&v[vi].x);
            // pch = strtok(NULL," \t");   sscanf(pch,"%f",&v[vi].y);
            // pch = strtok(NULL," \t");   sscanf(pch,"%f",&v[vi].z);
            vi++;
          }
        break;
        case 4:
          if( nfaces>=(iblk+1)*triangle_per_temp )
          {
            iblk++;
            faces = (FLOAT3*) realloc( faces, (iblk+1)*triangle_per_temp * 4*sizeof(FLOAT3) );
          }
          faces[4*nfaces+0] = fn;
          faces[4*nfaces+1] = v[0];
          faces[4*nfaces+2] = v[1];
          faces[4*nfaces+3] = v[2];

          fn=vcross( v[1]-v[0],  v[2]-v[0] );
          if( fn.norm()>10*FLT_EPSILON )
            faces[4*nfaces+0] = vnormalize(fn);

          vmin = FLOAT3( g_min(v[0].x,vmin.x), g_min(v[0].y,vmin.y), g_min(v[0].z,vmin.z) );
          vmin = FLOAT3( g_min(v[1].x,vmin.x), g_min(v[1].y,vmin.y), g_min(v[1].z,vmin.z) );
          vmin = FLOAT3( g_min(v[2].x,vmin.x), g_min(v[2].y,vmin.y), g_min(v[2].z,vmin.z) );
          vmax = FLOAT3( g_max(v[0].x,vmax.x), g_max(v[0].y,vmax.y), g_max(v[0].z,vmax.z) );
          vmax = FLOAT3( g_max(v[1].x,vmax.x), g_max(v[1].y,vmax.y), g_max(v[1].z,vmax.z) );
          vmax = FLOAT3( g_max(v[2].x,vmax.x), g_max(v[2].y,vmax.y), g_max(v[2].z,vmax.z) );

          nfaces++;
        break;
      }
    }
  }
  f0.close();

  //faces = (FLOAT3*) realloc( faces, nfaces * 4*sizeof(FLOAT3) );

}
