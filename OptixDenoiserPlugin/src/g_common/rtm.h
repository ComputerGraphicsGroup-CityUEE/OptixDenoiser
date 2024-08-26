#ifndef RTM_H
#define RTM_H

#include "g_renderbuffer.h"

#include "g_pfm.h"

#define RTM_MAXBUF 256

class RTm
{
  public:
    int w,h;
    RTm( const RTm &a );
    RTm();
    ~RTm();


    void packed_load( GPf4 &pf4 );
    void packed_load( GPf1 &pf1 );
    void packed_load( int width, int height, float *fm );
    void packed_load( int width, int height, bool state_free = false );
    RTm unpack();
    
    void load( GPfm &pfm );
    void load( GPf1 &pf1 );
    void load( int width, int height, bool state_free = false );

    void save( GPfm &pfm );
    void save( const char *spath );

    RTm& operator=( const RTm &a );

    RTm operator+( const RTm &b ) const;
    RTm operator-( const RTm &b ) const;

    RTm operator*( const RTm &b ) const;
    RTm operator~() const;

    void set( const RTm &a, int x, int y );
    RTm get( int x, int y, int cw, int ch );

    RTm& ground_state();


    static void debug_print();
    static void garbage_collect();
    static void prepare();

    void emux_krc( 
      RTm &a, int ax, int ay,
      RTm &b, int bx, int by,
      int cx, int cy,
      int k 
    );
    RenderBuffer* rt() const;

    void csumx( 
      RTm &a, 
      int ax, int ay, 
      int cx, int cy,
      int w0, int h0, int n
    );


    bool ps;
    bool ts;
    bool ls;
  private:
    int buf_id;



    void kill();


    static float curr_size;
    static float peak_size;
    static int sum_pass;

    static int QueryReadBuf( int id );
    static void FreeBuf( int id );
    static int N_RTBUF;
    static RenderBuffer *RTBUF[RTM_MAXBUF];
    static int RTBUF_read_access[RTM_MAXBUF];

    static void RTM__draw( 
      RenderBuffer *a, int ax, int ay, int aw, int ah, int at,
      RenderBuffer *c, int cx, int cy, int cw, int ch, int ct,
      int anc
    );

    static void RTM__unpack( 
      RenderBuffer *a, int ax, int ay, int aw, int ah, int at,
      RenderBuffer *c, int cx, int cy, int cw, int ch, int ct 
    );

    static void RTM__a_add_b ( RenderBuffer *a, int at, RenderBuffer *b, FLOAT4 sb, RenderBuffer *c, int cw, int ch, int ct );
    static void RTM__a_add_bt( RenderBuffer *a, int at, RenderBuffer *b, FLOAT4 sb, RenderBuffer *c, int cw, int ch, int ct );
    static void RTM__a_add_bt_packed ( RenderBuffer *a, int at, RenderBuffer *b, FLOAT4 sb, RenderBuffer *c, int cw, int ch, int ct );
    static void RTM__at_add_bt_packed( RenderBuffer *a, int at, RenderBuffer *b, FLOAT4 sb, RenderBuffer *c, int cw, int ch, int ct );

    static void RTM__colsumx( 
      RenderBuffer *a, int ax, int ay,
      RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
      int h0, int h1, int anc
      );

    static void RTM__rowsumx( 
      RenderBuffer *a, int ax, int ay,
      RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
      int w0, int h1, int anc
      );

    static void RTM__emux_krc( 
      RenderBuffer *a, int ax, int ay, 
      RenderBuffer *b, int bx, int by, int bw, int bh,
      RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
      int k 
      );

    static void RTM__emux_kcc( 
      RenderBuffer *a, int ax, int ay, 
      RenderBuffer *b, int bx, int by, int bw, int bh,
      RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
      int k 
    );

    static void RTM__emux_kcr( 
      RenderBuffer *a, int ax, int ay, 
      RenderBuffer *b, int bx, int by, int bw, int bh,
      RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
      int k 
      );

    static void RTM__emux_krr( 
      RenderBuffer *a, int ax, int ay, 
      RenderBuffer *b, int bx, int by, int bw, int bh,
      RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
      int k 
      );


    static void RTM__emux_krc_packed( 
      RenderBuffer *a, int ax, int ay, 
      RenderBuffer *b, int bx, int by, int bw, int bh,
      RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
      int k, int anc
      );

    static void RTM__emux_kcc_packed( 
      RenderBuffer *a, int ax, int ay, 
      RenderBuffer *b, int bx, int by, int bw, int bh,
      RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
      int k, int anc
    );

    static void RTM__emux_kcr_packed( 
      RenderBuffer *a, int ax, int ay, 
      RenderBuffer *b, int bx, int by, int bw, int bh,
      RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
      int k, int anc
      );

    static void RTM__emux_krr_packed( 
      RenderBuffer *a, int ax, int ay, 
      RenderBuffer *b, int bx, int by, int bw, int bh,
      RenderBuffer *c, int cx, int cy, int cw, int ch, int ct, 
      int k, int anc
      );

};

#endif
