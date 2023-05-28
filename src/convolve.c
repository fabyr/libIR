#include "convolve.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void free_convolve_data(convolve_data* data)
{
    free(data->x_tdl);
    free(data->x_fdl);
    free(data->y_fft);
}

convolve_data create_convolve_data(int32_t block_size, size_t ir_samples)
{
    convolve_data result;
    result.block_size_log2 = (int32_t)truncf(log2f(block_size + 1));
    result.block_size = 1 << result.block_size_log2;
    result.fft_n_log2 = result.block_size_log2 + 1;
    result.fft_n = 1 << result.fft_n_log2;

    result.x_tdl = malloc(sizeof(complex) * result.fft_n);
    memset(result.x_tdl, 0, sizeof(complex) * result.fft_n);
    result.y_fft = malloc(sizeof(complex) * result.fft_n);
    memset(result.y_fft, 0, sizeof(complex) * result.fft_n);
    result.n_blocks_ir = ir_samples / result.block_size;
    size_t sz = result.fft_n * result.n_blocks_ir;
    result.x_fdl = malloc(sizeof(complex) * sz);
    memset(result.x_fdl, 0, sizeof(complex) * sz);
    /*for(size_t i = 0; i < sz; i++)
    {
        result.x_fdl[i].X = NAN;
        result.x_fdl[i].Y = NAN;
    }*/

    result.x_fdl_at = 0;

    return result;
}

void block_convolve(convolve_data* data, complex ir_fft[], complex sig[], complex out[])
{
    memcpy(data->x_tdl, data->x_tdl + data->block_size, data->block_size * sizeof(complex));
    memcpy(data->x_tdl + data->block_size, sig, data->block_size * sizeof(complex));

    data->x_fdl_at++;
    if(data->x_fdl_at >= data->n_blocks_ir)
        data->x_fdl_at = 0;

    complex* current_x_fdl = data->x_fdl + (data->n_blocks_ir - data->x_fdl_at - 1) * data->fft_n;

    fft_to_buffer(FFT_FORWARD, data->fft_n_log2, data->x_tdl, current_x_fdl);
    
    for(size_t block = 0; block < data->n_blocks_ir; block++)
    {
        complex* x_fdl_shifted = (data->x_fdl + pos_modulo(-data->x_fdl_at + block, data->n_blocks_ir) * data->fft_n);
        complex* ir_fft_shifted = ir_fft + block * data->fft_n;
        for(int32_t i = 0; i < data->fft_n; i++)
        {
            complex r = complex_mul(x_fdl_shifted[i], ir_fft_shifted[i]);
            complex_mul_real_i(&r, data->fft_n);
            complex_add_i(data->y_fft + i, r);
        }
    }
    
    fft(FFT_BACKWARD, data->fft_n_log2, data->y_fft);
    memcpy(out, data->y_fft + data->block_size, data->block_size * sizeof(complex));
    memset(data->y_fft, 0, sizeof(complex) * data->fft_n);
}

void ir_fft(convolve_data* data, complex ir[], complex ir_fft[])
{
    for(size_t i = 0; i < data->n_blocks_ir; i++)
    {
        size_t ir_fft_offset = i * data->fft_n;
        size_t ir_offset = i * data->block_size;
        memcpy(ir_fft + ir_fft_offset, ir + ir_offset, data->block_size * sizeof(complex));
        memset(ir_fft + ir_fft_offset + data->block_size, 0, data->block_size * sizeof(complex));
        fft(FFT_FORWARD, data->fft_n_log2, ir_fft + ir_fft_offset);
    }
}

void convolve_all(int32_t block_size, complex ir[], complex sig[], complex out[], size_t ir_n, size_t sig_n)
{
    convolve_data data = create_convolve_data(block_size, ir_n);
    //print_convolve_data(&data);
    size_t n_blocks_sig = sig_n / data.block_size;
    complex* ir_buf = malloc(sizeof(complex) * data.n_blocks_ir * data.fft_n);
    ir_fft(&data, ir, ir_buf);
    //for(size_t i = 0; i < data.n_blocks_ir / 2; i++) block_convolve_pre(&data, ir_buf);
    for(size_t i = 0; i < n_blocks_sig; i++)
    {
        //printf("BLOCK CONVOLVE #%lu\n", i);
        size_t shift = i * data.block_size;
        block_convolve(&data, ir_buf, sig + shift, out + shift);
    }
    free(ir_buf);
    free_convolve_data(&data);
}

void print_convolve_data(convolve_data* data)
{
    printf("Block-Size: %i\n", data->block_size);
    printf("Block-Size-M: %i\n", data->block_size_log2);

    printf("FFT-Size: %i\n", data->fft_n);
    printf("FFT-Size-M: %i\n", data->fft_n_log2);

    printf("N-Blocks-IR: %lu\n", data->n_blocks_ir);
}