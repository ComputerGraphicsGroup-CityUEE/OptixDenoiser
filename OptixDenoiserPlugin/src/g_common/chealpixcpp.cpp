// chealpixcpp1.cpp: implementation of the chealpixcpp class.
//
//////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "fitsio.h"
#include "chealpixcpp.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

chealpixcpp::chealpixcpp()
{}

chealpixcpp::~chealpixcpp()
{}


void chealpixcpp::ang2pix_nest( const long nside, double theta, double phi, long *ipix)
{
  /* =======================================================================
   * subroutine ang2pix_nest(nside, theta, phi, ipix)
   * =======================================================================
   * gives the pixel number ipix (NESTED) corresponding to angles theta and phi
   *
   * the computation is made to the highest resolution available (nside=8192)
   * and then degraded to that required (by integer division)
   * this doesn't cost more, and it makes sure that the treatement of round-off 
   * will be consistent for every resolution
   * =======================================================================
   */
  
  double z, za, z0, tt, tp, tmp;
  int    face_num,jp,jm;
  long   ifp, ifm;
  int    ix, iy, ix_low, ix_hi, iy_low, iy_hi, ipf, ntt;
  double piover2 = 0.5*M_PI, pi = M_PI, twopi = 2.0*M_PI;
  int    ns_max = 8192;
  static int x2pix[128], y2pix[128];
  static char setup_done = 0;
    
  if( nside<1 || nside>ns_max ) {
    fprintf(stderr, "nside out of range\n");
    exit(0);
  }
  if( theta<0 || theta>pi ) {
    fprintf(stderr, "theta out of range\n");
    exit(0);
  }
  if( !setup_done ) {
    mk_xy2pix(x2pix,y2pix);
    setup_done = 1;
  }
//  for (i=0;i<128;i++)
//	  printf("%d %d\n",x2pix[i],y2pix[i]);
  z  = cos(theta);
  za = fabs(z);
  z0 = 2./3.;
  if( phi>=twopi ) phi = phi - twopi;
  if( phi<0. )    phi = phi + twopi;
  tt = phi / piover2; /* in [0,4[ */
  
  if( za<=z0 ) { /* equatorial region */
    
    /* (the index of edge lines increase when the longitude=phi goes up) */
    jp = (int)floor(ns_max*(0.5 + tt - z*0.75)); /* ascending edge line index */
    jm = (int)floor(ns_max*(0.5 + tt + z*0.75)); /* descending edge line index */
    
    /* finds the face */
    ifp = jp / ns_max; /* in {0,4} */
    ifm = jm / ns_max;
    
    if( ifp==ifm ) face_num = (int)fmod(ifp,4) + 4; /* faces 4 to 7 */
    else if( ifp<ifm ) face_num = (int)fmod(ifp,4); /* (half-)faces 0 to 3 */
    else face_num = (int)fmod(ifm,4) + 8;           /* (half-)faces 8 to 11 */
    
    ix = (int)fmod(jm, ns_max);
    iy = ns_max - (int)fmod(jp, ns_max) - 1;
  }
  else { /* polar region, za > 2/3 */
    
    ntt = (int)floor(tt);
    if( ntt>=4 ) ntt = 3;
    tp = tt - ntt;
    tmp = sqrt( 3.*(1. - za) ); /* in ]0,1] */
    
    /* (the index of edge lines increase when distance from the closest pole
     * goes up)
     */
    /* line going toward the pole as phi increases */
    jp = (int)floor( ns_max * tp          * tmp ); 

    /* that one goes away of the closest pole */
    jm = (int)floor( ns_max * (1. - tp) * tmp );
    jp = (int)(jp < ns_max-1 ? jp : ns_max-1);
    jm = (int)(jm < ns_max-1 ? jm : ns_max-1);
    
    /* finds the face and pixel's (x,y) */
    if( z>=0 ) {
      face_num = ntt; /* in {0,3} */
      ix = ns_max - jm - 1;
      iy = ns_max - jp - 1;
    }
    else {
      face_num = ntt + 8; /* in {8,11} */
      ix =  jp;
      iy =  jm;
    }
  }
  
  ix_low = (int)fmod(ix,128);
  ix_hi  =     ix/128;
  iy_low = (int)fmod(iy,128);
  iy_hi  =     iy/128;

  ipf = (x2pix[ix_hi]+y2pix[iy_hi]) * (128 * 128)+ (x2pix[ix_low]+y2pix[iy_low]);
  ipf = (long)(ipf / pow(ns_max/nside,2));     /* in {0, nside**2 - 1} */
  *ipix =(long)( ipf + face_num*pow(nside,2)); /* in {0, 12*nside**2 - 1} */
}

void chealpixcpp::ang2pix_ring( const long nside, double theta, double phi, long *ipix)
{
  /*
    c=======================================================================
    c     gives the pixel number ipix (RING) 
    c     corresponding to angles theta and phi
    c=======================================================================
  */
  
  int nl2, nl4, ncap, npix, jp, jm, ipix1;
  double  z, za, tt, tp, tmp;
  int ir, ip, kshift;
  
  double piover2 = 0.5*M_PI;
  double PI=M_PI;
  double twopi=2.0*M_PI;
  double z0=2.0/3.0;
  long ns_max=8192;
  
  if( nside<1 || nside>ns_max ) {
    fprintf(stderr, "nside out of range\n");
    exit(0);
  }
  
  if( theta<0. || theta>PI) {
    fprintf(stderr, "theta out of range\n");
    exit(0);
  }
  
  z = cos(theta);
  za = fabs(z);
  if( phi >= twopi)  phi = phi - twopi;
  if (phi < 0.)     phi = phi + twopi;
  tt = phi / piover2;//  ! in [0,4)
  
  nl2 = 2*nside;
  nl4 = 4*nside;
  ncap  = nl2*(nside-1);// ! number of pixels in the north polar cap
  npix  = 12*nside*nside;
  
  if( za <= z0 ) {
    
    jp = (int)floor(nside*(0.5 + tt - z*0.75)); /*index of ascending edge line*/
    jm = (int)floor(nside*(0.5 + tt + z*0.75)); /*index of descending edge line*/
    
    ir = nside + 1 + jp - jm;// ! in {1,2n+1} (ring number counted from z=2/3)
    kshift = 0;
    if (fmod(ir,2)==0.) kshift = 1;// ! kshift=1 if ir even, 0 otherwise
    
    ip = (int)floor( ( jp+jm - nside + kshift + 1 ) / 2 ) + 1;// ! in {1,4n}
    if( ip>nl4 ) ip = ip - nl4;
    
    ipix1 = ncap + nl4*(ir-1) + ip ;
  }
  else {
    
    tp = tt - floor(tt);//      !MOD(tt,1.d0)
    tmp = sqrt( 3.*(1. - za) );
    
    jp = (int)floor( nside * tp * tmp );// ! increasing edge line index
    jm = (int)floor( nside * (1. - tp) * tmp );// ! decreasing edge line index
    
    ir = jp + jm + 1;//        ! ring number counted from the closest pole
    ip = (int)floor( tt * ir ) + 1;// ! in {1,4*ir}
    if( ip>4*ir ) ip = ip - 4*ir;
    
    ipix1 = 2*ir*(ir-1) + ip;
    if( z<=0. ) {
      ipix1 = npix - 2*ir*(ir+1) + ip;
    }
  }
  *ipix = ipix1 - 1;// ! in {0, npix-1}
}

void chealpixcpp::pix2ang_nest( long nside, long ipix, double *theta, double *phi)
{

  /*
    c=======================================================================
    subroutine pix2ang_nest(nside, ipix, theta, phi)
    c=======================================================================
    c     gives theta and phi corresponding to pixel ipix (NESTED) 
    c     for a parameter nside
    c=======================================================================
  */
    
      int npix, npface, face_num;
      int  ipf, ip_low, ip_trunc, ip_med, ip_hi;
      int     ix, iy, jrt, jr, nr, jpt, jp, kshift, nl4;
      double z, fn, fact1, fact2;
      double piover2=0.5*M_PI;
      int ns_max=8192;
      
      static int pix2x[1024], pix2y[1024];
      //      common /pix2xy/ pix2x, pix2y
      
      int jrll[12], jpll[12];// ! coordinate of the lowest corner of each face
      //      data jrll/2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4/ ! in unit of nside
      //      data jpll/1, 3, 5, 7, 0, 2, 4, 6, 1, 3, 5, 7/ ! in unit of nside/2
      jrll[0]=2;
      jrll[1]=2;
      jrll[2]=2;
      jrll[3]=2;
      jrll[4]=3;
      jrll[5]=3;
      jrll[6]=3;
      jrll[7]=3;
      jrll[8]=4;
      jrll[9]=4;
      jrll[10]=4;
      jrll[11]=4;
      jpll[0]=1;
      jpll[1]=3;
      jpll[2]=5;
      jpll[3]=7;
      jpll[4]=0;
      jpll[5]=2;
      jpll[6]=4;
      jpll[7]=6;
      jpll[8]=1;
      jpll[9]=3;
      jpll[10]=5;
      jpll[11]=7;
      
      
      if( nside<1 || nside>ns_max ) {
	fprintf(stderr, "nside out of range\n");
	exit(0);
      }
      npix = 12 * nside*nside;
      if( ipix<0 || ipix>npix-1 ) {
	fprintf(stderr, "ipix out of range\n");
	exit(0);
      }

      /* initiates the array for the pixel number -> (x,y) mapping */
      if( pix2x[1023]<=0 ) mk_pix2xy(pix2x,pix2y);

      fn = 1.*nside;
      fact1 = 1./(3.*fn*fn);
      fact2 = 2./(3.*fn);
      nl4   = 4*nside;

      //c     finds the face, and the number in the face
      npface = nside*nside;

      face_num = ipix/npface;//  ! face number in {0,11}
      ipf = (int)fmod(ipix,npface);//  ! pixel number in the face {0,npface-1}

      //c     finds the x,y on the face (starting from the lowest corner)
      //c     from the pixel number
      ip_low = (int)fmod(ipf,1024);//       ! content of the last 10 bits
      ip_trunc =   ipf/1024 ;//       ! truncation of the last 10 bits
      ip_med = (int)fmod(ip_trunc,1024);//  ! content of the next 10 bits
      ip_hi  =     ip_trunc/1024   ;//! content of the high weight 10 bits

      ix = 1024*pix2x[ip_hi] + 32*pix2x[ip_med] + pix2x[ip_low];
      iy = 1024*pix2y[ip_hi] + 32*pix2y[ip_med] + pix2y[ip_low];

      //c     transforms this in (horizontal, vertical) coordinates
      jrt = ix + iy;//  ! 'vertical' in {0,2*(nside-1)}
      jpt = ix - iy;//  ! 'horizontal' in {-nside+1,nside-1}

      //c     computes the z coordinate on the sphere
      //      jr =  jrll[face_num+1]*nside - jrt - 1;//   ! ring number in {1,4*nside-1}
      jr =  jrll[face_num]*nside - jrt - 1;
      //      cout << "face_num=" << face_num << endl;
      //      cout << "jr = " << jr << endl;
      //      cout << "jrll(face_num)=" << jrll[face_num] << endl;
      //      cout << "----------------------------------------------------" << endl;
      nr = nside;//                  ! equatorial region (the most frequent)
      z  = (2*nside-jr)*fact2;
      kshift = (int)fmod(jr - nside, 2);
      if( jr<nside ) { //then     ! north pole region
         nr = jr;
         z = 1. - nr*nr*fact1;
         kshift = 0;
      }
      else {
	if( jr>3*nside ) {// then ! south pole region
         nr = nl4 - jr;
         z = - 1. + nr*nr*fact1;
         kshift = 0;
	}
      }
      *theta = acos(z);
      
      //c     computes the phi coordinate on the sphere, in [0,2Pi]
      //      jp = (jpll[face_num+1]*nr + jpt + 1 + kshift)/2;//  ! 'phi' number in the ring in {1,4*nr}
      jp = (jpll[face_num]*nr + jpt + 1 + kshift)/2;
      if( jp>nl4 ) jp = jp - nl4;
      if( jp<1 )   jp = jp + nl4;

      *phi = (jp - (kshift+1)*0.5) * (piover2 / nr);
}

void chealpixcpp::pix2ang_ring( long nside, long ipix, double *theta, double *phi)
{
  /*
    c=======================================================================
    c     gives theta and phi corresponding to pixel ipix (RING) 
    c     for a parameter nside
    c=======================================================================
  */
  
  int nl2, nl4, npix, ncap, iring, iphi, ip, ipix1;
  double  fact1, fact2, fodd, hip, fihip;
  double PI=M_PI;
  //      PARAMETER (pi     = 3.1415926535897932384626434d0)
  //      parameter (ns_max = 8192) ! 2^13 : largest nside available
  
  int ns_max=8192;
  
  if( nside<1 || nside>ns_max ) {
    fprintf(stderr, "nside out of range\n");
    exit(0);
  }
  npix = 12*nside*nside;      // ! total number of points
  if( ipix<0 || ipix>npix-1 ) {
    fprintf(stderr, "ipix out of range\n");
    exit(0);
  }
  
  ipix1 = ipix + 1; // in {1, npix}
  nl2 = 2*nside;
  nl4 = 4*nside;
  ncap = 2*nside*(nside-1);// ! points in each polar cap, =0 for nside =1
  fact1 = 1.5*nside;
  fact2 = 3.0*nside*nside;
  
  if( ipix1 <= ncap ) {  //! North Polar cap -------------
    
    hip   = ipix1/2.;
    fihip = floor(hip);
    iring = (int)floor( sqrt( hip - sqrt(fihip) ) ) + 1;// ! counted from North pole
    iphi  = ipix1 - 2*iring*(iring - 1);
    
    *theta = acos( 1. - iring*iring / fact2 );
    *phi   = (1.*iphi - 0.5) * PI/(2.*iring);
  }
  else if( ipix1 <= nl2*(5*nside+1) ) {//then ! Equatorial region ------
    
    ip    = ipix1 - ncap - 1;
    iring = (int)floor( ip / nl4 ) + nside;// ! counted from North pole
    iphi  = (int)fmod(ip,nl4) + 1;
    
    fodd  = 0.5 * (1 + fmod((double)(iring+nside),2));//  ! 1 if iring+nside is odd, 1/2 otherwise
    *theta = acos( (nl2 - iring) / fact1 );
    *phi   = (1.*iphi - fodd) * PI /(2.*nside);
  }
  else {//! South Polar cap -----------------------------------
    
    ip    = npix - ipix1 + 1;
    hip   = ip/2.;
/* bug corrige floor instead of 1.* */
    fihip = floor(hip);
    iring = (int)floor( sqrt( hip - sqrt(fihip) ) ) + 1;//     ! counted from South pole
    iphi  = (int)(4.*iring + 1 - (ip - 2.*iring*(iring-1)));
    
    *theta = acos( -1. + iring*iring / fact2 );
    *phi   = (1.*iphi - 0.5) * PI/(2.*iring);
  }
}

void chealpixcpp::nest2ring( long nside, long int ipnest, long *ipring)
{
  /*
    c=======================================================================
    subroutine nest2ring(nside, ipnest, ipring)
    c=======================================================================
    c     conversion from NESTED to RING pixel number
    c=======================================================================
  */
      int npix, npface, face_num, ncap, n_before;
      int ipf, ip_low, ip_trunc, ip_med, ip_hi;
      int ix, iy, jrt, jr, nr, jpt, jp, kshift, nl4;
      int ns_max=8192;

      static int pix2x[1024], pix2y[1024];
      static char nest2string_setup_done = 0;

      int jrll[12], jpll[12];// ! coordinate of the lowest corner of each face
      //      data jrll/2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4/ ! in unit of nside
      //      data jpll/1, 3, 5, 7, 0, 2, 4, 6, 1, 3, 5, 7/ ! in unit of nside/2
      jrll[0]=2;
      jrll[1]=2;
      jrll[2]=2;
      jrll[3]=2;
      jrll[4]=3;
      jrll[5]=3;
      jrll[6]=3;
      jrll[7]=3;
      jrll[8]=4;
      jrll[9]=4;
      jrll[10]=4;
      jrll[11]=4;
      jpll[0]=1;
      jpll[1]=3;
      jpll[2]=5;
      jpll[3]=7;
      jpll[4]=0;
      jpll[5]=2;
      jpll[6]=4;
      jpll[7]=6;
      jpll[8]=1;
      jpll[9]=3;
      jpll[10]=5;
      jpll[11]=7;

      if( nside<1 || nside>ns_max ) {
	fprintf(stderr, "nside = %ld out of range\n", nside);
	exit(0);
      }
      npix = 12 * nside*nside;
      if( ipnest<0 || ipnest>npix-1 ) {
	fprintf(stderr, "ipnest out of range\n");
	exit(0);
      }

      //c     initiates the array for the pixel number -> (x,y) mapping
      if ( !nest2string_setup_done ) {
	 mk_pix2xy(pix2x,pix2y);
	 nest2string_setup_done = 1;
      }

      ncap  = 2*nside*(nside-1);// ! number of points in the North Polar cap
      nl4   = 4*nside;

      //c     finds the face, and the number in the face
      npface = nside*nside;
      //cccccc      ip = ipnest - 1         ! in {0,npix-1}

      face_num = ipnest/npface;//  ! face number in {0,11}
      ipf = (int)fmod(ipnest,npface);//  ! pixel number in the face {0,npface-1}

      //c     finds the x,y on the face (starting from the lowest corner)
      //c     from the pixel number
      ip_low = (int)fmod(ipf,1024);//       ! content of the last 10 bits
      ip_trunc =   ipf/1024;//        ! truncation of the last 10 bits
      ip_med = (int)fmod(ip_trunc,1024);//  ! content of the next 10 bits
      ip_hi  =     ip_trunc/1024;//   ! content of the high weight 10 bits

      //      ix = 1024*pix2x[ip_hi] + 32*pix2x[ip_med] + pix2x[ip_low];
      //      iy = 1024*pix2y[ip_hi] + 32*pix2y[ip_med] + pix2y[ip_low];
      ix = 1024*pix2x[ip_hi] + 32*pix2x[ip_med] + pix2x[ip_low];
      iy = 1024*pix2y[ip_hi] + 32*pix2y[ip_med] + pix2y[ip_low];
      //      cout << "ix = " << ix << " iy = " << iy << endl;

      //c     transforms this in (horizontal, vertical) coordinates
      jrt = ix + iy;//  ! 'vertical' in {0,2*(nside-1)}
      jpt = ix - iy;//  ! 'horizontal' in {-nside+1,nside-1}

      //c     computes the z coordinate on the sphere
      //      jr =  jrll[face_num+1]*nside - jrt - 1;//   ! ring number in {1,4*nside-1}
      jr =  jrll[face_num]*nside - jrt - 1;

      nr = nside;//                  ! equatorial region (the most frequent)
      n_before = ncap + nl4 * (jr - nside);
      kshift = (int)fmod(jr - nside, 2);
      if( jr<nside ) {//then     ! north pole region
         nr = jr;
         n_before = 2 * nr * (nr - 1);
         kshift = 0;
      }
      else if( jr>3*nside ) {//then ! south pole region
         nr = nl4 - jr;
         n_before = npix - 2 * (nr + 1) * nr;
         kshift = 0;
      }

      //c     computes the phi coordinate on the sphere, in [0,2Pi]
      //      jp = (jpll[face_num+1]*nr + jpt + 1 + kshift)/2;//  ! 'phi' number in the ring in {1,4*nr}
      jp = (jpll[face_num]*nr + jpt + 1 + kshift)/2;

      if( jp>nl4 ) jp = jp - nl4;
      if( jp<1 )   jp = jp + nl4;

      *ipring = n_before + jp - 1;// ! in {0, npix-1}
}

void chealpixcpp::ring2nest( long nside, long ipring, long *ipnest)
{
  /*
    c=======================================================================
    subroutine ring2nest(nside, ipring, ipnest)
    c=======================================================================
    c     conversion from RING to NESTED pixel number
    c=======================================================================
  */
  
  double fihip, hip;
  int npix, nl2, nl4, ncap, ip, iphi, ipt, ipring1;
  int     kshift, face_num, nr;
  int irn, ire, irm, irs, irt, ifm , ifp;
  int ix, iy, ix_low, ix_hi, iy_low, iy_hi, ipf;
  int ns_max=8192;
  
  static int x2pix[128], y2pix[128];
  //      common    /xy2pix/ x2pix,y2pix

  int jrll[12], jpll[12];// ! coordinate of the lowest corner of each face
  //      data jrll/2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4/ ! in unit of nside
  //      data jpll/1, 3, 5, 7, 0, 2, 4, 6, 1, 3, 5, 7/ ! in unit of nside/2
  jrll[0]=2;
  jrll[1]=2;
  jrll[2]=2;
  jrll[3]=2;
  jrll[4]=3;
  jrll[5]=3;
  jrll[6]=3;
  jrll[7]=3;
  jrll[8]=4;
  jrll[9]=4;
  jrll[10]=4;
  jrll[11]=4;
  jpll[0]=1;
  jpll[1]=3;
  jpll[2]=5;
  jpll[3]=7;
  jpll[4]=0;
  jpll[5]=2;
  jpll[6]=4;
  jpll[7]=6;
  jpll[8]=1;
  jpll[9]=3;
  jpll[10]=5;
  jpll[11]=7;
  
  if( nside<1 || nside>ns_max ) {
    fprintf(stderr, "nside out of range\n");
    exit(0);
  }
  npix = 12 * nside*nside;
  if( ipring<0 || ipring>npix-1 ) {
    fprintf(stderr, "ipring out of range\n");
    exit(0);
  }
  if( x2pix[127]<=0 ) mk_xy2pix(x2pix,y2pix);
  
  nl2 = 2*nside;
  nl4 = 4*nside;
  npix = 12*nside*nside;//      ! total number of points
  ncap = 2*nside*(nside-1);// ! points in each polar cap, =0 for nside =1
  ipring1 = ipring + 1;
  
  //c     finds the ring number, the position of the ring and the face number
  if( ipring1<=ncap ) {//then
    
    hip   = ipring1/2.;
    fihip = (int)floor ( hip );
    irn   = (int)floor( sqrt( hip - sqrt(fihip) ) ) + 1;// ! counted from North pole
    iphi  = ipring1 - 2*irn*(irn - 1);
    
    kshift = 0;
    nr = irn   ;//               ! 1/4 of the number of points on the current ring
    face_num = (iphi-1) / irn;// ! in {0,3}
  }
  else if( ipring1<=nl2*(5*nside+1) ) {//then
    
    ip    = ipring1 - ncap - 1;
    irn   = (int)floor( ip / nl4 ) + nside;//               ! counted from North pole
    iphi  = (int)fmod(ip,nl4) + 1;
    
    kshift  = (int)fmod(irn+nside,2);//  ! 1 if irn+nside is odd, 0 otherwise
    nr = nside;
    ire =  irn - nside + 1;// ! in {1, 2*nside +1}
    irm =  nl2 + 2 - ire;
    ifm = (iphi - ire/2 + nside -1) / nside;// ! face boundary
    ifp = (iphi - irm/2 + nside -1) / nside;
    if( ifp==ifm ) {//then          ! faces 4 to 7
      face_num = (int)fmod(ifp,4) + 4;
    }
    else if( ifp + 1==ifm ) {//then ! (half-)faces 0 to 3
      face_num = ifp;
    }
    else if( ifp - 1==ifm ) {//then ! (half-)faces 8 to 11
      face_num = ifp + 7;
    }
  }
  else {
    
    ip    = npix - ipring1 + 1;
    hip   = ip/2.;
    fihip = floor ( hip );
    irs   = (int)floor( sqrt( hip - sqrt(fihip) ) ) + 1;//  ! counted from South pole
    iphi  = 4*irs + 1 - (ip - 2*irs*(irs-1));
    
    kshift = 0;
    nr = irs;
    irn   = nl4 - irs;
    face_num = (iphi-1) / irs + 8;// ! in {8,11}
  }
  
  //c     finds the (x,y) on the face
  //  irt =   irn  - jrll[face_num+1]*nside + 1;//       ! in {-nside+1,0}
  //  ipt = 2*iphi - jpll[face_num+1]*nr - kshift - 1;// ! in {-nside+1,nside-1}
  irt =   irn  - jrll[face_num]*nside + 1;//       ! in {-nside+1,0}
  ipt = 2*iphi - jpll[face_num]*nr - kshift - 1;


  if( ipt>=nl2 ) ipt = ipt - 8*nside;// ! for the face #4
  
  ix =  (ipt - irt ) / 2;
  iy = -(ipt + irt ) / 2;
  
  ix_low = (int)fmod(ix,128);
  ix_hi  = ix/128;
  iy_low = (int)fmod(iy,128);
  iy_hi  = iy/128;
  //  cout << "ix_low = " << ix_low << " ix_hi = " << ix_hi << endl;
  //  cout << "iy_low = " << iy_low << " iy_hi = " << iy_hi << endl;
  //  ipf =  (x2pix[ix_hi +1]+y2pix[iy_hi +1]) * (128 * 128)
  //    + (x2pix[ix_low+1]+y2pix[iy_low+1]);//        ! in {0, nside**2 - 1}
  ipf =  (x2pix[ix_hi]+y2pix[iy_hi]) * (128 * 128)
    + (x2pix[ix_low]+y2pix[iy_low]);


  //  cout << "ipf = " << ipf << endl;
  //  for( int i(0);i<128;i++ ) cout << x2pix[i] << " || " << y2pix[i] << endl;
  *ipnest = ipf + face_num* nside *nside;//   ! in {0, 12*nside**2 - 1}
  
}

void chealpixcpp::mk_pix2xy(int *pix2x, int *pix2y) {

  /* =======================================================================
   * subroutine mk_pix2xy
   * =======================================================================
   * constructs the array giving x and y in the face from pixel number
   * for the nested (quad-cube like) ordering of pixels
   *
   * the bits corresponding to x and y are interleaved in the pixel number
   * one breaks up the pixel number by even and odd bits
   * =======================================================================
   */

  int i, kpix, jpix, IX, IY, IP, ID;
  for (i = 0; i < 1023; i++) pix2x[i]=0;
  
  for( kpix=0;kpix<1024;kpix++ ) {
    jpix = kpix;
    IX = 0;
    IY = 0;
    IP = 1 ;//              ! bit position (in x and y)
    while( jpix!=0 ){// ! go through all the bits
      ID = (int)fmod(jpix,2);//  ! bit value (in kpix), goes in ix
      jpix = jpix/2;
      IX = ID*IP+IX;
      
      ID = (int)fmod(jpix,2);//  ! bit value (in kpix), goes in iy
      jpix = jpix/2;
      IY = ID*IP+IY;
      
      IP = 2*IP;//         ! next bit (in x and y)
    }
    
    pix2x[kpix] = IX;//     ! in 0,31
    pix2y[kpix] = IY;//     ! in 0,31
  }
  
  /* Later */
  return;
}

void chealpixcpp::mk_xy2pix(int *x2pix, int *y2pix)
{
  /* =======================================================================
   * subroutine mk_xy2pix
   * =======================================================================
   * sets the array giving the number of the pixel lying in (x,y)
   * x and y are in {1,128}
   * the pixel number is in {0,128**2-1}
   *
   * if  i-1 = sum_p=0  b_p * 2^p
   * then ix = sum_p=0  b_p * 4^p
   * iy = 2*ix
   * ix + iy in {0, 128**2 -1}
   * =======================================================================
   */
  int i, K,IP,I,J,ID;
  
  for(i = 0; i < 127; i++) x2pix[i] = 0;
  for( I=1;I<=128;I++ ) {
    J  = I-1;//            !pixel numbers
    K  = 0;//
    IP = 1;//
    truc : if( J==0 ) {
      x2pix[I-1] = K;
      y2pix[I-1] = 2*K;
    }
    else {
      ID = (int)fmod(J,2);
      J  = J/2;
      K  = IP*ID+K;
      IP = IP*4;
      goto truc;
    }
  }     
}

long chealpixcpp::nside2npix(const long nside)
{
  return 12*nside*nside;
}

void chealpixcpp::printerror(int status)
{
    /*****************************************************/
    /* Print out cfitsio error messages and exit program */
    /*****************************************************/


    if (status)
    {
       fits_report_error(stderr, status); /* print error report */

       exit( status );    /* terminate the program, returning error status */
    }
    return;
}

float * chealpixcpp::read_healpix_map(char *infile, long *nside, char *coordsys, char *ordering)
{
  /* Local Declarations */
  long     naxes, *naxis, npix, npercol, irow;
  int      status, hdutype, nfound, anynul;
  float    nulval, *map;
  char     comment[FLEN_COMMENT];
  fitsfile *fptr;

  /* Initializations */
  status = 0;

  /* Open the file */
  if ( fits_open_file(&fptr, infile, READONLY, &status) ) {
    printerror( status );
    return (float *)NULL;
  }

  /* Move to the HDU */
  if ( fits_movabs_hdu(fptr, 2, &hdutype, &status) ) {
    printerror( status );
    return (float *)NULL;
  }
  if (hdutype != BINARY_TBL) {
    fprintf(stderr, "%s (%d): Extension is not binary!\n", __FILE__, __LINE__);
    return (float *)NULL;
  }

  /* Read the sizes of the array */
  if ( fits_read_key_lng(fptr, "NAXIS", &naxes, comment, &status) ) {
    printerror( status );
    return (float *)NULL;
  }
  naxis = (long *)malloc(((size_t)naxes)*sizeof(long));
  if ( fits_read_keys_lng(fptr, "NAXIS", 1, naxes, naxis, &nfound, &status) 
       || nfound != naxes ) {
    printerror( status );
    return (float *)NULL;
  }
  if ( fits_read_key_lng(fptr, "NSIDE", nside, comment, &status) ) {
    printerror(status);
    return (float *)NULL;
  }
  npix = 12*(*nside)*(*nside);
  if ( (npix%naxis[1]) != 0 ) {
    fprintf(stderr, "%s (%d): Problem with npix.\n", __FILE__, __LINE__);
    return (float *)NULL;
  }
  npercol = npix/naxis[1];

  if (fits_read_key(fptr, TSTRING, "COORDSYS",coordsys, comment, &status)) {
    fprintf(stderr, "WARNING: Could not find %s keyword in in file %s\n", 
	    "COORDSYS",infile);
    status = 0;
  }

  if (fits_read_key(fptr, TSTRING, "ORDERING", ordering, comment, &status)) {
    fprintf(stderr, "WARNING: Could not find %s keyword in in file %s\n", 
	    "ORDERING",infile);
    status = 0;
  }

  /* Read the array */
  map = (float *)malloc(((size_t)npix)*sizeof(float));
  nulval = (float)HEALPIX_NULLVAL;
  for (irow = 0; irow < naxis[1]; irow++) {
    if ( fits_read_col(fptr, TFLOAT, 1, irow+1, 1, npercol, &nulval, 
		       &(map[irow*npercol]), &anynul, &status) ) {
      printerror(status);
      return (float *)NULL;
    }
  }

  /* Close the file */
  if ( fits_close_file(fptr, &status) ) {
    printerror( status );
    return (float *)NULL;
  }

  /* Later */
  return map;
}

void chealpixcpp::setCoordSysHP(char *coordsys,char *coordsys9)
{
  strcpy(coordsys9,"C       ");
  
  if(strncmp(coordsys,"G",1)!=0 &&  strncmp(coordsys,"E",1)!=0 &&  strncmp(coordsys,"C",1)!=0 && strncmp(coordsys,"Q",1)!=0)
    fprintf(stderr, "%s (%d): System Cordinates is not correct (Galactic,Ecliptic,Celestial=Equatorial). Celestial system was set.\n", __FILE__, __LINE__);
  
  
/*    if(strncmp(coordsys,"GAL",3)==0) */
/*      strcpy(coordsys9,"G       "); */
/*    else if(strncmp(coordsys,"ECL",3)==0) */
/*      strcpy(coordsys9,"E       "); */
/*    else if(strncmp(coordsys,"EQU",3)==0 || strncmp(coordsys,"CEL",3)==0) */
/*      strcpy(coordsys9,"C       "); */

  if(strncmp(coordsys,"G",1)==0)
    strcpy(coordsys9,"G       ");
  else if(strncmp(coordsys,"E",1)==0)
    strcpy(coordsys9,"E       ");
  else if(strncmp(coordsys,"C",1)==0 || strncmp(coordsys,"Q",1)==0)
    strcpy(coordsys9,"C       ");
}

int chealpixcpp::write_healpix_map( float *signal, long nside, char *filename, char nest, char *coordsys)
{
  /*******************************************************************/
  /* Create a binary table extension                                 */
  /*******************************************************************/
  fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
  int status, hdutype;
  long firstrow, firstelem;
  
  int bitpix   =  SHORT_IMG;
  long naxis   =   0;
  long naxes[] = {0,0};
  
  int tfields   = 1;
  long nrows;
  
  char order[9];                 /* HEALPix ordering */
  char extname[] = "BINTABLE";   /* extension name */
  char *ttype[] = { "SIGNAL" };
  char *tform[] = { "1E" };
  char *tunit[] = { " " };
  char coordsys9[9];
  
  
  /* Calculate the number of pixels in the full map */
  nrows = 12L*nside*nside;
  
  /* initialize status before calling fitsio routines */
  status = 0;
  
  /* create new FITS file */
  if (fits_create_file(&fptr, filename, &status)) 
    fprintf(stderr, "%s (%d): Could not create new fits file.\n", 
	    __FILE__, __LINE__);
  
  if ( fits_create_img(fptr,  bitpix, naxis, naxes, &status) )
    fprintf(stderr, "%s (%d): Could not create new image file.\n", 
	    __FILE__, __LINE__);
  
  if ( fits_write_date(fptr, &status) )
    fprintf(stderr, "%s (%d): Could not add date.\n", 
	    __FILE__, __LINE__);
  
  /* move to 1nd HDU  */
  if ( fits_movabs_hdu(fptr, 1, &hdutype, &status) ) 
    fprintf(stderr, "%s (%d): Could not move to first HDU.\n", 
	    __FILE__, __LINE__);
  
  /* append a new empty binary table onto the FITS file */
  if ( fits_create_tbl( fptr, BINARY_TBL, nrows, tfields, ttype, tform,
			tunit, extname, &status) )
    fprintf(stderr, "%s (%d): Could not create new binary table.\n", 
	    __FILE__, __LINE__);
  
  if (fits_write_key(fptr, TSTRING, "PIXTYPE", "HEALPIX",
		     "HEALPIX Pixelisation", &status))
    fprintf(stderr, "%s (%d): Could not write PIXTYPE keyword.\n", 
	    __FILE__, __LINE__);
  
  if (nest) strcpy(order, "NESTED  ");
  else      strcpy(order, "RING    ");
  if (fits_write_key(fptr, TSTRING, "ORDERING", order, 
		     "Pixel ordering scheme, either RING or NESTED", &status))
    fprintf(stderr, "%s (%d): Could not write ORDERING keyword.\n", 
	    __FILE__, __LINE__);
  
  if (fits_write_key(fptr, TLONG, "NSIDE", &nside,
		     "Resolution parameter for HEALPIX", &status))  
    fprintf(stderr, "%s (%d): Could not write NSIDE keyword.\n", 
	    __FILE__, __LINE__);
 
  /*****NEW*************/
  setCoordSysHP(coordsys,coordsys9);
  if (fits_write_key(fptr, TSTRING, "COORDSYS", coordsys9,"Pixelisation coordinate system", &status))
    fprintf(stderr, "%s (%d): Could not write COORDSYS keyword.\n",__FILE__, __LINE__);

  /*****END**NEW*******/

  if (fits_write_comment(fptr,"           G = Galactic, E = ecliptic, C = celestial = equatorial  ", &status))
    fprintf(stderr, "%s (%d): Could not write COORDSYS explanation keyword.\n", 
	    __FILE__, __LINE__);
  
  firstrow  = 1;  /* first row in table to write   */
  firstelem = 1;  /* first element in row  (ignored in ASCII tables)  */
  
  if (fits_write_col(fptr, TFLOAT, 1, firstrow, firstelem, nrows, signal,
		     &status))
    fprintf(stderr, "%s (%d): Could not write signal.\n", __FILE__, __LINE__);

  /*fits_write_col(fptr, TLONG, 2, firstrow, firstelem, nrows, pixel,  &status);*/
  /*fits_write_col(fptr, TLONG, 3, firstrow, firstelem, nrows, n_obs,  &status);*/
  /*fits_write_col(fptr, TFLOAT, 4, firstrow, firstelem, nrows, serror,&status);*/
  
  if ( fits_close_file(fptr, &status) )       /* close the FITS file */
    fprintf(stderr, "%s (%d): Could not close file.\n", 
	    __FILE__, __LINE__);
  
  return 0;
}

long chealpixcpp::get_fits_size(char *filename, long *nside, char *ordering ){

  fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
  int status, hdutype;
  char comment[80];
  long obs_npix;

  status = 0;

  if ( fits_open_file(&fptr, filename, READONLY, &status) ) 
    printerror( status );

  /* ---------------------------------------- */
  if ( fits_movabs_hdu(fptr, 2, &hdutype, &status) ) /* move to 2nd HDU  */
    printerror( status );

  if (fits_read_key(fptr, TSTRING, "ORDERING", ordering, comment, &status))
    printerror(status);

  if (fits_read_key(fptr, TLONG,"NSIDE"   , nside, comment, &status))  
    printerror(status); 

  if (fits_read_key(fptr, TLONG,"OBS_NPIX", &obs_npix, comment, &status)) {
    obs_npix = 12 * (*nside) * (*nside) ;
    status = 0;
  }


  if ( fits_close_file(fptr, &status) )
    printerror( status );
  
  return obs_npix;
}