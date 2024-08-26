#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rmd.h"


#include "g_common.h"
#include "g_pmd.h"

#include "g_basis.h"
  #include "RBFBasis.h"
  #include "SHBasis2.h"
  #include "HDCTBasis.h"
  #include "hpbasis.h"
  #include "riderbasis.h"
  #include "mrbfbasis.h"
#include "poibasis.h"
#include "mpoibasis.h"
#include "spoibasis.h"



GBasis *parse_pmd( const char *spath )
{
  char str[256];
  char str0[256];
  
  GPmd t_pmd;

  BASIS_TYPE bt;
  char color_space_tag[256];
  //char rmd_file[256];
  int n_env;

  FILE *f0 = fopen( spath, "rb" );

  if( f0==NULL )
  {
    printf( "[Error] parse_pmd(), file %s not found\n", spath );
    exit(-1);
  }

    fgets( str, 256, f0 );


    fgets( str, 256, f0 );
    while( str[0]=='#' )
    {
      printf( str );
      fgets( str, 256, f0 );
    }

    sscanf( str, "%s", str0 );
    if( strcmp( str0, "PMD" )==0 )
    {
      fgets( str, 256, f0 );
      sscanf( str, "%s", str0 );
      while( strcmp( str0, "/PMD" )!=0 )
      {
        t_pmd.addline( str );
        fgets( str, 256, f0 );
        sscanf( str, "%s", str0 );
      };
    }else
    {
      printf( "[Error] : open_env(), %s has no header info.\n", spath );
      exit(-1);
    }

    fclose(f0);

    
    
    
    char basis_tag[256];
    t_pmd.query( "BASIS", "%s %i", basis_tag, &n_env);
  
    bt = StrBasis(basis_tag);
  

    switch( bt )
    {
      case BASIS_RBF:
      {
        char dname[256],fname[256],ename[256], rmd_name[256];
          parse_path( spath, dname, fname, ename );

        t_pmd.query( "COLOR_SPACE", "%s", color_space_tag );
        //t_pmd.query( "RMD_FILE", "%s", rmd_file );
        RMD t_rmd;


        sprintf( rmd_name, "%s%s.rmd", dname, fname );
        t_rmd.ReadRMDFile( rmd_name );
        n_env = t_rmd.num_srbf_center;
    
        return new RBFBasis( t_rmd.num_srbf_center, (const float **)t_rmd.srbf_center, t_rmd.srbf_spread );

      }
      case BASIS_SH:
        t_pmd.query( "COLOR_SPACE", "%s", color_space_tag );
        return new SHBasis2( n_env );

      case BASIS_HDCT:
        t_pmd.query( "COLOR_SPACE", "%s", color_space_tag );
        int n_side;
        t_pmd.query( "NSIDE", "%i", &n_side );
        return new HDCTBasis( n_env, n_side );

      case BASIS_HP:
        t_pmd.query( "COLOR_SPACE", "%s", color_space_tag );
        return new HPBasis( n_env );

      case BASIS_MRBF:
        return new MRBFBasis( n_env );

      case BASIS_POI:
        return new POIBasis( n_env, .86 );

      //case BASIS_MPOI:
      //  return new MPOIBasis( n_env );

      case BASIS_SPOI:
        return new SPOIBasis( n_env );

      case BASIS_RIDER:
      {
        char puppet_path[256];
        t_pmd.query( "PUPPET_PATH", "%s", puppet_path );

        int a_grid, b_grid, r_grid;
        t_pmd.query( "EULER_GRID", "%i %i %i", &a_grid, &b_grid, &r_grid );


        if( strcmp( puppet_path, "MAP2" )==0 )
        {
          printf( "[Error] : parse_pmd(), BASIS RIDER MAP2 is not implemented in this version.\n" );
          exit(-1);
        }
        GBasis *puppet_basis = parse_pmd( puppet_path ); 
        return new RiderBasis( puppet_basis, a_grid, b_grid, r_grid );
      }
      default:
        printf( "[Error] : open_env(), unknown basis type.\n" );
        exit(-1);
    }


}

