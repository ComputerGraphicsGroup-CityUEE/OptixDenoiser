#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glut.h>
#include <cg/cgGL.h>
#include <cg/cg.h>

#include "g_pfm.h"
#include "g_cubemap.h"
#include "filter_cubemap.h"


static void filter_cubemap( const GPfm &cmsrc, const GPfm &cmdir, const GPf1 &cmsld, const GPfm &output_cmdir, float filter_size, GPfm &res );

void gpugen_cubemapmip( const GPfm &cmsrc, GPfm &mip )
{
  GPfm output_cmdir[32];
  GPfm map;
  int i, n, j0;
  GPfm cmdir;
  GPf1 cmsld;

  get_cubemap( cmsrc.w, cmdir );
  get_cubemap_solid_angle( cmsrc.w, cmsld );

  for( n = cmsrc.w/2, i=0; n>0; n/=2, i++ )
    get_cubemap( n, output_cmdir[i] );

  mip.load( cmsrc.w, cmsrc.h * 2 );
  j0=0;

  mip.draw( cmsrc, 0,j0 );
  j0+=cmsrc.h;

  for( n = cmsrc.w/2, i=0; n>0; n/=2, i++ )
  {
    float ss = n*n/4;
    filter_cubemap( cmsrc, cmdir, cmsld, output_cmdir[i], ss, map );
    mip.draw( map, 0,j0 );
    j0+=map.h;
  }

}
void gpugen_cubemapmip( const char *tcm_path, int cmside, const char *spath )
{
  GPfm cmsrc, mip;
  load_tcm( tcm_path, cmsrc, cmside );

  /*for(int i=0;i<cmsrc.w*cmsrc.h;i++)
  {
	  cmsrc.fm[i] = cmsrc.fm[i].norm();
  }*/

  gpugen_cubemapmip( cmsrc, mip );
  mip.save( spath );
}



void gpugen_filter_cubemap( const GPfm &cmsrc, float filter_size, GPfm &res )
{
  GPfm cmdir;
  GPf1 cmsld;
  get_cubemap( cmsrc.w, cmdir );
  get_cubemap_solid_angle( cmsrc.w, cmsld );

  filter_cubemap( cmsrc, cmdir, cmsld, cmdir, filter_size, res );
}

void gpugen_filter_cubemap( const char *tcm_path, float filter_size, const char *spath )
{
  GPfm cmsrc, res;
  load_tcm( tcm_path, cmsrc );
  gpugen_filter_cubemap( cmsrc, filter_size, res );
  res.save( spath );
}



void filter_cubemap( const GPfm &cmsrc, const GPfm &cmdir, const GPf1 &cmsld, const GPfm &output_cmdir, float filter_size, GPfm &res )
{
  char cgsrc[] = 
  "void cgfilter(                                       \n"
  "  float2 tc: TEXCOORD0,                              \n"
  "  uniform sampler2D tex0,                            \n"
  "  uniform float  fls,                                \n"
  "  uniform float3 col,                                \n"
  "  uniform float3 cmr,                                \n"
  "  uniform float  sld,                                \n"
  "  out float4 res : COLOR                             \n"
  "){                                                   \n"
  "  float3 cml = f3tex2D(tex0,tc);                     \n"
  "  float val =  pow( max(dot(cmr,cml),0), fls )*sld;  \n"
  "  res = float4( col*val, val );                      \n"
  "}                                                    \n";

  CGcontext CG_shader = NULL;
  CGprofile CG_fprofile;
  CGprogram   CG_cgfilter;
  CGparameter CG_cgfilter_fls;
  CGparameter CG_cgfilter_col;
  CGparameter CG_cgfilter_cmr;
  CGparameter CG_cgfilter_sld;

  CG_shader = cgCreateContext();
  CG_fprofile = cgGLGetLatestProfile( CG_GL_FRAGMENT );
  cgGLSetOptimalOptions( CG_fprofile );
  CG_cgfilter = cgCreateProgram( CG_shader, CG_SOURCE, cgsrc, CG_fprofile, "cgfilter", 0 );

  cgGLLoadProgram( CG_cgfilter );
  CG_cgfilter_fls = cgGetNamedParameter( CG_cgfilter, "fls" );
  CG_cgfilter_col = cgGetNamedParameter( CG_cgfilter, "col" );
  CG_cgfilter_cmr = cgGetNamedParameter( CG_cgfilter, "cmr" );
  CG_cgfilter_sld = cgGetNamedParameter( CG_cgfilter, "sld" );
  

  GLuint cmdir_id;
  GLuint fbo;
  GLuint fcscn_id;
  GLuint list;
  {
    list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glBegin( GL_QUADS );
    glTexCoord2f( 0,0 );  glVertex2f( 0,0 );
    glTexCoord2f( 1,0 );  glVertex2f( 1,0 );
    glTexCoord2f( 1,1 );  glVertex2f( 1,1 );
    glTexCoord2f( 0,1 );  glVertex2f( 0,1 );
    glEnd();
    glEndList();
  }



  int i;

  glGenTextures( 1, &cmdir_id );
  glBindTexture( GL_TEXTURE_2D, cmdir_id );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, output_cmdir.w, output_cmdir.h, 0, GL_RGB, GL_FLOAT, output_cmdir.fm );


  glGenTextures( 1, &fcscn_id );
  glBindTexture( GL_TEXTURE_2D, fcscn_id );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, output_cmdir.w, output_cmdir.h, 0, GL_RGBA, GL_FLOAT, 0 );


  glGenFramebuffersEXT( 1, &fbo );
  glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fbo );

  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fcscn_id, 0);
  glBindTexture( GL_TEXTURE_2D, cmdir_id );
  cgGLSetParameter1f( CG_cgfilter_fls, filter_size );

  glViewport( 0,0,output_cmdir.w,output_cmdir.h );

  glClearColor( 0,0,0,0 );
  glClear( GL_COLOR_BUFFER_BIT );
  glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,1,0,1);
  glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
    glBlendFunc( GL_ONE, GL_ONE );

    cgGLEnableProfile( CG_fprofile );    
    cgGLBindProgram( CG_cgfilter );
    for( i=0; i<cmsrc.w*cmsrc.h; i++ )
    {
      if( i%100000==0 )
      printf( "[%i/%i] ", i, cmsrc.w*cmsrc.h );

      cgGLSetParameter3fv( CG_cgfilter_col, (float*)&cmsrc.fm[i] );
      cgGLSetParameter3fv( CG_cgfilter_cmr, (float*)&cmdir.fm[i] );
      cgGLSetParameter1f( CG_cgfilter_sld, cmsld.fm[i] );
      glCallList(list);

      if( (i+1)%10000==0 )
      {
        glFinish();
        printf( "." );
      if( (i+1)%100000==0 )
        printf( "\n" );
      }
    }
    printf( "\n" );
    cgGLDisableProfile( CG_fprofile );

    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );

  GPf4 scn;
  

  scn.load( output_cmdir.w, output_cmdir.h );
	glBindTexture( GL_TEXTURE_2D, fcscn_id );
  glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, scn.fm );

  res.load( scn.w, scn.h );
  for( i=0; i<res.w*res.h; i++ )
    res.fm[i] = FLOAT3( scn.fm[i].x, scn.fm[i].y, scn.fm[i].z ) / scn.fm[i].w;

  glDeleteTextures( 1, &cmdir_id );
  glDeleteTextures( 1, &fcscn_id );
  glDeleteFramebuffersEXT( 1, &fbo );
  glDeleteLists(list,1);
  cgDestroyProgram( CG_cgfilter );
  cgDestroyContext(CG_shader);
}

void cpugen_cubemapmip( const char *tcm_path, int cmside, const char *spath )
{
  GPfm cmsrc;
  load_tcm( tcm_path, cmsrc, cmside );

  GPfm cmdir;
  GPf1 cmsld;

  get_cubemap( cmsrc.w, cmdir );
  get_cubemap_solid_angle( cmsrc.w, cmsld );

  int i, j, j0;
  float ss, bb;
  FLOAT3 l;
  GPfm meandir;
  GPfm meanmap;
  GPf1 meansld;
  int n;

  GPfm meanmip;
  meanmip.load( cmsrc.w, cmsrc.h * 2 );
  meanmip.draw( cmsrc, 0,0 );

  //for( n=cmsrc.w/2, j0=cmsrc.w*6; n>0; j0+=n*6, n/=2 )
  for( n=cmsrc.w, j0=0; n>0; j0+=n*6, n/=2 )
  {
    printf( "%i\n", n );
    get_cubemap( n, meandir );
    meanmap.load( meandir.w, meandir.h );
    meansld.load( meandir.w, meandir.h );
    
    ss = n*n/4;

    for( j=0; j<meandir.w*meandir.h; j++ )
    {
      l = meandir.fm[j];
      for( i=0; i<cmsrc.w*cmsrc.h; i++ )
      {
        bb = vdot(l,cmdir.fm[i]);
        if( bb > 0 )
        {
          bb = powf( bb, ss );

          meanmap.fm[j] += bb *cmsrc.fm[i] * cmsld.fm[i];
          meansld.fm[j] += bb *cmsld.fm[i];
        }
      }
      meanmap.fm[j] /= meansld.fm[j];
    }
    meanmip.draw( meanmap, 0,j0 );
  }
  meanmip.save( spath );
}



