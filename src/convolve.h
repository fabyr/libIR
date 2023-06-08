#include "standard.h"
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include "util.h"

#define ALLOCATE_FFT_BUFFER 0b00000001
#define FFTW_FLAG_MEASURE   0b00000010

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
    ir_fftw_plan* forward_plans;
    ir_fftw_plan* backward_plans;
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
    IR_COMPLEX_T* x_tdl;
    IR_COMPLEX_T* x_fdl;
    IR_COMPLEX_T* fft_buffer_in;
    IR_COMPLEX_T* fft_buffer_out;
    IR_COMPLEX_T* y_buffer;
    IR_COMPLEX_T* ir_buffer_fft;
    int32_t flags;
    //int32_t x_tdl_at;
    int32_t x_fdl_at;
    int32_t y_fdl_at;
} convolve_data;

convolve_data create_convolve_data(convolve_schedule schedule, int32_t ir_samples, int32_t flags);
void free_convolve_data(convolve_data* data);

void block_convolve(convolve_data* data, IR_COMPLEX_T ir_fft[], IR_COMPLEX_T sig[], IR_COMPLEX_T out[]);
void block_convolve_fft(convolve_data* data, IR_COMPLEX_T ir[], IR_COMPLEX_T sig[], IR_COMPLEX_T out[]);

void ir_fft(convolve_data* data, IR_COMPLEX_T ir[], IR_COMPLEX_T ir_fft[]);

void convolve_all(convolve_schedule schedule, IR_COMPLEX_T ir[], IR_COMPLEX_T sig[], IR_COMPLEX_T out[], int32_t ir_n, int32_t sig_n, int32_t flags);
void convolve_all_fft(convolve_schedule schedule, IR_COMPLEX_T ir[], IR_COMPLEX_T sig[], IR_COMPLEX_T out[], int32_t ir_n, int32_t sig_n, int32_t flags);