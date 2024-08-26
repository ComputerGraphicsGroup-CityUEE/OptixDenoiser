#ifndef FILETER_CUBEMAP_H
#define FILETER_CUBEMAP_H

void gpugen_filter_cubemap( const GPfm &cmsrc, float filter_size, GPfm &res );
void gpugen_filter_cubemap( const char *tcm_path, float filter_size, const char *spath );

void gpugen_cubemapmip( const char *tcm_path, int cmside, const char *spath );
void gpugen_cubemapmip( const GPfm &cmsrc, GPfm &mip );
void cpugen_cubemapmip( const char *tcm_path, int cmside, const char *spath );


#endif
