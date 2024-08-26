#include <stdio.h>

#include <cuda_runtime.h>
#include <optix.h>
#include <optix_stubs.h>
#include <optix_function_table_definition.h>

#include "g_optix_denoiser.h"

#define XOPTIX_CHECK( call )                                          \
do                                                                    \
{                                                                     \
  OptixResult res = call;                                             \
  if( res != OPTIX_SUCCESS )                                          \
  {                                                                   \
    printf( "CUDA failed:\n%s\nline %i\n%s\n"                         \
            , __FILE__, __LINE__, #call );                            \
    exit(0);                                                          \
  }                                                                   \
} while( 0 )

#define XCUDA_CHECK( call )                                           \
do                                                                    \
{                                                                     \
  cudaError_t error = call;                                           \
  if( error != cudaSuccess )                                          \
  {                                                                   \
    printf( "CUDA failed:\n%s\nline %i\n%s\n%s\n"                     \
            , __FILE__, __LINE__, cudaGetErrorString(error), #call ); \
    exit(0);                                                          \
  }                                                                   \
} while( 0 )

GOptixDenoiser::GOptixDenoiser()
{
  memset(this,0,sizeof(this));
}
GOptixDenoiser::~GOptixDenoiser()
{
  unload();
}

//void* GOptixDenoiser::map()
//{
//  void *ptr;
//  size_t buffer_size;
//  XCUDA_CHECK( cudaGraphicsMapResources( 1, &frame_gfx, denoiser_stream ));
//  XCUDA_CHECK( cudaGraphicsResourceGetMappedPointer( &ptr, &buffer_size, frame_gfx ));
//  return ptr;
//}
//void GOptixDenoiser::unmap()
//{
//  XCUDA_CHECK( cudaGraphicsUnmapResources( 1, &frame_gfx, denoiser_stream ));
//}

void GOptixDenoiser::unload()
{
  if(denoiser)
    optixDenoiserDestroy(denoiser);
  if(denoiserState)
    XCUDA_CHECK(cudaFree((void*)denoiserState));
  if(scratch)
    XCUDA_CHECK(cudaFree((void*)scratch));
  if(hdrIntensity)
    XCUDA_CHECK(cudaFree((void*)hdrIntensity));
  //if(frame_pbo)
  //  glDeleteBuffers(1, &frame_pbo);
  memset(this,0,sizeof(this));
}

void GOptixDenoiser::load( CUstream stream, OptixDeviceContext context, int width, int height )
{
  if(denoiser)
    unload();

  denoiser_stream = stream;

  options.inputKind = OPTIX_DENOISER_INPUT_RGB;
  options.pixelFormat = OPTIX_PIXEL_FORMAT_FLOAT4;
  XOPTIX_CHECK( optixDenoiserCreate(context, &options, &denoiser ));
  XOPTIX_CHECK( optixDenoiserSetModel(denoiser, OPTIX_DENOISER_MODEL_KIND_HDR, 0, 0));
    
  resize(width,height);
}

void GOptixDenoiser::resize( int width, int height )
{
  //{
  //  if(frame_pbo)
  //    glDeleteBuffers(1, &frame_pbo);
  //  glGenBuffers(1, &frame_pbo);
  //  glBindBuffer(GL_ARRAY_BUFFER, frame_pbo);
  //  glBufferData(GL_ARRAY_BUFFER, width*height*sizeof(float3), 0, GL_STREAM_DRAW);
  //  glBindBuffer(GL_ARRAY_BUFFER, 0);
  //  XCUDA_CHECK(cudaGraphicsGLRegisterBuffer( &frame_gfx, frame_pbo, cudaGraphicsMapFlagsWriteDiscard ) );
  //}

  {
    if( denoiserState )
      XCUDA_CHECK(cudaFree((void*)denoiserState));
    if( scratch )
      XCUDA_CHECK(cudaFree((void*)scratch));
    if( hdrIntensity )
      XCUDA_CHECK(cudaFree((void*)hdrIntensity));
    int i;
    for( i=0; i<sizeof(buf)/sizeof(buf[0]); i++ )
    {
      buf[i].format = OPTIX_PIXEL_FORMAT_FLOAT4;
      buf[i].width = width;
      buf[i].height= height;
      buf[i].pixelStrideInBytes = sizeof(float4);
      buf[i].rowStrideInBytes   = width*sizeof(float4);
    }
    XOPTIX_CHECK( optixDenoiserComputeMemoryResources( denoiser, width, height, &returnSizes ));
    XCUDA_CHECK( cudaMalloc( (void**)&denoiserState, returnSizes.stateSizeInBytes ) );
    XCUDA_CHECK( cudaMalloc( (void**)&scratch      , returnSizes.recommendedScratchSizeInBytes ));
    XOPTIX_CHECK( optixDenoiserSetup( 
      denoiser, denoiser_stream, 
      width, height, 
      denoiserState, returnSizes.stateSizeInBytes, 
      scratch, returnSizes.recommendedScratchSizeInBytes
    ));
    XCUDA_CHECK( cudaMalloc( (void**)&hdrIntensity, width*height*sizeof(float) ) );
    params.hdrIntensity = hdrIntensity;
    params.blendFactor = 0.0;
    params.denoiseAlpha = 1;
  }
}

//#include "g_pfm.h"
void GOptixDenoiser::denoise( CUdeviceptr src, CUdeviceptr des )
{
  buf[0].data = src;
  buf[1].data = des;
  XOPTIX_CHECK( optixDenoiserComputeIntensity( denoiser, denoiser_stream, &buf[0], params.hdrIntensity, scratch, returnSizes.stateSizeInBytes ));
  XOPTIX_CHECK( optixDenoiserInvoke(denoiser, denoiser_stream, &params, denoiserState, returnSizes.stateSizeInBytes, &buf[0], 1, 0, 0, &buf[1], scratch, returnSizes.recommendedScratchSizeInBytes));

  //XCUDA_CHECK(cudaMemcpy((void*)buf[1].data, (void*)buf[0].data, buf[1].width*buf[1].height*sizeof(FLOAT3), cudaMemcpyDeviceToDevice));

  //GPfm a;
  //a.load( buf[0].width, buf[0].height );
  //XCUDA_CHECK(cudaMemcpy(a.fm, (void*)buf[1].data, buf[1].width*buf[1].height*sizeof(FLOAT3), cudaMemcpyDeviceToHost));
  //a.save( "a.pfm" );
  //exit(0);
}

