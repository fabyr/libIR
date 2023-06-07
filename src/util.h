#include "standard.h"
#include <stdint.h>

void complex_mul_i(fftw_complex a, fftw_complex b);

void complex_add_i(fftw_complex a, fftw_complex b);

void complex_mul_real_i(fftw_complex a, float b);

int32_t pos_modulo(int32_t value, uint32_t m);