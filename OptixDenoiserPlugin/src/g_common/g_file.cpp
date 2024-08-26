#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "g_common.h"
#include "g_file.h"

GFile::GFile(){  
  memset( this, 0, sizeof(GFile) );  
}

GFile::~GFile()
{
  close();
}

void GFile::close()
{
  if(f0) fclose(f0);
  if(_dat) free(_dat);
  memset( this, 0, sizeof(GFile) );
}

void GFile::load( const char *_spath, int _mem_size, int _mem_size_ahead )
{
  if( !fexist(_spath) )
  {
    printf( "[Error], file %s not found...\n", _spath );
    exit(-1);
  }

  close();

  strcpy( spath, _spath );
  flen = fsize( spath );

  if(flen>_mem_size)
  {
    mem_size_ahead = _mem_size_ahead;
    mem_size = _mem_size;

    mem_size_ahead = g_min(mem_size,mem_size_ahead);
  }else
  {
    mem_size_ahead = 0;
    mem_size = flen;
  }

}

char* GFile::getline()
{
  if( f0==0 )
  {
    f0 = fopen( spath, "rb" );
    _dat = (char*) malloc( (mem_size_ahead + mem_size + 1)*sizeof(char) );
    dat = &_dat[mem_size_ahead];

    fread( dat, mem_size, 1, f0 );
    dat[mem_size]=0;
  }

  if( cpos*mem_size+fpos==flen )
    return NULL;

  int upos, vpos;
  for( upos=vpos=fpos; vpos<mem_size; vpos++ )
  {
    if(dat[vpos]=='\r')
      dat[vpos]=0;
    if(dat[vpos]=='\n')
    {
      dat[vpos]=0;
      break;
    }
  }

  if( vpos<mem_size || cpos*mem_size+vpos==flen )
  {
    fpos = vpos+1;
    return &dat[upos];
  }else
  {
    int nbyte = g_min(mem_size,flen-cpos*mem_size);

    memcpy( &_dat[mem_size_ahead-(mem_size-upos)], &dat[upos], (mem_size-upos)*sizeof(char) );
    fread( dat, nbyte, 1, f0 );
    cpos++;
    dat[nbyte]=0;

    for( vpos=0; vpos<nbyte; vpos++ )
    {
      if(dat[vpos]=='\r')
        dat[vpos]=0;
      if(dat[vpos]=='\n')
      {
        dat[vpos]=0;
        break;
      }
    }

    if( vpos<mem_size )
    {
      fpos = vpos+1;
      return &_dat[mem_size_ahead-(mem_size-upos)];
    }else
    {
      printf( "[Error], a single line with more than 100 millions characters.  No way...\n" );
      exit(-1);
    }
  }

}

