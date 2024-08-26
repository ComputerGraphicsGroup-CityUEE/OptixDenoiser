#ifndef G_FILE_H
#define G_FILE_H

class GFile
{
  public:
    GFile();
    ~GFile();
    void load( const char *spath, int mem_size =20*1024*1024, int mem_size_ahead =1*1024*1024 );
    char* getline();
    void close();

    FILE *f0;
    int mem_size;
    int mem_size_ahead;
    char spath[256];
    int flen, cpos, fpos;
    char *_dat, *dat;
};

#endif
