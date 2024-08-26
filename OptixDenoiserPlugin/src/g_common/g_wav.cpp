#include <stdio.h>
#include <string.h>
#include <string>

#include "g_common.h"

#include "g_wav.h"


GWav::GWav(){  memset(this,0,sizeof(*this));  }
GWav::~GWav()
{
  if(fm)
  {
    free(fm);
    fm = NULL;
  }
  //printf( "~GWav()\n" );
}
float GWav::amax(int i0, int nn) const
{
  int si, ci, ns, nc;
	float amax;
  amax = -FLT_MAX;
	ns = g_min( nsample, i0+nn );
	nc = nchannel;
  for( si=i0; si<ns; si++ )
  for( ci= 0; ci<nc; ci++ )
  {
    if( amax<fabsf(fm[si*nchannel+ci]) )
      amax = fabsf(fm[si*nchannel+ci]);
  }
  return amax;
}
void GWav::mul(float a)
{
  int i;
  for( i=0; i<nsample*nchannel; i++ )
    fm[i] *= a;
}
float GWav::get_duration() const
{
  return float(nsample) / sample_rate;
}

void GWav::load( int nsample0, int nchannel0, int sample_rate0 )
{
  if(fm)
    free(fm);
  nsample = nsample0;
  nchannel = nchannel0;
  sample_rate = sample_rate0;
  fm = (float*) malloc( nsample*nchannel*sizeof(float) );
  memset( fm, 0, nsample*nchannel*sizeof(float) );
}
void GWav::load( int nsample0, int nchannel0, int sample_rate0, void *dat0, int byte_per_val )
{
  load( nsample0, nchannel0, sample_rate0 );

  unsigned char *dat = (unsigned char*) dat0;
  int i, val;
  for( i=0; i<nsample0*nchannel0; i++, dat+=byte_per_val )
  {
    switch(byte_per_val)
    {
      case 1:
      {
        unsigned char p[4] = {0, 0, 0, dat[0]};
        memcpy(&val, p, 4);
        val /= 256*256*256;
        fm[i] = float(val)/0x7F;
      }
      break;
      case 2:
      {
        unsigned char p[4] = {0, 0, dat[0], dat[1]};
        memcpy(&val, p, 4);
        val /= 256*256;
        fm[i] = float(val)/0x7FFF;
      }
      break;
      case 3:
      {
        unsigned char p[4] = {0, dat[0], dat[1], dat[2]};
        memcpy(&val, p, 4);
        val /= 256;
        fm[i] = float(val)/0x7FFFFF;
      }
      break;
      case 4:
      break;
    };
  }
}

GWav::GWav(int nsample0, int nchannel0, int sample_rate0, float *dat0)
{
  memset(this,0,sizeof(*this));
  load( nsample0, nchannel0, sample_rate0, dat0 );
}
GWav::GWav( const GWav &gwav )
{
  memset(this,0,sizeof(*this));
  load( gwav.nsample, gwav.nchannel, gwav.sample_rate, gwav.fm );
}

void GWav::load( int nsample0, int nchannel0, int sample_rate0, float *dat0 )
{
  load( nsample0, nchannel0, sample_rate0 );
  memcpy( fm, dat0, nsample0*nchannel0*sizeof(float) );
}

GWav& GWav::operator=( const GWav &gwav )
{
  if( &gwav==this )
    return *this;
  load( gwav.nsample, gwav.nchannel, gwav.sample_rate, gwav.fm );
  return *this;
}
bool GWav::concatenate( const GWav &tmp, int ti0, int ti1, int ti2 ) 
{
  if( ti2>nsample )
    return false;

  int ti, si;
  for( ti=ti0, si=0; ti<ti1; ti++, si++ )
    fm[ti] = g_lerp( fm[ti], tmp.fm[si], float(si)/(ti1-ti0) );
  for( ; ti<ti2; ti++, si++ )
    fm[ti] = tmp.fm[si];

  return true;
}

GWav GWav::upsample(int sample_rate1) const
{
  GWav res;
  int si, ci;
  int nsample1;

  nsample1 = int(double(nsample)/sample_rate*sample_rate1);
  res.load( nsample1, nchannel, sample_rate1 );
  
  for( si=0; si<nsample1; si++ )
  {
    float uu = double(si)/(nsample1-1) * (nsample-1);
    int   v0 = g_clamp( int(floor(uu)), 0, nsample-1 );
    int   v1 = g_clamp(           v0+1, 0, nsample-1 );
    float vs = g_clamp( uu-v0, 0.f, 1.f );
    for( ci=0; ci<nchannel; ci++ )
      res.fm[si*nchannel+ci] = g_lerp( fm[v0*nchannel+ci], fm[v1*nchannel+ci], vs );
  }
  return res;
}

GWav GWav::get_channel(int ci) const
{
  if( ci<0 || ci>=nchannel )
  {
    printf( L"[Error] GWav::get_channel(), there is only %d channels, channel #%d doesn't exist.\n", nchannel, ci );
    exit(0);
  }
  GWav res;
  int si;
  res.load( nsample, 1, sample_rate );
  for( si=0; si<nsample; si++ )
    res.fm[si*1+0] = fm[si*nchannel+ci];
  return res;
}

GWav GWav::add(const GWav &gwav) const
{
  if( nsample!=gwav.nsample || sample_rate!=gwav.sample_rate || nchannel!=gwav.nchannel )
  {
    printf( L"[Error] GWav::add(), cannot add two wav with different setting\n" );
    exit(0);
  }
  GWav res;
  int i, s;
  res.load( nsample, nchannel, sample_rate );
  for( i=0; i<nsample; i++ )
  for( s=0; s<nchannel; s++ )
    res.fm[i*nchannel+s] = fm[i*nchannel+s] + gwav.fm[i*nchannel+s];
  return res;
}

GWav GWav::sub(const GWav &gwav) const
{
  if( nsample!=gwav.nsample || sample_rate!=gwav.sample_rate || nchannel!=gwav.nchannel )
  {
    printf( L"[Error] GWav::sub(), cannot subtract two wav with different setting\n" );
    exit(0);
  }
  GWav res;
  int si, ci;
  res.load( nsample, nchannel, sample_rate );
  for( si=0; si<nsample; si++ )
  for( ci=0; ci<nchannel; ci++ )
    res.fm[si*nchannel+ci] = fm[si*nchannel+ci] - gwav.fm[si*nchannel+ci];
  return res;
}

float GWav::norm() const
{
  float acc;
  int i;
  for( acc=0, i=0; i<nsample*nchannel; i++ )
    acc += fm[i]*fm[i];
  return sqrtf(acc);
}


void GWav::load( const wchar_t *spath )
{
  if(!fexist(spath) )
  {
    printf( L"[Error] GWav::load(), file %s not found\n", spath );
    exit(0);
  }
  void *dat0 = freadall(spath);
  load_from_memory(dat0);
  free(dat0);
}
void GWav::load( const char *spath )
{
  if(!fexist(spath) )
  {
    printf( "[Error] GWav::load(), file %s not found\n", spath );
    exit(0);
  }
  void *dat0 = freadall(spath);
  load_from_memory(dat0);
  free(dat0);
}
void GWav::load_from_memory( const void *dat0 )
{
  if(fm)
    free(fm);
  unsigned char *dat = (unsigned char*) dat0;

  struct WTag
  {
	  char name[4];
    unsigned int len;
  };

  memcpy( &header, dat, 12 ); dat+=12;

  do
  {
    WTag &tag = *((WTag*)dat);
    if( std::string(tag.name,4)=="fmt " )
    {
      memcpy( &header.fmt_chunk_marker, dat, sizeof(WTag) ); dat+=sizeof(WTag);
      memcpy( &header.format_type, dat, 16 );  dat+=tag.len;
    }
    else if( std::string(tag.name,4)=="data" )
    {
      memcpy( &header.data_chunk_header, dat, sizeof(WTag) ); dat+=sizeof(WTag);
      break;
    }
    else
    {
      dat+=sizeof(WTag);
      dat+=tag.len;
    }
  }while(1);


  

  nsample = (8 * header.data_size) / (header.channels * header.bits_per_sample);
  nchannel = header.channels;
  sample_rate = header.sample_rate;

  //print_info();


  fm = (float*) malloc( nsample*nchannel*sizeof(float) );


  int i;
  int size_of_each_sample = (header.channels * header.bits_per_sample) / 8;
  int bytes_in_each_channel = (size_of_each_sample / header.channels);
  int low_limit  = -(1<<(header.bits_per_sample-1))  ;
  int high_limit =  (1<<(header.bits_per_sample-1))-1;
  //printf( "low_limit %i\n", low_limit );
  //printf( "high_limit %i\n", high_limit );

  unsigned char *data_buffer;
  for( data_buffer = dat, i=0; i<nsample; i++, data_buffer+=size_of_each_sample )
  {
    unsigned int xchannels = 0;
    int data_in_channel = 0;
    int offset = 0; // move the offset for every iteration in the loop below
    for( xchannels = 0; xchannels < header.channels; xchannels++ )
    {
      if( bytes_in_each_channel == 4 )
        data_in_channel = *((int*)&data_buffer[offset]);
      else if( bytes_in_each_channel == 3 )
      {
        unsigned char p[4] = {0, data_buffer[offset+0], data_buffer[offset+1], data_buffer[offset+2]};
        memcpy(&data_in_channel, p, 4);
        data_in_channel /= 256;
      }else if( bytes_in_each_channel == 2 )
        data_in_channel = *((short*)&data_buffer[offset]);
      else if( bytes_in_each_channel == 1 )
      {
        data_in_channel = *((unsigned char*)&data_buffer[offset]);
        data_in_channel += low_limit; //in wave, 8-bit are unsigned, so shifting to signed
      }
      offset += bytes_in_each_channel;
      fm[ i*header.channels + xchannels ] = g_clamp( float(data_in_channel)/high_limit, -1.f, 1.f );
    }
  }
}

void GWav::getblk(GWav &gwav, int i0, int i1) const
{
  if( i0>=i1 )
  {
    printf( "[Error] GWav::getblk(), start index >= end index, i0 %d, i1 %d\n", i0, i1 );
    exit(0);
  }
  if( i0<0 || i0>nsample-1 )
  {
    printf( "[Error] GWav::getblk(), start index out of range [%d,%d-1), i0 %d\n", 0, nsample, i0 );
    exit(0);
  }
  if( i1<0 || i1>nsample )
  {
    printf( "[Error] GWav::getblk(), end index out of range [%d,%d), i1 %d\n", 0, nsample, i1 );
    exit(0);
  }
  gwav.load( i1-i0, nchannel, sample_rate, &fm[i0*nchannel] );
}

void GWav::save(const char* spath) const
{
  struct WTag
  {
	  char name[4];
    unsigned int len;
  };

  int BitsPerSample = 24;

  HEADER header =
  {
    {'R','I','F','F'},
    8, // 86400220
    {'W','A','V','E'},
    {'f','m','t',' '},
    16, // header.length_of_fmt: 16

    1, // header.format_type: 1-PCM
    nchannel, // header.channels: 1
    sample_rate, // header.sample_rate: 48000
    sample_rate * nchannel * BitsPerSample / 8, // header.byterate: 144000
    nchannel * BitsPerSample / 8, // header.block_align: NumChannels * BitsPerSample/8
    BitsPerSample, // header.bits_per_sample: 24

    {'d','a','t','a'},
    nsample * nchannel * BitsPerSample / 8, // header.data_size: NumSamples * NumChannels * BitsPerSample/8 
  };

  int i;
  int size_of_each_sample = (header.channels * header.bits_per_sample) / 8;
  int bytes_in_each_channel = header.bits_per_sample / 8;
  int low_limit  = -(1<<(header.bits_per_sample-1))  ;
  int high_limit =  (1<<(header.bits_per_sample-1))-1;
  //printf( "low_limit %i\n", low_limit );
  //printf( "high_limit %i\n", high_limit );s

  float mval = this->amax();
  if( mval>1 )
  {
    printf( "[Warning] GWav::save(), the wave signal has values bigger than one.\n"
            "          It will be divided by %lf to enforce the signal sits inside \n"
            "          range [-1,1].\n", mval );
  }else
    mval = 1;


  unsigned char *dat0 = (unsigned char*) malloc( nsample * nchannel * BitsPerSample / 8 *sizeof(unsigned char) );

  unsigned char *data_buffer;
  for( data_buffer = dat0, i=0; i<nsample; i++, data_buffer+=size_of_each_sample )
  {
    unsigned int xchannels = 0;
    int data_in_channel = 0;
    int offset = 0; // move the offset for every iteration in the loop below
    for( xchannels = 0; xchannels < header.channels; xchannels++ )
    {
      if( bytes_in_each_channel == 4 )
      {
        data_in_channel = *((int*)&data_buffer[offset]);
        printf( "GWav::save(), 32 bit save not implemented yet.\n" );
        exit(0);
      }
      else if( bytes_in_each_channel == 3 )
      {

        data_in_channel = fm[i * header.channels + xchannels]/mval * high_limit;
        data_in_channel *= 256;
        unsigned char *p = (unsigned char*)&data_in_channel;
        data_buffer[offset+0] = p[1];
        data_buffer[offset+1] = p[2];
        data_buffer[offset+2] = p[3];

      }else if( bytes_in_each_channel == 2 )
      {
        data_in_channel = *((short*)&data_buffer[offset]);
        printf("GWav::save(), 16 bit save not implemented yet.\n");
        exit(0);
      }
      else if( bytes_in_each_channel == 1 )
      {
        data_in_channel = *((unsigned char*)&data_buffer[offset]);
        data_in_channel += low_limit; //in wave, 8-bit are unsigned, so shifting to signed
        printf("GWav::save(), 8 bit save not implemented yet.\n");
        exit(0);
      }
      offset += bytes_in_each_channel;
      //fm[ i*header.channels + xchannels ] = g_clamp( float(data_in_channel)/high_limit, -1.f, 1.f );
    }
  }

  FILE *f0 = fopen( spath, "wb" );

  if( f0==NULL )
  {
    GPath gp = parse_spath(spath);
    printf( "[Error] GWav::save(), cannot open file \"%s.%s\" for writing\n", gp.fname, gp.ename );
    exit(0);
  }

  fwrite(&header, sizeof(header), 1, f0);
  fwrite(dat0, sizeof(unsigned char), nsample * nchannel * BitsPerSample / 8 *sizeof(unsigned char), f0);
  fclose(f0);

  free(dat0);
}



void GWav::print_info() const
{
  //printf("nsample: %d\n", nsample);
  printf("Number of channel: %d\n", nchannel);
  printf("Sampling rate    : %d Hz\n", sample_rate);
  printf("Duration         : %f seconds\n", get_duration() );
  //printf("  (01-04) %s\n", std::string(header.riff,4).c_str() );
  //printf("  (05-08) Overall size: bytes: %d\n", header.overall_size);
  //printf("  (09-12) Wave marker: %s\n", std::string(header.wave,4).c_str());
  //printf("  (13-16) Fmt marker: %s\n", std::string(header.fmt_chunk_marker,4).c_str());
  //printf("  (17-20) Length of Fmt header: %u\n", header.length_of_fmt);
  //char format_name[10] = "";
  //if( header.format_type == 1 )
  //  strcpy(format_name, "PCM");
  //else if( header.format_type == 6 )
  //  strcpy(format_name, "A-law");
  //else if( header.format_type == 7 )
  //  strcpy(format_name, "Mu-law");
  //printf("  (21-22) Format type: %u %s\n", header.format_type, format_name);
  //printf("  (23-24) Channels: %u\n", header.channels);
  //printf("  (25-28) Sample rate: %u\n", header.sample_rate);
  //printf("  (29-32) Byte Rate: %u\n", header.byterate);
  //printf("  (33-34) Block Alignment: %u\n", header.block_align);
  //printf("  (35-36) Bits per sample: %u\n", header.bits_per_sample);
  //printf("  (37-40) Data Marker: %s\n", std::string(header.data_chunk_header,4).c_str());
  //printf("  (41-44) Size of data chunk: %u\n", header.data_size);
}

void GWav::print_header() const
{
  printf("header.riff: %s\n", std::string(header.riff, 4).c_str());
  printf("header.overall_size: %d\n", header.overall_size);
  printf("header.wave: %s\n", std::string(header.wave, 4).c_str());
  printf("header.fmt_chunk_marker: %s\n", std::string(header.fmt_chunk_marker, 4).c_str());
  printf("header.length_of_fmt: %d\n", header.length_of_fmt);

  printf("header.format_type: %d\n", header.format_type);
  printf("header.channels: %d\n", header.channels);
  printf("header.sample_rate: %d\n", header.sample_rate);
  printf("header.byterate: %d\n", header.byterate);
  printf("header.block_align: %d\n", header.block_align);
  printf("header.bits_per_sample: %d\n", header.bits_per_sample);

  printf("header.data_chunk_header: %s\n", std::string(header.data_chunk_header, 4).c_str());
  printf("header.data_size: %d\n", header.data_size);
}



