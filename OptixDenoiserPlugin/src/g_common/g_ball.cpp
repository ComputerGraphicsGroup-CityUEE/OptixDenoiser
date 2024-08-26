#include <windows.h>
#include <stdio.h>
#include <math.h>

#include <GL/glut.h>


#include "g_ball.h"

int GBall::N_GBALL = 0;
GBall* GBall::GBALL_LIST[16];

int GBall::N_DOCK = 0;
GBall* GBall::DOCK_LIST[16];


GBall::GBall()
{
  ball_win = 0;     // handle of the arcball window
  fellow_win = 0;   // handle of the fellow window
  fellow_state = GBALL_UP | GBALL_DOWN | GBALL_DRAG;
  drag_state = -1;
}

void GBall::set_fellow( int win_id, int state )
{
  fellow_win = win_id ? win_id : glutGetWindow();
  fellow_state = state;


  if( state & GBALL_DOCK )
  {
    set_dock( this );
    int hwin = glutGetWindow();

      glutSetWindow(fellow_win);
        glutMouseFunc( ball_mousebutton );
        glutMotionFunc( ball_mousemove );

    if(hwin)
      glutSetWindow(hwin);
  }
}

GBall::~GBall()
{
  del_ball( this );
  del_dock( this );
}

void GBall::save( const char *spath )
{
  FILE *f0 = fopen( spath, "wb" );
    fwrite( &qPast, sizeof(GQuat), 1, f0 );
    fwrite( &qNow, sizeof(GQuat), 1, f0 );
  fclose(f0);
}

void GBall::load( const char *spath )
{
  int t_win = glutGetWindow();
  
  FILE *f0 = fopen( spath, "rb" );
    if( f0==NULL ) return;
    fread( &qPast, sizeof(GQuat), 1, f0 );
    fread( &qNow, sizeof(GQuat), 1, f0 );
  fclose(f0);

  glutSetWindow(ball_win);
  glutPostRedisplay();
  glutSetWindow(fellow_win);
  glutPostRedisplay();

  glutSetWindow(t_win);
}


void GBall::create_window( const char *title, int ball_w, int ball_h )
{

  int curr_win_idx = glutGetWindow();

  if( ball_win )
    return;

  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);


  char str[256];

  if( title==NULL )
    sprintf( str, "GBall#%02i", N_GBALL );
  else
    strcpy( str, title );

  ball_win = glutCreateWindow( str );
    glutPositionWindow(1024-4-ball_w,22);
    glutReshapeWindow( ball_w, ball_h );

    glutMouseFunc(ball_mousebutton);
    glutMotionFunc(ball_mousemove);
    glutDisplayFunc(ball_display);
    glutReshapeFunc(ball_reshape);
    glutKeyboardFunc(ball_keyboard);


    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHT0);
    glClearColor(0.0, 0.0, 0.0, 0.0);

    float spec[] = {.8, .8, .8, 1.0};
    float amb[]  = {.3, .3, .3, 1.0};
    float diff[] = {.5, .5, .5, 1.0};
    float shin[]  = {10};
      glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
      glMaterialfv(GL_FRONT, GL_DIFFUSE,  diff);
      glMaterialfv(GL_FRONT, GL_AMBIENT,  amb);
      glMaterialfv(GL_FRONT, GL_SHININESS, shin);

    glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(-1.0,  1.0,  -1.0,  1.0,  -1.0,  100000.0);
  

  GBALL_LIST[N_GBALL] = this;
  N_GBALL++;

  if(curr_win_idx)
  glutSetWindow( curr_win_idx );
}


void GBall::ball_mousebutton(int button, int state, int x, int y)
{
  GBall *gb;
  int hwin = glutGetWindow();

  if( gb=get_dock(hwin) )
  {
  }else
  {
    gb = get_ball( hwin );
    if(gb==NULL) return;
    set_dock( gb );
  }

  if( button == GLUT_LEFT_BUTTON )
  {
    if( state == GLUT_DOWN )
    {
      int w,h;
        w = glutGet( GLUT_WINDOW_WIDTH );
        h = glutGet( GLUT_WINDOW_HEIGHT );


      gb->vFrom = MouseOnSphere( x,y, w,h );
      gb->vTo = gb->vFrom;



      gb->drag_state = GLUT_LEFT_BUTTON;

      if( gb->fellow_win && (gb->fellow_state & GBALL_DOWN) )
      {
        glutSetWindow(gb->fellow_win);
        glutPostRedisplay();
      }

    }

    if( state == GLUT_UP )
    {
      // we normalize qNow before assign to qPast simply
      // to avoid propagation of numurical error.  
      // Nothing more than that.
      gb->qPast = gb->qNow.normalize();
      gb->drag_state = -1;

      glutSetWindow(gb->ball_win);
      glutPostRedisplay();

      if( gb->fellow_win && (gb->fellow_state & GBALL_UP) )
      {
        glutSetWindow(gb->fellow_win);
        glutPostRedisplay();
      }
    }
  }

}

void GBall::ball_mousemove(int x, int y)
{
  GBall *gb;
  int hwin = glutGetWindow();

  if( gb=get_dock(hwin) )
  {
  }else
  {
    gb = get_ball( hwin );
    if(gb==NULL) return;
    set_dock( gb );
  }

  if( gb->drag_state == GLUT_LEFT_BUTTON )
  {
    int w,h;
      w = glutGet( GLUT_WINDOW_WIDTH );
      h = glutGet( GLUT_WINDOW_HEIGHT );

      gb->vTo = MouseOnSphere( x,y, w,h );

    // ATTENTION :
    //
    // Quaternion multiplication DO NOT COMMUTE, therefore
    // the order of multiplication is IMPORTANT.
    //
    // In case of OpenGL, row vector assumed, next transform matrix
    // is concatenated on the right hand side, ie, M0 * M1 * M2 * ...
    //
    // As Quaternion is just a fancy representation of rotation matrix 
    // only, it should be concatenated in the same fashion as 
    // ordinary tranform matrix.
    //
    // For OpenGL, right handed concatenation please.  ^O^ok
    //
    GQuat qDrag( gb->vTo, gb->vFrom );
    gb->qNow = gb->qPast * qDrag;

    glutSetWindow(gb->ball_win);
    glutPostRedisplay();
  
    if( gb->fellow_win && (gb->fellow_state & GBALL_DRAG) )
    {
      glutSetWindow(gb->fellow_win);
      glutPostRedisplay();
    }
  }
}

void GBall::ball_keyboard( unsigned char key, int x, int y )
{
  GBall *gb = get_ball( glutGetWindow() );
  if(gb==NULL) return;

  switch( key )
  {
    case 27:
      gb->reset();
    break;
  };
}

void GBall::ball_display()
{
  GBall *gb = get_ball( glutGetWindow() );
  if(gb==NULL) return;

    float m[16];
      gb->qNow.matrix( m );


  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  double ball_r = .75;
  double spot_r = .0666 * ball_r;
  double teapot_r = .333 * ball_r;

  float v[4] = { m[8], m[9], m[10], 0 };
  glLightfv(GL_LIGHT0, GL_POSITION, v);    

  glEnable(GL_LIGHTING);

    glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
    glPushMatrix();
      //glCullFace(GL_BACK);
      glMultMatrixf( m );
      glTranslatef(0.0, 0.0, ball_r);
      glutSolidSphere( spot_r, 20, 20);
    glPopMatrix();

    glPushMatrix();

    //glCullFace(GL_FRONT);
      glutSolidTeapot( teapot_r );
    glPopMatrix();

  glDisable(GL_LIGHTING);

  glPushMatrix();

    //(~gb->qView).matrix( vm );
    //  glMultMatrixf( vm );

  
    glScalef( ball_r, ball_r, ball_r );
    static GLUquadricObj *qobj = gluNewQuadric();
    gluQuadricDrawStyle(qobj, (GLenum)GLU_SILHOUETTE);
    glColor3ub(255,  255,  255);
    gluDisk( qobj, 0.0, 1.0, 32, 1);

    if( gb->drag_state != -1 )
    {
      glColor3ub(127,  255,  255);
      DrawAnyArc( gb->vFrom, gb->vTo );
    }

  glPopMatrix();
  
  glutSwapBuffers();
  
}

void GBall::ball_reshape( int w, int h )
{
  glViewport(0, 0, w, h );
  ball_display();
}

float* GBall::get_rot( float *m_rot )
{
  qNow.matrix( m_rot );
  return m_rot;
}

void GBall::reset()
{
  int t_win = glutGetWindow();
  
  qNow = qPast = GQuat();
  glutSetWindow(ball_win);
  glutPostRedisplay();
  glutSetWindow(fellow_win);
  glutPostRedisplay();
  
  glutSetWindow(t_win);
}

GBall* GBall::get_ball( int hrd_win )
{
  for( int i=0; i<N_GBALL; i++ )
  {
    if( hrd_win==GBALL_LIST[i]->ball_win )
      return GBALL_LIST[i];
  }

  return NULL;
}

void GBall::del_ball( GBall *ball )
{
  int i,j;
  if( ball->ball_win )
  {
    glutDestroyWindow( ball->ball_win );
    for( i=0; i<N_GBALL; i++ )
      if( GBALL_LIST[i]->ball_win == ball->ball_win )
      {
        N_GBALL--;
        for( j=i; j<N_GBALL; j++ )
          GBALL_LIST[j] = GBALL_LIST[j+1];
      }
  }
}

void GBall::set_dock( GBall *ball )
{
  int i;
  for( i=0; i<N_DOCK; i++ )
  {
    if( DOCK_LIST[i]->fellow_win == ball->fellow_win )
    {
      DOCK_LIST[i] = ball;
      return;
    }
  }
  DOCK_LIST[N_DOCK++] = ball;
}

void GBall::del_dock( GBall *ball )
{
  int i,j;
  for( i=0; i<N_DOCK; i++ )
  {
    if( DOCK_LIST[i] == ball )
    {
      N_DOCK--;
      for( j=i; j<N_DOCK; j++ )
        DOCK_LIST[j] = DOCK_LIST[j+1];
      return;
    }
  }
}

GBall *GBall::get_dock( int win_id )
{
  int i;
  for( i=0; i<N_DOCK; i++ )
    if( DOCK_LIST[i]->fellow_win == win_id )
      return DOCK_LIST[i];
  return NULL;
}


// Draw an arc defined by its ends.
#define LG_NSEGS 4
#define NSEGS (1<<LG_NSEGS)
void GBall::DrawAnyArc( const FLOAT3 &vFrom, const FLOAT3 &vTo )
{
  int i;
  FLOAT3 pts[NSEGS+1];
  double dot;

  pts[0] = vFrom;
  pts[1] = pts[NSEGS] = vTo;

  for (i=0; i<LG_NSEGS; i++) 
    pts[1] = vbisect(pts[0], pts[1]);

  dot = 2.0*vdot(&pts[0], &pts[1]);

  for (i=2; i<NSEGS; i++) 
  {
    pts[i] = pts[i-1] * dot  -  pts[i-2] ;
  }

  // OGLXXX for multiple, independent line segments: use GL_LINES
  glBegin(GL_LINE_STRIP);
  for (i=0; i<=NSEGS; i++)
    glVertex3fv((float *)&pts[i]);
  glEnd();
}


