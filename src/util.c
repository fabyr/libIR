#include "util.h"

inline complex complex_mul(complex a, complex b)
{
    complex result;
    result.X = (a.X * b.X - a.Y * b.Y);
    result.Y = (a.X * b.Y + a.Y * b.X);
    return result;
}

inline void complex_mul_i(complex* a, complex b)
{
    a->X = (a->X * b.X - a->Y * b.Y);
    a->Y = (a->X * b.Y + a->Y * b.X);
}

inline void complex_mul_real_i(complex* a, float b)
{
    a->X *= b;
    a->Y *= b;
}

inline complex complex_mul_real(complex a, float b)
{
    complex result;
    result.X = a.X * b;
    result.Y = a.Y * b;
    return result;
}

inline complex complex_add(complex a, complex b)
{
    complex result;
    result.X = a.X + b.X;
    result.Y = a.Y + b.Y;
    return result;
}

inline void complex_add_i(complex* a, complex b)
{
    a->X += b.X;
    a->Y += b.Y;
}

int32_t pos_modulo(int32_t value, uint32_t m) {
    int32_t mod = value % (int32_t)m;
    if (mod < 0) {
        mod += m;
    }
    return mod;
}