#ifndef G_WAN_H
#define G_WAN_H


typedef struct _GWan
{
  char spath_pts[256];
  char spath_cs[256];
  int        n_light;
  float energy_light;
  float    *tp_light;
  float   *rgb_light;
  float   *xyz_light;
}GWan;



GWan* gwanLoad( const char *spath_pts=NULL, const char *spath_cs=NULL );
void gwanDelete( GWan *wan );
GLuint gwanList( GWan *wan, float color_scale );







#endif
