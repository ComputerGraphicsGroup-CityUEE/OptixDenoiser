#ifndef RELITDIR_PSNR_H
#define RELITDIR_PSNR_H

void relitdir_psnr( const char *src_info_path, int n_basis, void (*func)(FLOAT3) );
void relitdir_cg_psnr( const char *src_info_path, int n_basis, void (*func)(FLOAT3) );
void relitdir_capture( const char *src_info_path, int n_basis, int idx, void (*func)(FLOAT3) );




void relitdir_cg_psnr_blk( 
  const char *src_info_path, 
  int n_basis,
  void (*func)(FLOAT3),
  int x, int y, int w, int h, int blk_idx
);
void relitdir_capture_blk( const char *src_info_path, int n_basis, int idx, void (*func)(FLOAT3), int w, int h, int blk_idx );

#endif
