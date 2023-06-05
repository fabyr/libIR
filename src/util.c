#include "util.h"

inline void complex_mul_i(fftw_complex a, fftw_complex b)
{
    a[0] = (a[0] * b[0] - a[1] * b[1]);
    a[1] = (a[0] * b[1] + a[1] * b[0]);
}

inline void complex_mul_real_i(fftw_complex a, float b)
{
    a[0] *= b;
    a[1] *= b;
}

inline void complex_add_i(fftw_complex a, fftw_complex b)
{
    a[0] += b[0];
    a[1] += b[1];
}

int32_t pos_modulo(int32_t value, uint32_t m) {
    int32_t mod = value % (int32_t)m;
    if (mod < 0) {
        mod += m;
    }
    return mod;
}