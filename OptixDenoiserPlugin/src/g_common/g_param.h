#ifndef G_PARAM_H
#define G_PARAM_H

#include "g_vector.h"

  // rotation x-axis by 90 degree
  const float ROTATE_X_AXIS_90[9] = {
    1,  0,  0,
    0,  0,  1,
    0, -1,  0
  };

  // rotation x-axis by 180 degree
  const float ROTATE_X_AXIS_180[9] = {
    1,  0,  0,
    0, -1,  0,
    0,  0, -1
  };
      
  // rotation y-axis by 90 degree
  const float ROTATE_Y_AXIS_90[9] = {
    0,  0,  1,
    0,  1,  0,
   -1,  0,  0
  };

class GParam
{

  public:
    enum P_TYPE{
      FULL_SPHERE,
      HALF_SPHERE_POS_X,
      HALF_SPHERE_POS_Y,
      HALF_SPHERE_POS_Z,
    };
  
    int n_sample;
    P_TYPE m_type;
    virtual FLOAT4* GetSamplesPt() =0;
    void get_dir( FLOAT3 *dir );
    void get_solid_angle( float *sld );

    // mRot is OpenGL 3x3 rotation matrix, ie, assuming row vector.
    virtual void rotate( float *mRot );
    virtual void save_tpmap( const char *spath );
    virtual void save_xfile( const char *spath );
};

#endif
