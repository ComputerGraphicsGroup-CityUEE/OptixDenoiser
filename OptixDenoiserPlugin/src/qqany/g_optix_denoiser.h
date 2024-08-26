#ifndef G_OPTIX_DENOISER
#define G_OPTIX_DENOISER

#include <cuda_runtime.h>
#include <optix.h>

class GOptixDenoiser
{
  public:
    //GLuint frame_pbo;

    CUstream denoiser_stream;
    CUdeviceptr denoiserState, scratch, hdrIntensity;
    //cudaGraphicsResource *frame_gfx;
    
    OptixDenoiser denoiser;
    OptixDenoiserOptions options;
    OptixDenoiserSizes returnSizes;
    OptixDenoiserParams params;
    OptixImage2D buf[2];

    GOptixDenoiser();
    ~GOptixDenoiser();
    void load(CUstream stream, OptixDeviceContext context, int width, int height);
    void unload();
    //void* map();
    //void unmap();
    void resize(int width, int height);
    void denoise( CUdeviceptr src, CUdeviceptr des );
};

#endif