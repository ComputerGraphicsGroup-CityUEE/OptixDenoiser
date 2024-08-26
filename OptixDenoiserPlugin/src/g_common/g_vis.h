#ifndef GVIS_H
#define GVIS_H

#include <float.h>
const float GL_EPSILON = 10000*FLT_EPSILON;

#include "RenderTexture.h"
#include "glme.h"
#include "g_vector.h"



class GVis
{
  public:
    GVis();

      void set_size( int cm_side );
      void set_obj( const char *spath );
      void prepare_render();

      void render( FLOAT3 v, FLOAT3 n, FLOAT3 t1, FLOAT3 t2 );
      BYTE lookup( FLOAT3 v );

      //void vis_cm_debug();
    ~GVis();

    GLMEmodel* glme_model;

  private:
    int vis_cm_side;
    RenderTexture *vis_rt[6];
    float vis_projection[16];

    void GetCmModelView( 
      FLOAT3 v, FLOAT3 n, FLOAT3 t1, FLOAT3 t2, 
      float **cm_modelview
    );
};


#endif
