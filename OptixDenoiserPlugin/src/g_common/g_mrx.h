#ifndef G_MRX_H
#define G_MRX_H

#include <stdio.h>

class GMrx
{
  public:

    // number of row of matrix, 
    // analogous to height in case of image.
	int row;  

    // number of col of matrix, 
    // analogous to width in case of image.
	int col;

    // a contiguous memory block of matrix elements.
	float *m;

    // 2D pointer of matrix elements,
    // access with matrix[row_idx][col_idx];
    // analogous to matrix[height_idx][width_idx];
	float **matrix;


    GMrx();
    GMrx( int row, int col );
    GMrx( int row, int col, float a );
    ~GMrx();

    void mem_set(
      const float *pSrc, int src_width, int src_height,
      int x, int y, int m_width, int m_height
    );


    void setblk( const float *pBlk, int x, int y, int blkw, int blkh );
    void getblk( float *pBlk, int x, int y, int blkw, int blkh ) const;

    void transpose( const GMrx *A );
    void transpose();
    void clone( const GMrx *A );
    void mul( const GMrx *A, const GMrx *B );

    void xsvds( GMrx *U, float *S, GMrx *V, int K ) const;
    void inverse( const GMrx *A );
    void identity( int n );

	void add( const GMrx *A, const GMrx *B );
	void sub( const GMrx *A, const GMrx *B );
	void scalar_mul( float a );
	void scalar_add( float a );
	void scalar_assign( float a );
    void round();
    void clamp(float a, float b);

    void load( const double *src, int w, int h );
    void load( const float *pSrc, int src_width, int src_height );
    void load( int src_width, int src_height );
    void load( const char *spath );

    float det() const;
    float sum() const;
    float norm() const;
    float condition( float *s ) const;

    float gmax() const;
    float gmin() const;
    float variance() const;
    float mean() const;


    void print( const char *format ) const;
    void print() const { print(NULL); };

    void size() const;


    static char *TXT_FORMAT;

    static void interleave( const GMrx *p_mrx, int num_of_mrx, float *pData );
    static void interleave( const GMrx *p_mrx, int num_of_mrx, GMrx *mrx );

    static void split( GMrx *p_mrx, int num_of_mrx, const float *pData, int tcol, int trow );
    static void split( GMrx *p_mrx, int num_of_mrx, const GMrx *mrx );

    static void block_join( const GMrx *p_mrx, int blk_num_w, int blk_num_h,      GMrx *mrx );
    static void block_split(      GMrx *p_mrx, int blk_num_w, int blk_num_h, const GMrx *mrx );

    static void channel_split(      GMrx *p_mrx, int n_blkw, int n_blkh, const GMrx *mrx );
    static void channel_join( const GMrx *p_mrx, int n_blkw, int n_blkh,       GMrx *mrx );

    static void save_all( const char *spath, const GMrx *p_mrx, int num_of_mrx, const char *mode, const char *s_comment=NULL );
    static void load_all( const char *spath, GMrx **pp_mrx, int *num_of_mrx );
    static void load_all( const char *spath, GMrx *p_mrx, int num_of_mrx );

    static float det( float **mm, int order );

    static void bitalloc( const GMrx *p_mrx, int n_mrx, float bitrate, float *assignbit, float *stepsize );


    void AxBt( const GMrx *A, const GMrx *B );
    void scale( GMrx *a, int a_col, int a_row );
    void flip_vertical(); 	    // flip vertically
    void flip_horizontal(); 	// flip horizontally

    void toDouble( double *a ) const;
    void toDoublet( double *a ) const;

  private:
    void save_pfm( const char *spath ) const;
    void mem_alloc();
    void save( FILE *f0, const char *mode ) const;
    bool _load( FILE *f0, const char *mode );
    static void load_all( const char *spath, GMrx *p_mrx, int num_of_mrx, const char *sid, const char *mode );

  public:
    void save( const char *spath, const char *mode ) const;

};

#endif
