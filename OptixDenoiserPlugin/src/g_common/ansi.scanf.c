/***************************************************************************/
/*	file name : scanf.c						   */
/*									   */
/*	purpose   : strictly conforming implement the ...scanf() function  */
/*		    family						   */
/*		    i.e. ANSI-defined scanf(), sscanf() and fscanf();	   */
/*			 internal vscanf(), vsscanf() and vfscanf().	   */
/*									   */
/*	revision :							   */
/* +--+----------+------------+------------------------------------------+ */
/* |rv| date     |by          | reason                                   | */
/* +--+----------+------------+------------------------------------------+ */
/* |01|09/18/1991|Kong Jiejun | initial release                          | */
/* +--+----------+------------+------------------------------------------+ */
/* |02|10/05/1991|Kong Jiejun | add %p conversion specifier              | */
/* +--+----------+------------+------------------------------------------+ */
/* |03|11/25/1991|Kong Jiejun | mod for strictly ANSI-conforming compiler| */
/* +--+----------+------------+------------------------------------------+ */
/*									   */
/*									   */
/*	Notes :								   */
/* +--+------------------------------------------------------------------+ */
/* |No| Contents                                                         | */
/* +--+------------------------------------------------------------------+ */
/* |01| The '-' character in the scanset is always regarded as a range   | */
/* |  | indicater if its left character is lower than its right part.    | */
/* +--+------------------------------------------------------------------+ */
/***************************************************************************/

/********************************/
/*				*/
/*	data division		*/
/*				*/
/********************************/
#include	<stdio.h>
#include	<ctype.h>
#include	"stdarg.h"

/********************************/
/*	macro define		*/
/********************************/
#define	NO_WIDTH	(-1)

#define	SUPPRESS	1
#define	HALF		2
#define	LONG		4
#define	LONGDOUBLE	8
#define	UNSIGN		0x10
#define	MATCH_FAIL	0x20

/* useful macro append to <ctype.h>	*/
#define	_isoctal(ch)	( (ch)>='0' && (ch)<'8' )

/************************************************/
/*		global declarations		*/
/************************************************/

/********************************/
/*	static variables	*/
/********************************/
static	unsigned _cnt;	/* have read in of source string:for all ...scanf() */
static	char	_init;	/* flag to inform _sgetc cleat its count */
static	unsigned _count;	/* counter for sscanf() */

/********************************/
/* function prototypes		*/
/********************************/
	/* Private Part */
static int	_getc(FILE *), _ungetc(int, FILE *);
static int	_sgetc(char *), _sungetc(int, char *);
static long		_scanl(int (*)(char *), int (*)(int, char *), \
			char *, int, int, unsigned *, unsigned *);
static long double	_scand(int (*)(char *), int (*)(int, char *), \
			char *, int, unsigned *, unsigned *);
static int		_scaner(int (*)(char *), int (*)(int, char *), \
			char *, const char *, va_list);
	/* Public Part */
int	vsscanf(char *, const char *, va_list);

/********************************************************/
/* _sgetc :						*/
/*	function : internal getc function for sscanf()	*/
/*	input : str - the input string			*/
/*	output : OK - the character getted from string	*/
/*		 ERROR - EOF				*/
/*	sub func : None					*/
/********************************************************/
static int
_sgetc(str)
char	*str;
{  register char cch;

   if(!_init)	_count = 0, _init++;
   cch = str[_count++];
   return (!cch? EOF:cch);
}

/****************************************************************/
/* _sungetc :							*/
/*	function : internal ungetc function for sscanf()	*/
/*	input :  ch - the ungetting character			*/
/*		 str - the input string				*/
/*	output : OK - the character ungetted to string		*/
/*		 ERROR - EOF					*/
/*	sub func : None						*/
/****************************************************************/
static int
_sungetc(ch, str)
int	ch;
char	*str;
{  return str[--_count]; }	/*!!!!!!!!!!!????????????*/

/********************************************************/
/* _getc :						*/
/*	function : convert getc from Macro to Function	*/
/*	input : f - the input stream			*/
/*	output : OK - the character getted from stream	*/
/*		 ERROR - EOF				*/
/*	sub func : None if getc is a macro		*/
/********************************************************/
static int
_getc(f)
FILE	*f;
{  return getc(f);  }

/****************************************************************/
/* _ungetc :							*/
/*	function : convert ungetc from Macro to Function	*/
/*	input : ch - the unget character			*/
/*		f  - the input stream				*/
/*	output : OK - the character ungetted to stream		*/
/*		 ERROR - EOF					*/
/*	sub func : None if ungetc is a macro			*/
/****************************************************************/
static int
_ungetc(ch, f)
int	ch;
FILE	*f;
{  return ungetc(ch, f);  }



/*------------------------------------------------------------------------
** v...scanf() function series
------------------------------------------------------------------------*/
/************************************************************************/
/* vsscanf() :								*/
/*	function : scan input(s) from a string with the variable	*/
/*		   argument list replaced by a pointer to the list	*/
/*	input    : s     -- the input string				*/
/*		   fmt   -- the format control string			*/
/*		   argP -- the pointer point to the argument list	*/
/*	output   : OK -- number of input items assigned			*/
/*		   input failuer before any conversion -- EOF		*/
/*		   match failure -- 0					*/
/*	sub func : _scaner()						*/
/************************************************************************/
int
vsscanf(s, fmt, argP)
char		*s;
const char	*fmt;
va_list		argP;
{  return _scaner(_sgetc, _sungetc, s, fmt, argP);
}


/*------------------------------------------------------------------------
** The basic low level function for ...scanf function series
------------------------------------------------------------------------*/
/************************************************************************/
/* _scaner() :								*/
/*	function : scan through the format control string and read in	*/
/*		   data according to conversion specifiers		*/
/*	input    : Get   - the function to get a character from input	*/
/*		   UnGet - the function to unget a character from input	*/
/*		   srceP - the input stream pointer			*/
/*		   fmt   - the format control string			*/
/*		   argP - the argument list pointer			*/
/*	output   : OK -- number of input items assigned			*/
/*		   input failuer before any conversion -- EOF		*/
/*		   match failure -- 0					*/
/*	sub func : _scanl(), _scand()					*/
/************************************************************************/
int
_scaner(Get, UnGet, srceP, fmt, argP)
int		(*Get)(char *), (*UnGet)(int, char *);
char		*srceP;
const char	*fmt;
va_list		argP;
{ 
   register char ch, iCh;	/* scanning chars in <fmt>	*/
   unsigned flgs;		/* record conversion specification	*/
   int	width;			/* record the specification's width field */
   int	base;			/* record the specification's radix field */
   int	count=0;			/* pattern have matched counter	*/
   char	bitSet[97];		/* for scan sets	*/
   long i_in;			/* the readin integer value	*/
   long double	f_in;		/* the readin float value	*/
   char *cptr;
   int  i;
   char exclude;		/* indicates the ^ of scanset	*/

   for(width=NO_WIDTH, flgs=_cnt=_init=0; ; width=NO_WIDTH, flgs=0)
   {
      if( (ch = *fmt++) == '\0' )	return count;   /* end meet */
      if( (ch!='%') || ( (ch = *fmt++)=='%' ) )	/* normal char, include "%%" */
      {
         if(_cnt++, (iCh=(*Get)(srceP))==EOF)	goto ssMatchFail;
         if( !(ch & 0x80) && isspace(ch) )
	/* A directive composed of white space(s) is executed by reading */
	/* input up to the first non-whitespace(which remains unread), or */
	/* until no more chars can be read. */
         {
            while( !(iCh & 0x80) && isspace(iCh) )
               if(_cnt++, (iCh=(*Get)(srceP))==EOF)	goto ssEnd;
            (*UnGet)(iCh, srceP), --_cnt;
         }
         else	if(ch!=iCh)	/* literal match required */
         {
            UnGet(iCh, srceP);
            goto ssMatchFail;
         }
         continue;
      }
      if(ch=='*')	flgs |= SUPPRESS, ch = *fmt++;
	/* assignment suppress	*/

      if((ch>'0') && (ch<='9'))		/* can not be 0	*/
         for(width=ch-'0', ch = *fmt++; isdigit(ch); ch = *fmt++)
            width = width*10 + ch-'0';
	/* width fileld	*/

    ssSwitch:
      switch(ch)
      {
         case 'h':	flgs |= HALF, ch = *fmt++;
			goto ssSwitch;
         case 'l':      flgs |= LONG, ch = *fmt++;
			goto ssSwitch;
         case 'L':      flgs |= LONGDOUBLE, ch = *fmt++;
			goto ssSwitch;
         case 'd':	base = 10;			/* use strtol() */
			goto ssInt;
         case 'D':	base = 10, flgs |= LONG;	/* use strtol() */
			goto ssInt;
         case 'o':	base = 8;	/* ANSI use strtoul(),but	*/
			goto ssInt;	/* OMRON request used as signed	*/
         case 'O':	base = 8, flgs |= LONG;	/* ANSI use strtoul(),but */
			goto ssInt;	/* OMRON request used as signed	*/
         case 'i':	base = 0;		/* use strtol() */
			goto ssInt;
         case 'I':	base = 0, flgs |= LONG;	/* use strtol() */
			goto ssInt;
         case 'u':	base = 10, flgs |= UNSIGN;	/* use strtoul() */
			goto ssInt;
         case 'U':	base = 10, flgs |= UNSIGN | LONG; /* use strtoul() */
			goto ssInt;
         case 'x':	base = 16;	/* ANSI use strtoul(), but */
			goto ssInt;	/* OMRON request used as signed	*/
         case 'X':	base = 16, flgs |= LONG; /* ANSI use strtoul(), but */
			goto ssInt;	/* OMRON request used as signed	*/
         case 'e':	case 'f':	case 'g':
         case 'E':	case 'G':
			goto ssFloat;
         case 's':	goto ssString;
         case 'c':	goto ssChar;
         case 'n':	i_in = _cnt;
			goto ssPutInt;
         case '[':	goto ssScanSet;
         case 'p':	goto ssPointer;
         default:	goto ssEnd; /* %<unknown> regard as fmt str end meet */
      }		/* switch */

    ssInt:
      if(width==NO_WIDTH)	width = 0x7fff;
      i_in = _scanl(Get, UnGet, srceP, base, width, &_cnt, &flgs);
      if(flgs & MATCH_FAIL)	goto ssMatchFail;
    ssPutInt:
      if( !(flgs & SUPPRESS) )		/* not suppressing the item */
      {  if(flgs & UNSIGN)
         {  if(flgs & LONG)	*( va_arg(argP, unsigned long *) ) = i_in;
            else if(flgs&HALF)	*( va_arg(argP, unsigned short *) ) = (unsigned short)i_in;
            else		*( va_arg(argP, unsigned *) ) = i_in;
         }
         else
         {  if(flgs & LONG)		*( va_arg(argP, long *) ) = i_in;
            else if(flgs & HALF)	*( va_arg(argP, short *) ) = (short)i_in;
            else			*( va_arg(argP, int *) ) = i_in;
         }
         if(ch != 'n')	count++;/* %n doesn't inc the assignment cnt */
      }			/* if */
      continue;
    ssPointer:
      i_in = _scanl(Get, UnGet, srceP, 0x10, 0x7fff, &_cnt, &flgs);
      if(flgs & MATCH_FAIL)	goto ssMatchFail;
      if( !(flgs & SUPPRESS) )		/* not suppressing the item */
         *( va_arg(argP, void **) ) = (void *)i_in;
      count++;
      continue;
    ssFloat:
      if(width==NO_WIDTH)	width = 0x7fff;
      f_in = _scand(Get, UnGet, srceP, width, &_cnt, &flgs);
      if(flgs & MATCH_FAIL)	goto ssMatchFail;
      if( !(flgs & SUPPRESS) )		/* not suppressing the item */
      {
         if(flgs & LONG)	    *( va_arg(argP, double *) ) = f_in;
         else if(flgs & LONGDOUBLE) *( va_arg(argP, long double *) ) = f_in;
         else                       *( va_arg(argP, float *) ) = (float)f_in;
         count++;
      }
      continue;
    ssChar:
      if(width==NO_WIDTH)	width = 1;
      if(width)
      {
         _cnt++;
         if( (iCh=(*Get)(srceP))==EOF )	goto ssMatchFail;
      }
      if( !(flgs & SUPPRESS) )	cptr = va_arg(argP, char *), count++;
      while( width-- && (iCh!=EOF) )
      {
         if( !(flgs & SUPPRESS) )	*cptr++ = iCh;
         _cnt++, iCh = (*Get)(srceP);
      }
      UnGet(iCh, srceP), --_cnt;
      continue;
    ssString:
      while(_cnt++, isspace(iCh=(*Get)(srceP)));
      if(width==NO_WIDTH)	width = 0x7fff;
      if(iCh==EOF)	goto ssMatchFail;
      if( !(flgs & SUPPRESS) )	cptr = va_arg(argP, char *), count++;
      while( width-- && !isspace(iCh) && (iCh!=EOF) )
		/* white spaces cause termination */
      {
         if( !(flgs & SUPPRESS) )	*cptr++ = iCh;
         _cnt++, iCh = (*Get)(srceP);
      }
      UnGet(iCh, srceP), --_cnt;
      if( !(flgs & SUPPRESS) )	*cptr = '\0';
      continue;
    ssScanSet:
      if(width==NO_WIDTH)	width = 0x7fff;
      for(i=32; i<128; i++)	bitSet[i-32] = 0;	/* clear include flg */
      if( (iCh = *fmt++)=='^' )	exclude=1, iCh = *fmt++;
      else			exclude=0;
		/* now iCh is the first letter in the scan-set */
      if(iCh==']')	bitSet[iCh-32]=1, iCh = *fmt++; /* ] is in the scanset */

      while(1)
      {
         if( !(ch=iCh) )	goto ssEnd;	/* unexpected end of fmt str */
         bitSet[ch-32] = 1;
         if( (iCh = *fmt++)==']' )	break;	/* meet ']' */
         if( (iCh=='-') && (ch<*fmt) && (*fmt!=']') )
         {
             iCh = *fmt++;	/* now ch is lowerbound, iCh is higherbound */
             while(++ch<iCh)	bitSet[ch-32] = 1;
         }
      }
      if(width)
         if(_cnt++, (iCh=(*Get)(srceP))==EOF)	goto ssMatchFail;
      if( !(flgs & SUPPRESS) )	cptr = va_arg(argP, char *), count++;
      while( width-- && (exclude!=bitSet[iCh-32]) && (iCh!=EOF) )
      {
         if( !(flgs & SUPPRESS) )	*cptr++ = iCh;
         _cnt++, iCh = (*Get)(srceP);
      }
      UnGet(iCh, srceP), --_cnt;
      if( !(flgs & SUPPRESS) )	*cptr = '\0';
      continue;
   }		/* for */
   ssEnd:	return (count? count:EOF);
   ssMatchFail:	return 0;
}

/************************************************************************/
/* _scanl() :								*/
/*	function : scan from input string to construct a long integer	*/
/*		   according to radix <radix>, at most <width> input	*/
/*		   character(s) will be read as effective input.	*/
/*	input : Get   - the function to get a character from input	*/
/*		UnGet - the function to unget a character from input	*/
/*		srceP - the input stream pointer			*/
/*		radix - the radix for input transformation		*/
/*		width - at most this number of input char will be read	*/
/*		cntP  - pointer to effect the readin counter		*/
/*		flgP  - pointer to effect the flag			*/
/*	output : return the same value as strtol().			*/
/*	sub func : None							*/
/* NOTE --- The grammar of input is :					*/
/*	long ::= [isspace]* [sign] [numeral]				*/
/*	numeral ::= { '0' ['x'|'X'] digit [digit]* } | {digit [digit]*}	*/
/*	See the ANSI Standard 3.1.3.2 for details.			*/
/************************************************************************/
static	long
_scanl(Get, UnGet, srceP, radix, width, cntP, flgP)
int	(*Get)(char *);		/* Getchar function for the input stream */
int	(*UnGet)(int, char *);	/* UnGetchar function for the input stream */
char	*srceP;			/* source input string */
int	radix;			/* convert radix */
int	width;			/* maximum convert amount */
unsigned *cntP;			/* input stream readin counter */
unsigned *flgP;
{
   register char ch;
   int	sign;
   long retval=0;
   register int dCnt=0, wsCnt=0;
		/* counters for digits input & white-spaces input */

   while( isspace( ch=(*Get)(srceP) ) )	wsCnt++;
		/* skip head white spaces */
   if( ch=='+' )	sign = 1, dCnt++, ch = (*Get)(srceP);
   else if( ch=='-' )	sign = -1, dCnt++, ch = (*Get)(srceP);
   else			sign = 1;	/* read through sign */

   switch(radix)
   {
      case 0:
         if(ch=='0')
         {
            dCnt++, ch=(*Get)(srceP);
            if( (ch=='x') || (ch=='X') )	/* 16-radix */
            {
               dCnt++, ch = (*Get)(srceP);
               goto rr_16;
            }
            /* then it's 8-radix */
            goto rr_8;
         }
         /* then it's 10-radix */
      case 10:	// rr_10:		/* readin input as radix-10 number */
         if( !isdigit(ch) )
         {
            *flgP |= MATCH_FAIL;
            return 0;
         }
         for(; isdigit(ch) && (dCnt<width); dCnt++, ch = (*Get)(srceP) )
            retval = retval*10 + ch - '0';
         break;
      case 8:	rr_8:		/* readin input as radix-8 number */
         for(; _isoctal(ch) && (dCnt<width); dCnt++, ch = (*Get)(srceP) )
            retval = retval*8 + ch - '0';
         break;
      case 16:	rr_16:		/* readin input as radix-16 number */
         if( !isxdigit(ch) )
         {
            *flgP |= MATCH_FAIL;
            return 0;
         }
         for(; isxdigit(ch) && (dCnt<width); dCnt++, ch = (*Get)(srceP) )
            retval = retval*16+(isdigit(ch)?ch-'0':(islower(ch)?ch-'a'+10:ch-'A'+10));
         break;
      default:	;		/* never enter */
   }
   UnGet(ch, srceP);
   *cntP += dCnt+wsCnt;		/* add counters */
   return retval*sign;
}

/************************************************************************/
/* _scand() :								*/
/*	function : scan from input string to construct a long double,	*/
/*		   at most <width> input character(s) will be read in	*/
/*		   as effective input.					*/
/*	input : Get   - the function to get a character from input	*/
/*		UnGet - the function to unget a character from input	*/
/*		srceP - the input stream pointer			*/
/*		width - at most this number of input char will be read	*/
/*		cntP  - pointer to effect the readin counter		*/
/*		flgP  - pointer to effect the flag			*/
/*	output : return the same value as strtod().			*/
/*	sub func : None							*/
/* NOTE --- The grammar of input is :					*/
/*	float ::= [isspace]*[sign]realnum[exponent]			*/
/*	realnum ::= {digit[digit]*['.'[digit]*]} | {'.'digit[digit]*}	*/
/*	exponent ::= 'e'|'E' [sign] digit [digit]*			*/
/*	See the ANSI Standard 3.1.3.1 for details.			*/
/************************************************************************/
static	long double
_scand(Get, UnGet, srceP, width, cntP, flgP)
int	(*Get)(char *);
int	(*UnGet)(int, char *);
char	*srceP;
int	width;
unsigned *cntP;
unsigned *flgP;
{
   register char ch;
   int	sign, esign;
   long double	retval=0, frac=0, div;
   int	e=0;
   register int dCnt=0, wsCnt=0;
		/* counters for digits input & white-spaces input */

   while( isspace( ch=(*Get)(srceP) ) )	wsCnt++;
		/* skip head white spaces */
   if( ch=='+' )	sign = 1, dCnt++, ch = (*Get)(srceP);
   else if( ch=='-' )	sign = -1, dCnt++, ch = (*Get)(srceP);
   else			sign = 1;	/* read through sign */


   if( isdigit(ch) )
   {
       while( isdigit(ch) && (dCnt<width) )
       {
          retval = retval*10 + ch - '0';
          dCnt++, ch = (*Get)(srceP);
       }
       if(ch=='.')	/* digit-sequence(opt) . */
       {
          dCnt++, ch=(*Get)(srceP), div=10;
          while( isdigit(ch) && (dCnt<width) )
          {
             frac += (ch-'0')/div, div *= 10;
             dCnt++, ch = (*Get)(srceP);
          }
       }
   }
   else if( ch=='.' )
   {
      dCnt++, ch=(*Get)(srceP), div=10;
      if( !isdigit(ch) )
      {
         *flgP |= MATCH_FAIL;
         return 0;
      }
      while( isdigit(ch) && (dCnt<width) )
      {
         frac += (ch-'0')/div, div *= 10;
         dCnt++, ch = (*Get)(srceP);
      }
   }
   else
   {
      *flgP |= MATCH_FAIL;
      UnGet(ch, srceP);
      return 0;
   }
   if(ch=='e' || ch=='E')	/* then the exponent part */
   {
       dCnt++, ch = (*Get)(srceP);
       if( ch=='+' )        esign = 0, dCnt++, ch = (*Get)(srceP);
       else if( ch=='-' )   esign = 1, dCnt++, ch = (*Get)(srceP);
       else                 esign = 0;       /* read through sign */
       if( !isdigit(ch) )
       {
          *flgP |= MATCH_FAIL;
          return 0;
       }
       while( isdigit(ch) && (dCnt<width) )
       {
          e = e*10 + ch - '0';
          dCnt++, ch = (*Get)(srceP);
       }
   }

   retval += frac;
   while(e--)
      if(esign)	retval /= 10;
      else	retval *= 10;
   UnGet(ch, srceP);
   *cntP += dCnt+wsCnt;
   return retval*sign;
}

