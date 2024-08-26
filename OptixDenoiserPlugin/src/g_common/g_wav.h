#ifndef G_WAV_H
#define G_WAV_H

class GWav
{
  public:
    GWav();
    ~GWav();
    void load( int nsample0, int nchannel0, int sample_rate0 );
    void load( int nsample0, int nchannel0, int sample_rate0, float *dat0 );
    void load( int nsample0, int nchannel0, int sample_rate0, void *dat0, int byte_per_val );
    void save(const char* spath) const;

    void load( const char *spath );
    void load( const wchar_t *spath );
    void load_from_memory( const void *dat0 );

    GWav(int nsample0, int nchannel0, int sample_rate0, float *dat0);
    GWav(const GWav &gwav);
    GWav& operator=( const GWav &gwav );
    bool concatenate( const GWav &gwav, int ti0, int ti1, int ti2 ); 

    GWav add(const GWav &gwav) const;
    GWav sub(const GWav &gwav) const;
    float norm() const;
    GWav get_channel(int ci) const;
    GWav upsample(int sample_rate1) const;

    void mul(float a);
    void print_info() const;
    float amax( int i0=0, int nn=INT_MAX ) const;
    float get_duration() const;
    void print_header() const;

    void getblk( GWav &gwav, int i0, int i1 ) const;


    float *fm;
    int nsample;
    int nchannel;
    int sample_rate;

  #pragma pack( push, 1 )
  struct HEADER
  {
	  char riff[4];										// RIFF string
	  unsigned int overall_size	;			// overall size of file in bytes
	  char wave[4];										// WAVE string
	  char fmt_chunk_marker[4];				// fmt string with trailing null char
	  unsigned int length_of_fmt;			// length of the format data

	  unsigned short format_type;			// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	  unsigned short channels;				// no.of channels
	  unsigned int sample_rate;				// sampling rate (blocks per second)
	  unsigned int byterate;					// SampleRate * NumChannels * BitsPerSample/8
	  unsigned short block_align;			// NumChannels * BitsPerSample/8
	  unsigned short bits_per_sample;	// bits per sample, 8- 8bits, 16- 16 bits etc

	  char data_chunk_header[4];		  // DATA string or FLLR string
	  unsigned int data_size;					// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
  };
  #pragma pack(pop)

  HEADER header;
};


#endif
