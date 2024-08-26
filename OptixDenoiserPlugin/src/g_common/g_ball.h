#ifndef G_BALL_H
#define G_BALL_H

#include "g_vector.h"

#define GBALL_UP   1
#define GBALL_DOWN 2
#define GBALL_DRAG 4
#define GBALL_DOCK 8

class GBall
{
  public:

    FLOAT3 vFrom, vTo;
    GQuat qPast, qNow;

    // handle of the arcball window
    int ball_win;

    // handle of the fellow window
    int fellow_win;

    // state value of events registered
    int fellow_state;

    GBall();
    ~GBall();
    void create_window( const char *title=0, int ball_w=200, int ball_h=200 );
    void reset();
    void save( const char *spath );
    void load( const char *spath );
    void set_fellow( int win_id = 0, int state = GBALL_UP | GBALL_DOWN | GBALL_DRAG | GBALL_DOCK );

    // Filling parameter m with 
    // 4x4 rotation matrix (opengl version), ie assumed the following :
    //  - row vector
    //  - right handed orientation
    //  - y-axis point upward, x-axis point rightward (ie, z-axis point out of the screen )
    float* get_rot( float *m_rot );

    static void ball_mousebutton(int button, int state, int x, int y);
    static void ball_mousemove(int x, int y);

  private:

    int drag_state;
    
    static int N_GBALL;
    static GBall* GBALL_LIST[16];
    static void del_ball( GBall *ball );
    static GBall* get_ball( int hrd_win );


    static int N_DOCK;
    static GBall* DOCK_LIST[16];
    static void set_dock( GBall *ball );
    static void del_dock( GBall *ball );
    static GBall *get_dock( int win_id );

    static void ball_display();
    static void ball_reshape( int w, int h );
    static void ball_keyboard( unsigned char key, int x, int y );

    static void DrawAnyArc( const FLOAT3 &vFrom, const FLOAT3 &vTo );
};


//////////////////////////////////////////////////////////////
// Quaternion Sample usage:
//
// FLOAT3 vFrom, vTo;
// GQuat qPast, qNow;
//
// When drag begin (ie. left mouse down),
//   gb->vFrom = MouseOnSphere( 2.f*(x+.5f)/w-1, 2.f*((h-y-1)+.5f)/h-1 );
//
// When drag end ( ie. left mouse up ),
//   qPast = gb->qNow.normalize();
//
// While draging ( ie. mouse move ),
//   gb->vTo = MouseOnSphere( 2.f*(x+.5f)/w-1, 2.f*((h-y-1)+.5f)/h-1 );
//   GQuat qDrag( vTo, vFrom );
//   qNow = qPast * qDrag;
//
//////////////////////////////////////////////////////////////

#endif
