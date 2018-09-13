#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>

double get_cur_time()
{
  struct timeval tv;
  double t;

  gettimeofday(&tv,NULL);
  t = tv.tv_sec + tv.tv_usec / 1e6;
  return t;
}

void cp_data(char *src, char *dst, size_t nbytes)
{
#ifdef USE_AVX
  intptr_t src_ptr = src;
  intptr_t dst_ptr = dst;
  int nb_m256 = nbytes / 32;
  __m256d *dst_m256 = NULL;
  assert (src_ptr % 8 == 0);
  assert (dst_ptr % 8 == 0);
  for (int i = 0; i < nb_m256; i++) {
    dst_m256 = (__m256d *)dst;
    *dst_m256 = _mm256_load_pd((double *)src);
    src += 32;
    dst += 32;
  }
#else
  printf("memcpy\n");
  memcpy(dst, src, nbytes);
#endif
  return;
}

int main()
{
  size_t seg_size = 4 * 1024 * 1024;
  int nb_segs = 20;
  double *buff_src = (double*)malloc(seg_size * nb_segs);
  double *buff_dst = (double*)malloc(seg_size * nb_segs);
  memset(buff_src, 0, seg_size * nb_segs);
  memset(buff_dst, 0, seg_size * nb_segs);
  
  char *src = (char*)buff_src;
  char *dst = (char*)buff_dst;
  
  for (int i = 0; i < nb_segs; i++) {
    cp_data(src, dst, seg_size);
    src += seg_size;
    dst += seg_size;
  }
  
  src = (char*)buff_src;
  dst = (char*)buff_dst;
  
  double time_start = get_cur_time();
  
  for (int i = 0; i < nb_segs; i++) {
    cp_data(src, dst, seg_size);
    src += seg_size;
    dst += seg_size;
  }
  
  double time_end = get_cur_time();
  double time_duration = time_end - time_start;
  
  printf("size %ld, BW %f\n", seg_size, (seg_size * nb_segs / 1024 / 1024) / time_duration);
  
  free(buff_src);
  free(buff_dst);
  return 0;
}