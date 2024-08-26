// chealpixcpp1.h: interface for the chealpixcpp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHEALPIXCPP1_H__6EC8D757_A885_4FE6_AC6C_CFC55147CC67__INCLUDED_)
#define AFX_CHEALPIXCPP1_H__6EC8D757_A885_4FE6_AC6C_CFC55147CC67__INCLUDED_

#ifndef HEALPIX_NULLVAL
#define HEALPIX_NULLVAL (-1.6375e30)
#endif /* HEALPIX_NULLVAL */

#pragma comment (lib, "cfitsio.lib")    /* link with cfitsio lib */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define M_PI 3.141592654

class chealpixcpp  
{
public:
	chealpixcpp();
	virtual ~chealpixcpp();
public:
	/* --------------------- */
	/* Function Declarations */
	/* --------------------- */
	
	/* pixel operations */
	/* ---------------- */
	void ang2pix_nest(const long nside, double theta, double phi, long *ipix);
	void ang2pix_ring(const long nside, double theta, double phi, long *ipix);
	
	void pix2ang_nest(long nside, long ipix, double *theta, double *phi);
	void pix2ang_ring(long nside, long ipix, double *theta, double *phi);
	
	void nest2ring(long nside, long ipnest, long *ipring);
	void ring2nest(long nside, long ipring, long *ipnest);
	
	void mk_pix2xy(int *pix2x, int *pix2y);
	void mk_xy2pix(int *x2pix, int *y2pix);
	
	long nside2npix(const long nside);
	
	/* FITS operations */
	/* --------------- */

	void printerror (int) ;
	float *read_healpix_map (char *, long *, char *, char *) ;

	void setCoordSysHP(char *,char *);
	int write_healpix_map( float *, long , char *, char ,char *) ;

	long get_fits_size(char *, long *, char * ) ;
};

#endif // !defined(AFX_CHEALPIXCPP1_H__6EC8D757_A885_4FE6_AC6C_CFC55147CC67__INCLUDED_)
