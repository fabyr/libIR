#include "fft/fft.h"
#include <math.h>
#include <stddef.h>

#define ALLOCATE_FFT_BUFFER   0b00000001

typedef struct {
    int32_t* block_sizes;
    int32_t entries;
} convolve_schedule;

typedef struct {
    int32_t* block_size_array;
    int32_t* block_size_sum_array;
    int32_t* block_size_log2_array;
    int32_t* fft_n_array;
    int32_t* fft_n_sum_array;
    int32_t* fft_n_log2_array;
    int32_t* layer_array;
    int32_t* layer_size_array;
    int32_t* layer_sum_array;
    int32_t* layer_nblocks_array;
    int32_t entries;
} convolve_fft_schedule;

typedef struct {
    convolve_fft_schedule schedule;
    int32_t in_block_size;
    int32_t in_block_size_log2;
    int32_t in_fft_size;
    int32_t in_fft_size_log2;
    int32_t max_block_size;
    int32_t max_block_size_log2;
    int32_t max_fft_n;
    int32_t max_fft_n_log2;
    int32_t n_blocks_ir;
    int32_t n_blocks_in_ir;
    int32_t blockbuffer_size;
    int32_t fftbuffer_size;
    int32_t x_fdl_layers;
    complex* x_tdl;
    complex* x_fdl;
    complex* y_fft;
    complex* y_buffer;
    complex* ir_buffer_fft;
    int32_t flags;
    //int32_t x_tdl_at;
    int32_t x_fdl_at;
    int32_t y_fdl_at;
} convolve_data;

convolve_data create_convolve_data(convolve_schedule schedule, int32_t ir_samples, int32_t flags);
void free_convolve_data(convolve_data* data);

void block_convolve(convolve_data* data, complex ir_fft[], complex sig[], complex out[]);
void block_convolve_fft(convolve_data* data, complex ir[], complex sig[], complex out[]);

void ir_fft(convolve_data* data, complex ir[], complex ir_fft[]);

void convolve_all(convolve_schedule schedule, complex ir[], complex sig[], complex out[], int32_t ir_n, int32_t sig_n);
void convolve_all_fft(convolve_schedule schedule, complex ir[], complex sig[], complex out[], int32_t ir_n, int32_t sig_n);