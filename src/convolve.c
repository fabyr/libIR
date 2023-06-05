#include "convolve.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

void free_convolve_data(convolve_data* data)
{
    free(data->schedule.block_size_array);
    free(data->schedule.block_size_log2_array);
    free(data->schedule.block_size_sum_array);
    free(data->schedule.fft_n_array);
    free(data->schedule.fft_n_log2_array);
    free(data->schedule.fft_n_sum_array);
    free(data->schedule.layer_array);
    free(data->schedule.layer_nblocks_array);
    free(data->schedule.layer_sum_array);
    free(data->schedule.layer_size_array);
    free(data->x_tdl);
    free(data->x_fdl);
    free(data->y_buffer);
    free(data->y_fft);
    if((data->flags & ALLOCATE_FFT_BUFFER) > 0)
    {
        free(data->ir_buffer_fft);
    }
}

convolve_data create_convolve_data(convolve_schedule schedule, int32_t ir_samples, int32_t flags)
{
    convolve_fft_schedule fft_schedule;
    int32_t samples_left = ir_samples;
    fft_schedule.entries = 0;
    int32_t last_size = -1;
    for(int32_t i = 0; i < schedule.entries; i++)
    {
        if(samples_left - schedule.block_sizes[i] < 0)
            break;
        last_size = schedule.block_sizes[i];
        fft_schedule.entries++;
        samples_left -= last_size;
    }
    if(last_size == -1)
    {
        exit(0xf777);
    }
    for(;;)
    {
        if(samples_left - last_size < 0)
            break;
        fft_schedule.entries++;
        samples_left -= last_size;
    }
    fft_schedule.block_size_array = (int32_t*)malloc(sizeof(int32_t) * fft_schedule.entries);
    fft_schedule.block_size_log2_array = (int32_t*)malloc(sizeof(int32_t) * fft_schedule.entries);
    fft_schedule.block_size_sum_array = (int32_t*)malloc(sizeof(int32_t) * fft_schedule.entries);
    fft_schedule.fft_n_array = (int32_t*)malloc(sizeof(int32_t) * fft_schedule.entries);
    fft_schedule.fft_n_log2_array = (int32_t*)malloc(sizeof(int32_t) * fft_schedule.entries);
    fft_schedule.fft_n_sum_array = (int32_t*)malloc(sizeof(int32_t) * fft_schedule.entries);

    fft_schedule.layer_array = (int32_t*)malloc(sizeof(int32_t) * fft_schedule.entries);
    // note: unoptimal array size
    fft_schedule.layer_size_array = (int32_t*)malloc(sizeof(int32_t) * fft_schedule.entries);
    fft_schedule.layer_sum_array = (int32_t*)malloc(sizeof(int32_t) * fft_schedule.entries);
    fft_schedule.layer_nblocks_array = (int32_t*)malloc(sizeof(int32_t) * fft_schedule.entries);
    int32_t bsz = -1, max_bsz = -1, max_bsz_log2 = -1, max_fftn = -1, max_fftn_log2 = -1;
    int32_t blockbuffersize = 0, fftbuffersize = 0;
    for(int32_t i = 0; i < fft_schedule.entries; i++)
    {
        if(i < schedule.entries)
            bsz = schedule.block_sizes[i];
        int32_t b_log2 = (int32_t)truncf(log2f(bsz + 1));
        int32_t b = 1 << b_log2;
        int32_t f = b << 1;
        int32_t f_log2 = b_log2 + 1;
        if(b > max_bsz)
            max_bsz = b;
        if(b_log2 > max_bsz_log2)
            max_bsz_log2 = b_log2;
        if(f > max_fftn)
            max_fftn = f;
        if(f_log2 > max_fftn_log2)
            max_fftn_log2 = f_log2;
        
        fft_schedule.block_size_array[i] = b;
        fft_schedule.block_size_log2_array[i] = b_log2;
        fft_schedule.block_size_sum_array[i] = blockbuffersize;
        fft_schedule.fft_n_array[i] = f;
        fft_schedule.fft_n_log2_array[i] = f_log2;
        fft_schedule.fft_n_sum_array[i] = fftbuffersize;

        blockbuffersize += b;
        fftbuffersize += f;
    }
    
    int32_t fdllayers = 0, fdlpos = 0;
    int8_t fdllayersvisited[fft_schedule.entries];
    memset(fdllayersvisited, 0, fft_schedule.entries * sizeof(int8_t));
    for(int32_t i = 0; i < fft_schedule.entries; i++)
    {
        if(fdllayersvisited[i] == 0)
        {
            fft_schedule.layer_sum_array[fdllayers] = fdlpos;
            fdlpos += (fft_schedule.layer_nblocks_array[fdllayers] = ir_samples / (1 << (fft_schedule.layer_size_array[fdllayers] = fft_schedule.block_size_log2_array[i]))) * fft_schedule.fft_n_array[i];
            fdllayers++;
        }
        else
            continue;
        for(int32_t j = 0; j < fft_schedule.entries; j++)
        {   
            if(fdllayersvisited[j] == 1)
                continue;
            if(fft_schedule.fft_n_log2_array[i] == fft_schedule.fft_n_log2_array[j])
            {
                fft_schedule.layer_array[i] = fdllayers - 1;
                fft_schedule.layer_array[j] = fdllayers - 1;
                fdllayersvisited[i] = 1;
                fdllayersvisited[j] = 1;
            }
        }
    }
    int32_t fdltotal = 0, fdlsingletotal = 0;
    for(int32_t i = 0; i < fdllayers; i++)
    {
        fdlsingletotal += (1 << fft_schedule.layer_size_array[i]);
        fdltotal += (1 << fft_schedule.layer_size_array[i]) * fft_schedule.layer_nblocks_array[i];
    }
    
    convolve_data result;
    result.schedule = fft_schedule;
    result.n_blocks_ir = fft_schedule.entries;
    result.in_block_size = fft_schedule.block_size_array[0];
    result.in_block_size_log2 = fft_schedule.block_size_log2_array[0];
    result.in_fft_size = fft_schedule.fft_n_array[0];
    result.in_fft_size_log2 = fft_schedule.fft_n_log2_array[0];
    result.max_block_size = max_bsz;
    result.max_block_size_log2 = max_bsz_log2;
    result.max_fft_n = max_fftn;
    result.max_fft_n_log2 = max_fftn_log2;
    result.blockbuffer_size = blockbuffersize;
    result.fftbuffer_size = fftbuffersize;
    result.n_blocks_in_ir = blockbuffersize / result.in_block_size;
    result.x_fdl_layers = fdllayers;
    
    result.x_tdl = malloc(sizeof(complex) * result.in_block_size * result.n_blocks_in_ir);
    memset(result.x_tdl, 0, sizeof(complex) * result.in_block_size * result.n_blocks_in_ir);
    result.y_fft = malloc(sizeof(complex) * result.in_fft_size);
    memset(result.y_fft, 0, sizeof(complex) * result.in_fft_size);
    result.x_fdl = malloc(sizeof(complex) * fdltotal * 2);
    memset(result.x_fdl, 0, sizeof(complex) * fdltotal * 2);
    result.y_buffer = malloc(sizeof(complex) * fdlsingletotal * 2);
    memset(result.y_buffer, 0, sizeof(complex) * fdlsingletotal * 2);
    
    result.x_fdl_at = 0;
    result.y_fdl_at = 0; // only used in time-variant version
    result.flags = flags;
    
    if((result.flags & ALLOCATE_FFT_BUFFER) > 0)
    {
        result.ir_buffer_fft = (complex*)malloc(sizeof(complex) * result.fftbuffer_size);
    }
    
    return result;
}

void block_convolve(convolve_data* data, complex ir_fft[], complex sig[], complex out[])
{
    complex* current_x_tdl_0 = data->x_tdl + pos_modulo(-data->x_fdl_at + 1, data->n_blocks_in_ir) * data->in_block_size; // + offset of 0
    complex* current_x_tdl_1 = data->x_tdl + pos_modulo(-data->x_fdl_at, data->n_blocks_in_ir) * data->in_block_size; // + offset of 0
    
    memcpy(current_x_tdl_1, sig, data->in_block_size * sizeof(complex));

    int32_t o3 = 0;
    for(int32_t i = 0; i < data->x_fdl_layers; i++)
    {
        int32_t layer_denom = (1 << data->schedule.layer_size_array[i]) / data->in_block_size;
        int32_t layer_fft_n_log2 = data->schedule.layer_size_array[i] + 1;
        int32_t layer_fft_n = 1 << layer_fft_n_log2;
        
        if(data->x_fdl_at % layer_denom == 0)
        {
            {
                int32_t sh, shi;
                if(i == 0)
                {
                    sh = 0;
                    shi = 0;
                } 
                else if(i == 1)
                {
                    sh = 1;
                    shi = -1;
                }
                else 
                {
                    sh = i + 1;
                    shi = 1;
                }

                int32_t o = pos_modulo((-data->x_fdl_at) / layer_denom + sh, data->schedule.layer_nblocks_array[i]);
                complex* layer_x_fdl = data->x_fdl + o * layer_fft_n + data->schedule.layer_sum_array[i];

                for(int32_t i = 0; i < layer_denom * 2; i++)
                {
                    complex* x_tdl_i = data->x_tdl + pos_modulo(-data->x_fdl_at + (layer_denom * 2 - i) + shi, data->n_blocks_in_ir)
                                         * data->in_block_size; // + offset of 0
                    
                    memcpy(layer_x_fdl, x_tdl_i, sizeof(complex) * data->in_block_size);
                    layer_x_fdl += data->in_block_size;
                }
                fft(FFT_FORWARD, layer_fft_n_log2, layer_x_fdl - layer_fft_n);
            }
            
            complex* y_buffer_at = data->y_buffer + o3;
            memset(y_buffer_at, 0, sizeof(complex) * layer_fft_n);

            int32_t baseblockoffset = 0;
            for(int32_t block = 0; block < data->schedule.entries; block++)
            {
                int32_t layer_at = data->schedule.layer_array[block];
                if(layer_at == i)
                {
                    //if(i != 2) continue;
                    complex* ptr = (data->x_fdl + pos_modulo((-data->x_fdl_at + baseblockoffset) / layer_denom, data->schedule.layer_nblocks_array[i]) * layer_fft_n + data->schedule.layer_sum_array[layer_at]);
                    complex* ir_fft_shifted = ir_fft + data->schedule.fft_n_sum_array[block];
            
                    for(int32_t j = 0; j < layer_fft_n; j++)
                    {
                        complex r = complex_mul(ptr[j], ir_fft_shifted[j]);
                        complex_mul_real_i(&r, layer_fft_n);
                        complex_add_i(y_buffer_at + j, r);
                    }
                }
                
                baseblockoffset += layer_denom;
            }
            fft(FFT_BACKWARD, layer_fft_n_log2, y_buffer_at);
        }
        {
            complex* y_buffer_at = data->y_buffer + o3 + layer_fft_n / 2
                                    + (data->x_fdl_at % layer_denom) * data->in_block_size;
            
            for(int32_t i = 0; i < data->in_block_size; i++)
            {
                complex_add_i(out + i, y_buffer_at[i]);
            }
        }
        o3 += (1 << data->schedule.layer_size_array[i]) * 2;
    }
    
    data->x_fdl_at++;
    if(data->x_fdl_at >= data->n_blocks_in_ir)
        data->x_fdl_at = 0;
}

void block_convolve_fft(convolve_data* data, complex ir[], complex sig[], complex out[])
{
    complex* current_x_tdl_0 = data->x_tdl + pos_modulo(-data->x_fdl_at + 1, data->n_blocks_in_ir) * data->in_block_size; // + offset of 0
    complex* current_x_tdl_1 = data->x_tdl + pos_modulo(-data->x_fdl_at, data->n_blocks_in_ir) * data->in_block_size; // + offset of 0
    
    memcpy(current_x_tdl_1, sig, data->in_block_size * sizeof(complex));

    int32_t o3 = 0;
    for(int32_t i = 0; i < data->x_fdl_layers; i++)
    {
        int32_t layer_denom = (1 << data->schedule.layer_size_array[i]) / data->in_block_size;
        int32_t layer_fft_n_log2 = data->schedule.layer_size_array[i] + 1;
        int32_t layer_fft_n = 1 << layer_fft_n_log2;
        
        if(data->x_fdl_at % layer_denom == 0)
        {
            {
                int32_t sh, shi;
                if(i == 0)
                {
                    sh = 0;
                    shi = 0;
                } 
                else if(i == 1)
                {
                    sh = 1;
                    shi = -1;
                }
                else 
                {
                    sh = i + 1;
                    shi = 1;
                }

                int32_t o = pos_modulo((-data->x_fdl_at) / layer_denom + sh, data->schedule.layer_nblocks_array[i]);
                complex* layer_x_fdl = data->x_fdl + o * layer_fft_n + data->schedule.layer_sum_array[i];

                for(int32_t i = 0; i < layer_denom * 2; i++)
                {
                    complex* x_tdl_i = data->x_tdl + pos_modulo(-data->x_fdl_at + (layer_denom * 2 - i) + shi, data->n_blocks_in_ir)
                                         * data->in_block_size; // + offset of 0
                    
                    memcpy(layer_x_fdl, x_tdl_i, sizeof(complex) * data->in_block_size);
                    layer_x_fdl += data->in_block_size;
                }
                fft(FFT_FORWARD, layer_fft_n_log2, layer_x_fdl - layer_fft_n);
            }
            
            complex* y_buffer_at = data->y_buffer + o3;
            memset(y_buffer_at, 0, sizeof(complex) * layer_fft_n);

            int32_t baseblockoffset = 0;
            for(int32_t block = 0; block < data->schedule.entries; block++)
            {
                int32_t layer_at = data->schedule.layer_array[block];
                if(layer_at == i)
                {
                    //if(i != 2) continue;
                    complex* ptr = (data->x_fdl + pos_modulo((-data->x_fdl_at + baseblockoffset) / layer_denom, data->schedule.layer_nblocks_array[i]) * layer_fft_n + data->schedule.layer_sum_array[layer_at]);
                    
                    complex* ir_fft_shifted = data->ir_buffer_fft + data->schedule.fft_n_sum_array[block];

                    // ADDED IN FFT-ON-THE-FLY VARIANT
                    int32_t bsz = data->schedule.block_size_array[block];
                    memcpy(ir_fft_shifted, ir + data->schedule.block_size_sum_array[block], bsz * sizeof(complex));
                    memset(ir_fft_shifted + bsz, 0, bsz * sizeof(complex));
                    fft(FFT_FORWARD, layer_fft_n_log2, ir_fft_shifted);
                    
                    for(int32_t j = 0; j < layer_fft_n; j++)
                    {
                        complex r = complex_mul(ptr[j], ir_fft_shifted[j]);
                        complex_mul_real_i(&r, layer_fft_n);
                        complex_add_i(y_buffer_at + j, r);
                    }
                }
                
                baseblockoffset += layer_denom;
            }
            fft(FFT_BACKWARD, layer_fft_n_log2, y_buffer_at);
        }
        {
            complex* y_buffer_at = data->y_buffer + o3 + layer_fft_n / 2
                                    + (data->x_fdl_at % layer_denom) * data->in_block_size;
            
            for(int32_t i = 0; i < data->in_block_size; i++)
            {
                complex_add_i(out + i, y_buffer_at[i]);
            }
        }
        o3 += (1 << data->schedule.layer_size_array[i]) * 2;
    }
    
    data->x_fdl_at++;
    if(data->x_fdl_at >= data->n_blocks_in_ir)
        data->x_fdl_at = 0;
}

void ir_fft(convolve_data* data, complex ir[], complex ir_fft[])
{
    int32_t ir_fft_offset = 0, ir_offset = 0;
    for(int32_t i = 0; i < data->n_blocks_ir; i++)
    {
        int32_t bsz = data->schedule.block_size_array[i], fsz = data->schedule.fft_n_array[i];
        memcpy(ir_fft + ir_fft_offset, ir + ir_offset, bsz * sizeof(complex));
        memset(ir_fft + ir_fft_offset + bsz, 0, bsz * sizeof(complex));
        fft(FFT_FORWARD, data->schedule.fft_n_log2_array[i], ir_fft + ir_fft_offset);
        
        ir_fft_offset += fsz;
        ir_offset += bsz;
    }
}

void convolve_all(convolve_schedule schedule, complex ir[], complex sig[], complex out[], int32_t ir_n, int32_t sig_n)
{
    convolve_data data = create_convolve_data(schedule, ir_n, 0);

    int32_t n_blocks_sig = sig_n / data.in_block_size;
    complex* ir_buf = malloc(sizeof(complex) * data.fftbuffer_size);
    ir_fft(&data, ir, ir_buf);
    for(int32_t i = 0; i < n_blocks_sig; i++)
    {
        int32_t shift = i * data.in_block_size;
        block_convolve(&data, ir_buf, sig + shift, out + shift);
    }
    free(ir_buf);
    free_convolve_data(&data);
}

void convolve_all_fft(convolve_schedule schedule, complex ir[], complex sig[], complex out[], int32_t ir_n, int32_t sig_n)
{
    convolve_data data = create_convolve_data(schedule, ir_n, ALLOCATE_FFT_BUFFER);

    int32_t n_blocks_sig = sig_n / data.in_block_size;
    for(int32_t i = 0; i < n_blocks_sig; i++)
    {
        int32_t shift = i * data.in_block_size;
        block_convolve_fft(&data, ir, sig + shift, out + shift);
    }
    free_convolve_data(&data);
}