#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
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
  intptr_t src_ptr = (intptr_t)src;
  intptr_t dst_ptr = (intptr_t)dst;
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
  memcpy(dst, src, nbytes);
#endif
  return;
}

int main(int argc, char **argv)
{
  size_t seg_size = 4 * 1024 * 1024;
  size_t seg_gap = 4 * 1024 * 1024;
  int nb_segs = 0;
  int nb_iter = 20;
  int use_cache = 0;
  int i;
  
  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-size")) {
      seg_size = atol(argv[++i]);
    }
    if (!strcmp(argv[i], "-iter")) {
      nb_iter = atoi(argv[++i]);
    }
    if (!strcmp(argv[i], "-cache")) {
      use_cache = atoi(argv[++i]);
    }
  }
  
  if (use_cache) {
    nb_segs = 1;
  } else {
    nb_segs = nb_iter;
  }
  
  printf("size %ld, iter %d, use_cache %d, nb_segs %d\n", seg_size, nb_iter, use_cache, nb_segs);
  
  char **buff_src_array = (char **)malloc(sizeof(char*) * nb_segs);
  char **buff_dst_array = (char **)malloc(sizeof(char*) * nb_segs);
  for (i = 0; i < nb_segs; i++) {
    buff_src_array[i]= aligned_alloc(sizeof(double), seg_size + seg_gap);
    buff_dst_array[i]= aligned_alloc(sizeof(double), seg_size + seg_gap);
    memset(buff_src_array[i], 0, seg_size + seg_gap);
    memset(buff_dst_array[i], 0, seg_size + seg_gap); 
  }
  
  char *src = NULL;
  char *dst = NULL;
  
  double time_start = get_cur_time();
  
  for (int i = 0; i < nb_iter; i++) {
    if (use_cache) {
      src = buff_src_array[0];
      dst = buff_dst_array[0];
    } else {
      src = buff_src_array[i];
      dst = buff_dst_array[i];
    }
    cp_data(src, dst, seg_size);
  }
  
  double time_end = get_cur_time();
  double time_duration = time_end - time_start;
  
  printf("size %ld, time %f seconds, BW %f\n", seg_size, time_duration, ((double)(seg_size*nb_iter) / 1024 / 1024) / time_duration);
  
  for (i = 0; i < nb_segs; i++) {
    free(buff_src_array[i]);
    free(buff_dst_array[i]);
  }
  free(buff_src_array);
  free(buff_dst_array);
  return 0;
}