#ifndef G_MRZ_H
#define G_MRZ_H

#include "g_vector.h"

class GMrz
{
  public:
    GMrz();
    GMrz( const GMrz &a );
    ~GMrz();



    // save( 'a.txt', 'a', '-ASCII' )
    // load a.txt -ASCII
    void load( const FLOAT3 &nx, const FLOAT3 &ny, const FLOAT3 &nz );
    void load( int number_of_row, int number_of_col, float *data );
    void load( const char *spath );
    void load( int number_of_row, int number_of_col );


    void save( const char *spath ) const;
    void clear();

    GMrz  operator~() const;

    GMrz& operator=( const GMrz &a );
    GMrz  operator+( const GMrz &a ) const;
    GMrz  operator-( const GMrz &a ) const;
    GMrz  operator*( const GMrz &b ) const;

    GMrz& operator=( const float &a );
    GMrz  operator+( const float &a ) const;
    GMrz  operator-( const float &a ) const;
    GMrz  operator*( const float &a ) const;
    GMrz  operator/( const float &a ) const;

    GMrz operator-() const;

    friend GMrz operator+( const float &a, const GMrz &mrz );
    friend GMrz operator-( const float &a, const GMrz &mrz );
    friend GMrz operator*( const float &a, const GMrz &mrz );
    friend GMrz operator/( const float &a, const GMrz &mrz );

    GMrz& operator+=( const GMrz &a );
    GMrz& operator-=( const GMrz &a );

    FLOAT4 operator*( const FLOAT3 &a ) const;
    FLOAT4 operator*( const FLOAT4 &a ) const;
    friend FLOAT4 operator*( const FLOAT3 &a, const GMrz &mrz );
    friend FLOAT4 operator*( const FLOAT4 &a, const GMrz &mrz );


    float det() const;
    void svd( GMrz &u, GMrz &s, GMrz &v ) const;
    void svds( int n_eig, GMrz &u, GMrz &s, GMrz &v ) const;
    float norm() const;
    float condition() const;
    GMrz inverse() const;

    void eig( int n_eig, GMrz &s, GMrz &v ) const;
    GMrz eigs( int n_eig ) const;

    void identity( int n );
    void print( const char *prefix = "%8.4f " ) const;
    void print_info() const;
    GMrz getrow( int row_index ) const;
    void GMrz::setrow( int row_index, const GMrz &vrow );

	  int row;
	  int col;

    float *m;
    float **matrix;

  private:
    static float det( float **mm, int order );
};

#endif