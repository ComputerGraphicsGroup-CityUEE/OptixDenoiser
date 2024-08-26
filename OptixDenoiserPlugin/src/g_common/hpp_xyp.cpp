// hpp_xyp.cpp: implementation of the hpp_xyp class.
//
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include "hpp_xyp.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

hpp_xyp::hpp_xyp()
{}

hpp_xyp::~hpp_xyp()
{}

//-----------------------------------------------------------------------
//	HEALPix index converts to x-y coordinate
//	Maximum of HEAPix index is equal to 65536*65536-1
void hpp_xyp::hp2xy(unsigned int hpindex, unsigned int* xptr, unsigned int* yptr)
{
	int i,j;
	int bitlength;

	bitlength=sizeof(hpindex)*8;
	//Odd bit --> y coordinate
	*yptr=0;
	for (i=0,j=0;i<bitlength;i+=2,j++)
	{
		*yptr+=(hpindex&(1<<i))>>j;
	}
	//Even bit --> x coordinate
	*xptr=0;
	for (i=1,j=1;i<bitlength;i+=2,j++)
	{
		*xptr+=(hpindex&(1<<i))>>j;
	}
}

//-----------------------------------------------------------------------
//	x-y coordinate converts to HEALPix index
//	Maximum of x value is equal to 65535
//	Maximum of y value is equal to 65535
void hpp_xyp::xy2hp(unsigned int x, unsigned int y, unsigned int* hpindexptr)
{
	int i,j;
	int bitlength;
	unsigned int dummyo;
	unsigned int dummye;

	bitlength=sizeof(*hpindexptr)*8;
	//Y coordinate --> odd bit
	dummyo=0;
	for (i=0,j=0;i<bitlength;i+=2,j++)
	{
		dummyo+=(y<<j)&(1<<i);
	}
	//X coordinate --> even bit
	dummye=0;
	for (i=1,j=1;i<bitlength;i+=2,j++)
	{
		dummye+=(x<<j)&(1<<i);
	}
	*hpindexptr=dummyo|dummye;
}

