
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include <cg/cgGL.h>
#include <cg/cg.h>

#include "RenderTexture.h"

#include "g_common.h"
#include "g_vector.h"
#include "save_screen.h"
#include "g_pfm.h"
//#include "g_ppm.h"
#include "g_pmd.h"

#include "relitdir_psnr.h"

void load_sample_pt( const char *angle_path, const char *prefix_src_path, char **&src_path, FLOAT3 *&v_src, int &n_src )
{
  int i;
  char str[256];
  float theta, phi;
  
  FILE *f0 = fopen( angle_path, "rb" );

  if( f0 == NULL )
  {
    printf( "[Error] load_sample_pt(), %s not found.\n", angle_path );
    exit(-1);
  }


    n_src = 0;
    while( fgets( str, 256, f0 ) )
      n_src++;

  fseek( f0, 0, SEEK_SET );
    v_src = (FLOAT3*) malloc( n_src * sizeof(FLOAT3) );
    for( i=0; i<n_src; i++ )
    {
      fgets( str, 256, f0 );
      sscanf( str, "%f %f", &theta, &phi );
      Angle2Float3( theta, phi, &v_src[i] );
    }
  fclose(f0);

  src_path = (char**) malloc( n_src * sizeof(char*) );

  for( i=0; i<n_src; i++ )
  {
    src_path[i] = (char*) malloc( 256 * sizeof(char) );
    sprintf( src_path[i], prefix_src_path, i );
  }
}

void relitdir_psnr( const char *src_info_path, int n_basis, void (*func)(FLOAT3) )
{
  int i;

  char angle_path[256], prefix_src_path[256];
  char data_set_name[256];
  char spath[256];
    FLOAT3 *v_src;
    char **src_path;
    int n_src;
    int w,h;

  GPmd pmd;
    pmd.load( src_info_path );
    pmd.query( "DATA_SET_NAME", "%s", data_set_name );
    pmd.query( "WIDTH", "%i", &w );
    pmd.query( "HEIGHT", "%i", &h );
    pmd.query_path( "ANGLE_PATH", angle_path );
    pmd.query_path( "SRC_PREFIX", prefix_src_path );


  GPfm src, des;
    des.load( w, h );

  float mse, psnr;

  load_sample_pt( angle_path, prefix_src_path, src_path, v_src, n_src );


  mse = 0;
  for( i=0; i<n_src; i++ )
  {
    if( (i+1)%100 == 0 )
      printf( "[%i/%i]\n", i+1, n_src );

    src.load( src_path[i] );

    func(v_src[i]);
    save_screen4( (float*)des.fm, 0,0, w,h );

    mse += GPfm::mse( src, des );
  }
  mse /= n_src;

  psnr = g_psnr(mse, 1 );

  printf( "mse = %f\n", mse );
  printf( "psnr = %f\n", psnr );

  sprintf( spath, "%s_report.txt", data_set_name );
  FILE *f1 = fopen( spath, "at" );
    fprintf( f1, "%i\t%f\t%f\n", n_basis, mse, psnr );
  fclose(f1);



  free(v_src);
  for( i=0; i<n_src; i++ )
    free( src_path[i] );
  free( src_path );
}
  /*
void cgErrorCallback2()
{
  CGerror lastError = cgGetError();
  if(lastError)
  {
    printf("%s\n\n", cgGetErrorString(lastError));
    printf("%s\n", cgGetLastListing(cg_context));
    printf("Cg error, exiting...\n");
    exit(0);
  }
}
*/

char *relitdir_psnr_src = "\
void main(                                                        //\n\
  float2 tex_cord : TEXCOORD0,                                    //\n\
                                                                  //\n\
  uniform samplerRECT src,                                        //\n\
  uniform samplerRECT des,                                        //\n\
  uniform samplerRECT acc,                                        //\n\
                                                                  //\n\
  out float3 color : COLOR0                                       //\n\
){                                                                //\n\
                                                                  //\n\
  float3 v0, v;                                                   //\n\
    v0 = f3texRECT( acc, tex_cord );                              //\n\
    v = f3texRECT( src, tex_cord ) - f3texRECT( des, tex_cord );  //\n\
    v = v*v;                                                      //\n\
                                                                  //\n\
  color = v0 + v;                                                 //\n\
                                                                  //\n\
}                                                                 //\n\
";



void relitdir_cg_psnr( const char *src_info_path, int n_basis, void (*func)(FLOAT3) )
{
  glewInit();

  int i;

  char angle_path[256], prefix_src_path[256];
  char data_set_name[256];
  char spath[256];
    FLOAT3 *v_src;
    char **src_path;
    int n_src;
    int w,h;

  GPmd pmd;
    pmd.load( src_info_path );
    pmd.query( "DATA_SET_NAME", "%s", data_set_name );
    pmd.query( "WIDTH", "%i", &w );
    pmd.query( "HEIGHT", "%i", &h );
    pmd.query_path( "ANGLE_PATH", angle_path );
    pmd.query_path( "SRC_PREFIX", prefix_src_path );




  CGcontext cg_context;
    CGprofile cg_fragment_profile;
      CGprogram cg_fragment;

        CGparameter cg_src;
        CGparameter cg_des;
        CGparameter cg_acc;

        GLuint gl_src;
        GLuint gl_des;
        RenderTexture *mse_rt, *acc_rt, *tmp_rt;
        RenderTexture *buf_rt[2];

  cg_context = cgCreateContext();
    cg_fragment_profile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
    cgGLSetOptimalOptions(cg_fragment_profile);

  //cgSetErrorCallback(cgErrorCallback2);

/*
  cg_fragment = 
    cgCreateProgramFromFile(
    cg_context, CG_SOURCE,
    "relitdir_psnr.cg", cg_fragment_profile, 
    NULL, 0 );
*/
  cg_fragment = 
    cgCreateProgram(
    cg_context, CG_SOURCE,
    relitdir_psnr_src, cg_fragment_profile, 
    NULL, 0 );


  cgGLLoadProgram(cg_fragment);

  cg_src = cgGetNamedParameter( cg_fragment, "src" );
  cg_des = cgGetNamedParameter( cg_fragment, "des" );
  cg_acc = cgGetNamedParameter( cg_fragment, "acc" );

  load_sample_pt( angle_path, prefix_src_path, src_path, v_src, n_src );

    buf_rt[0] = new RenderTexture( w, h );
    buf_rt[0]->Initialize(true, false, false, false, false, 32,32,32,32);
    buf_rt[0]->BeginCapture();
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    buf_rt[0]->EndCapture();

    buf_rt[1] = new RenderTexture( w, h );
    buf_rt[1]->Initialize(true, false, false, false, false, 32,32,32,32);
    buf_rt[1]->BeginCapture();
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    buf_rt[1]->EndCapture();

    glGenTextures( 1, &gl_src );
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, gl_src );
    glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures( 1, &gl_des );
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, gl_des );
    glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  GPPm src;
  mse_rt = buf_rt[0];
  acc_rt = buf_rt[1];
  for( i=0; i<n_src; i++ )
  {
    if( (i+1)%100 == 0 )
      printf( "[%i/%i]\n", i+1, n_src );

    src.load( src_path[i] );
    src.flip_vertical();
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, gl_src );
    glTexImage2D( GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, src.bm );

    func(v_src[i]);
    glBindTexture( GL_TEXTURE_RECTANGLE_NV, gl_des );
    glCopyTexImage2D( GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0,0,w,h,0 );


    mse_rt->BeginCapture();

      glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
          gluOrtho2D( 0, w, 0, h );

      cgGLSetTextureParameter( cg_acc, acc_rt->GetTextureID() );
      cgGLEnableTextureParameter( cg_acc );

      cgGLSetTextureParameter( cg_src, gl_src );
      cgGLEnableTextureParameter( cg_src );

      cgGLSetTextureParameter( cg_des, gl_des );
      cgGLEnableTextureParameter( cg_des );

      cgGLEnableProfile( cg_fragment_profile );
      cgGLBindProgram( cg_fragment );
      glBegin(GL_QUADS);
        glTexCoord2i( 0 ,0 );    glVertex2i( 0, 0 );
        glTexCoord2i( w, 0 );    glVertex2i( w, 0 );
        glTexCoord2i( w, h );    glVertex2i( w, h );
        glTexCoord2i( 0, h );    glVertex2i( 0, h );
      glEnd();

      cgGLDisableTextureParameter(cg_src);
      cgGLDisableTextureParameter(cg_des);
      cgGLDisableTextureParameter(cg_acc);
      cgGLDisableProfile( cg_fragment_profile );

    mse_rt->EndCapture();

    tmp_rt = mse_rt;
    mse_rt = acc_rt;
    acc_rt = tmp_rt;
  }



  GPfm mse_pfm;
    mse_pfm.load( w, h );
    acc_rt->BeginCapture();
      glReadPixels( 0, 0, w, h, GL_RGB, GL_FLOAT, mse_pfm.fm );
    acc_rt->EndCapture();
    mse_pfm.flip_vertical();
    mse_pfm.mul( 1.0/n_src );

  float mse, psnr;

    FILE *f0;
    i=0;
    do{
      sprintf( spath, "%s_mse%03i_%04i.pfm", data_set_name, n_basis, i++ );
      f0=fopen(spath, "rb" );
      if( f0!=NULL)
      fclose(f0);
    }while(f0!=NULL);



    mse_pfm.save( spath );
    FLOAT3 mse3f = mse_pfm.vmean();
    mse = (mse3f.x + mse3f.y + mse3f.z)/3;
    psnr = g_psnr(mse, 1 );



  printf( "mse = %f\n", mse );
  printf( "psnr = %f\n", psnr );

  sprintf( spath, "%s_report.txt", data_set_name );
  FILE *f1 = fopen( spath, "at" );
    fprintf( f1, "%i\t%f\t%f\n", n_basis, mse, psnr );
  fclose(f1);


  cgDestroyProgram( cg_fragment );
  cgDestroyContext( cg_context );

  glDeleteTextures( 1, &gl_src );
  glDeleteTextures( 1, &gl_des );

  free(v_src);
  for( i=0; i<n_src; i++ )
    free( src_path[i] );
  free( src_path );
  delete buf_rt[0];
  delete buf_rt[1];
}

void relitdir_capture( const char *src_info_path, int n_basis, int idx, void (*func)(FLOAT3) )
{
  char angle_path[256], prefix_src_path[256], spath[256];
  char data_set_name[256];
    FLOAT3 *v_src;
    char **src_path;
    int n_src;
    int w,h;
    int i;

  GPmd pmd;
    pmd.load( src_info_path );
    pmd.query( "DATA_SET_NAME", "%s", data_set_name );
    pmd.query( "WIDTH", "%i", &w );
    pmd.query( "HEIGHT", "%i", &h );
    pmd.query_path( "ANGLE_PATH", angle_path );
    pmd.query_path( "SRC_PREFIX", prefix_src_path );

    load_sample_pt( angle_path, prefix_src_path, src_path, v_src, n_src );



    func(v_src[idx]);

    FILE *f0;
    
    i=0;
    do{
      sprintf( spath, "%s_c%03i_%04i_%04i.ppm", data_set_name, n_basis, idx, i++ );
      f0=fopen(spath, "rb" );
      if( f0!=NULL)
      fclose(f0);
    }while(f0!=NULL);

    save_screen( spath );


  free(v_src);
  for( i=0; i<n_src; i++ )
    free( src_path[i] );
  free( src_path );
}

void relitdir_capture_blk( const char *src_info_path, int n_basis, int idx, void (*func)(FLOAT3), int w, int h, int blk_idx )
{
  char angle_path[256], prefix_src_path[256];
    FLOAT3 *v_src;
    char **src_path;
    int n_src;
  char data_set_name[256];
  char spath[256];
  int i,j;
  int w0, h0;

  GPmd pmd;
    pmd.load( src_info_path );
    pmd.query( "DATA_SET_NAME", "%s", data_set_name );
    pmd.query( "WIDTH", "%i", &w0 );
    pmd.query( "HEIGHT", "%i", &h0 );
    pmd.query_path( "ANGLE_PATH", angle_path );
    pmd.query_path( "SRC_PREFIX", prefix_src_path );
    load_sample_pt( angle_path, prefix_src_path, src_path, v_src, n_src );

    if( blk_idx == -1 )
    {
      int nw, nh;
        nw = w0/w;
        nh = h0/h;
      GPfm tmp, cap;
        cap.load( w0, h0 );

      for( j=0; j<nh; j++ )
        for( i=0; i<nw; i++ )
        {
          sprintf( spath, "c%04i_%02i.ppm", idx, j*nw+i  );
          tmp.load( spath );
          //tmp.flip_vertical();
          cap.draw( tmp, i*w, j*h );
        }

      sprintf( spath, "%s_c%03i_%04i.ppm", data_set_name, n_basis, idx  );
      //cap.flip_vertical();
      cap.save( spath, "ppm" );

      return;
    }



    func(v_src[idx]);
    sprintf( spath, "c%04i_%02i.ppm", idx, blk_idx  );
    save_screen( spath );
  free(v_src);
  for( i=0; i<n_src; i++ )
    free( src_path[i] );
  free( src_path );
}

void relitdir_cg_psnr_blk( 
  const char *src_info_path, 
  int n_basis,
  void (*func)(FLOAT3),
  int x, int y, int w, int h, int blk_idx
){
  int i;
  char data_set_name[256];
  char angle_path[256], prefix_src_path[256];
    FLOAT3 *v_src;
    char **src_path;
    int n_src;
    int w0, h0;

  GPmd pmd;
    pmd.load( src_info_path );
    pmd.query( "DATA_SET_NAME", "%s", data_set_name );
    pmd.query_path( "ANGLE_PATH", angle_path );
    pmd.query_path( "SRC_PREFIX", prefix_src_path );
    pmd.query( "WIDTH", "%i", &w0 );
    pmd.query( "HEIGHT", "%i", &h0 );

    load_sample_pt( angle_path, prefix_src_path, src_path, v_src, n_src );

  if( blk_idx == -1 )
  {
      int i,j;
      char spath[256];
      GPfm tmp, mse_pfm;
        mse_pfm.load( w0, h0 );

      int nw, nh;
        nw = w0/w;
        nh = h0/h;


      for( j=0; j<nh; j++ )
        for( i=0; i<nw; i++ )
        {
          sprintf( spath, "mse_%02i.pfm", j*nw+i );
          tmp.load( spath );
          //tmp.flip_vertical();
          mse_pfm.draw( tmp, i*w, j*h );
        }

      sprintf( spath, "%s_mse%03i.pfm", data_set_name, n_basis );
      //mse_pfm.flip_vertical();
      mse_pfm.save( spath );

      float mse, psnr;
        FLOAT3 mse3f = mse_pfm.vmean();
        mse = (mse3f.x + mse3f.y + mse3f.z)/3;
        psnr = g_psnr(mse, 1 );



      printf( "mse = %f\n", mse );
      printf( "psnr = %f\n", psnr );

      sprintf( spath, "%s_report.txt", data_set_name );
      FILE *f1 = fopen( spath, "at" );
        fprintf( f1, "%i\t%f\t%f\n", n_basis, mse, psnr );
      fclose(f1);
  
    return;
  
  }



  glewInit();
  CGcontext cg_context;
    CGprofile cg_fragment_profile;
      CGprogram cg_fragment;

        CGparameter cg_src;
        CGparameter cg_des;
        CGparameter cg_acc;

        GLuint gl_src;
        GLuint gl_des;
        RenderTexture *mse_rt, *acc_rt, *tmp_rt;
        RenderTexture *buf_rt[2];

  cg_context = cgCreateContext();
    cg_fragment_profile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
    cgGLSetOptimalOptions(cg_fragment_profile);

  //cgSetErrorCallback(cgErrorCallback2);

/*
  cg_fragment = 
    cgCreateProgramFromFile(
    cg_context, CG_SOURCE,
    "relitdir_psnr.cg", cg_fragment_profile, 
    NULL, 0 );
*/
  cg_fragment = 
    cgCreateProgram(
    cg_context, CG_SOURCE,
    relitdir_psnr_src, cg_fragment_profile, 
    NULL, 0 );


  cgGLLoadProgram(cg_fragment);

  cg_src = cgGetNamedParameter( cg_fragment, "src" );
  cg_des = cgGetNamedParameter( cg_fragment, "des" );
  cg_acc = cgGetNamedParameter( cg_fragment, "acc" );

    buf_rt[0] = new RenderTexture( w, h );
    buf_rt[0]->Initialize(true, false, false, false, false, 32,32,32,32);
    buf_rt[0]->BeginCapture();
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    buf_rt[0]->EndCapture();

    buf_rt[1] = new RenderTexture( w, h );
    buf_rt[1]->Initialize(true, false, false, false, false, 32,32,32,32);
    buf_rt[1]->BeginCapture();
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    buf_rt[1]->EndCapture();

    glGenTextures( 1, &gl_src );
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, gl_src );
    glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures( 1, &gl_des );
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, gl_des );
    glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  GPPm src, blk;
  mse_rt = buf_rt[0];
  acc_rt = buf_rt[1];
  for( i=0; i<n_src; i++ )
  {
    if( (i+1)%100 == 0 )
      printf( "[%i/%i]\n", i+1, n_src );

    //src.load( src_path[i] );
    //src.flip_vertical();
    //src.getblk( blk, x,y, w,h );
    blk.load( src_path[i], x, y, w, h );
    blk.flip_vertical();

    glBindTexture(GL_TEXTURE_RECTANGLE_NV, gl_src );
    glTexImage2D( GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, blk.bm );

    func(v_src[i]);
    glBindTexture( GL_TEXTURE_RECTANGLE_NV, gl_des );
    glCopyTexImage2D( GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0,0,w,h,0 );


    mse_rt->BeginCapture();

      glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
          gluOrtho2D( 0, w, 0, h );

      cgGLSetTextureParameter( cg_acc, acc_rt->GetTextureID() );
      cgGLEnableTextureParameter( cg_acc );

      cgGLSetTextureParameter( cg_src, gl_src );
      cgGLEnableTextureParameter( cg_src );

      cgGLSetTextureParameter( cg_des, gl_des );
      cgGLEnableTextureParameter( cg_des );

      cgGLEnableProfile( cg_fragment_profile );
      cgGLBindProgram( cg_fragment );
      glBegin(GL_QUADS);
        glTexCoord2i( 0 ,0 );    glVertex2i( 0, 0 );
        glTexCoord2i( w, 0 );    glVertex2i( w, 0 );
        glTexCoord2i( w, h );    glVertex2i( w, h );
        glTexCoord2i( 0, h );    glVertex2i( 0, h );
      glEnd();

      cgGLDisableTextureParameter(cg_src);
      cgGLDisableTextureParameter(cg_des);
      cgGLDisableTextureParameter(cg_acc);
      cgGLDisableProfile( cg_fragment_profile );

    mse_rt->EndCapture();

    tmp_rt = mse_rt;
    mse_rt = acc_rt;
    acc_rt = tmp_rt;
  }



  GPfm mse_pfm;
    mse_pfm.load( w, h );
    acc_rt->BeginCapture();
      glReadPixels( 0, 0, w, h, GL_RGB, GL_FLOAT, mse_pfm.fm );
    acc_rt->EndCapture();
    mse_pfm.flip_vertical();
    mse_pfm.mul( 1.0/n_src );

    char spath[256];
      sprintf( spath, "mse_%02i.pfm", blk_idx );
    mse_pfm.save( spath );

  cgDestroyProgram( cg_fragment );
  cgDestroyContext( cg_context );

  glDeleteTextures( 1, &gl_src );
  glDeleteTextures( 1, &gl_des );

  free(v_src);
  for( i=0; i<n_src; i++ )
    free( src_path[i] );
  free( src_path );
  delete buf_rt[0];
  delete buf_rt[1];
}

