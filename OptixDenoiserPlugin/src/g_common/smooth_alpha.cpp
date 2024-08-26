#include "smooth_alpha.h"

void smooth_alpha(
  const unsigned char *pSrc, const int width, const int height,
  const float *kernal, const int w, const int h,
  unsigned char *pDes
){
  int wd2 = w/2;
  int hd2 = h/2;
  
  unsigned char *tDes = pDes;
  for( int j=0; j<height; j++ )
    for( int i=0; i<width; i++, tDes++ )
    {
      float sum = 0;
      const float *tkernal = kernal;
      int t0 = j-hd2; int tlen = t0+h;
      int s0 = i-wd2; int slen = s0+w;
      for( int t=t0; t<tlen; t++ )
        for( int s=s0; s<slen; s++, tkernal++ )
          if( s>=0 && t>=0 && s<width && t<height )
            sum += pSrc[ t*width + s ] * *tkernal;
          
          *tDes = sum>255? 255:(unsigned char)(sum);
    } 
}

