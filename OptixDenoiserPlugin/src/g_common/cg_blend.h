#ifndef CG_BLEND_H
#define CG_BLEND_H

#include <cg/cg.h>

#include "RenderTexture.h"

class CGBlend
{

  public:
    CGBlend();
    ~CGBlend();

    void load();
    void load( int width, int height, int num_of_buffer=1, bool bDepth=false, bool bStencil=false );
    void prepare( int num_of_pass );
    void begin();
    void end();

    static int best_num( int n_pass );

    int w;
    int h;
    int n_pass;
    int n_buf;

    bool auto_dimensioned;

  private:
    int t_pass;

    CGcontext CG_blendContext;
    CGprofile CG_blendProfile;
    CGprogram CG_blend;

    CGparameter *CG_buf_rt;
    CGparameter  CG_src_rt;

    RenderTexture **buf_rt;
    RenderTexture *in_rt, *src_rt, *des_rt;
};


#endif