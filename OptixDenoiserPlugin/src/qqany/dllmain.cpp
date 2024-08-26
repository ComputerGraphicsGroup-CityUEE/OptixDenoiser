#include <optix.h>
#include <optix_stubs.h>

#include "g_optix_denoiser.h"

CUcontext cu_ctx = 0;  // zero means take the current context
OptixDeviceContext context;
GOptixDenoiser *qq = NULL;
void *cuda_src, *cuda_des;
int width, height;

#include "g_pfm.h"

extern "C" __declspec(dllexport) void __stdcall qq_init(int width0, int height0, int size)
{
	width = width0;
	height = height0;
	cudaFree(0);
	optixInit();
	{
		OptixDeviceContextOptions options = {};
		options.logCallbackFunction = NULL;
		options.logCallbackLevel = 0;
		optixDeviceContextCreate(cu_ctx, &options, &context);
	}
	qq = new GOptixDenoiser();
	qq->load(0, context, width, height);
	cudaMalloc(&cuda_src, width * height * sizeof(float4));
	cudaMalloc(&cuda_des, width * height * sizeof(float4));
}


extern "C" __declspec(dllexport) void __stdcall qq_denoise( const void *src, void *des)
{
	//GPfm aaaa;
	//aaaa.load("4spp_resize.png");
	//aaaa.resize(width, height);
	//GPf4 aaa;
	//aaa.load(width, height, aaaa.fm);
	//printf("sizeof(float4) %d\n", sizeof(float4));
	//cudaMemcpy(cuda_src, src, width * height + 1, cudaMemcpyHostToDevice);

	cudaMemcpy( cuda_src, src, width * height * sizeof(float4), cudaMemcpyHostToDevice );
	qq->denoise((CUdeviceptr)cuda_src, (CUdeviceptr)cuda_des);
	cudaMemcpy(      des, cuda_des, width * height * sizeof(float4), cudaMemcpyDeviceToHost );

	GPf4 img;
	img.load(width, height);
	cudaMemcpy(img.fm, src, width * height * sizeof(float4), cudaMemcpyDeviceToHost);
	img.save("img.png");

	//int i;
	//for (i = 0; i < width * height; i++)
	//	((FLOAT4*)des)[i] = FLOAT4(1, 0, 0, 1);

}

extern "C" __declspec(dllexport) void __stdcall qq_resize(int width0, int height0)
{
	width = width0;
	height = height0;
	cudaFree(cuda_src);
	cudaFree(cuda_des);
	cudaMalloc(&cuda_src, width * height * sizeof(float4));
	cudaMalloc(&cuda_des, width * height * sizeof(float4));
	qq->resize(width, height);
}
