#include "fft/fft.h"
#include <math.h>
#include <stddef.h>

typedef struct {
    int32_t block_size_log2;
    int32_t block_size;
    int32_t fft_n_log2;
    int32_t fft_n;
    size_t n_blocks_ir;
    complex* x_tdl;
    complex* x_fdl;
    complex* y_fft;
    //size_t x_tdl_at;
    size_t x_fdl_at;
} convolve_data;

convolve_data create_convolve_data(int32_t block_size, size_t ir_samples);
void free_convolve_data(convolve_data* data);

void block_convolve(convolve_data* data, complex ir_fft[], complex sig[], complex out[]);

void ir_fft(convolve_data* data, complex ir[], complex ir_fft[]);

void convolve_all(int32_t block_size, complex ir[], complex sig[], complex out[], size_t ir_n, size_t sig_n);

void print_convolve_data(convolve_data* data);