#include "util.h"

inline void complex_mul_i(IR_COMPLEX_T a, IR_COMPLEX_T b)
{
    IR_FLOAT_T areTmp = a[0];
    a[0] = (a[0] * b[0] - a[1] * b[1]);
    a[1] = (areTmp * b[1] + a[1] * b[0]);
}

inline void complex_mul_real_i(IR_COMPLEX_T a, IR_FLOAT_T b)
{
    a[0] *= b;
    a[1] *= b;
}

inline void complex_add_i(IR_COMPLEX_T a, IR_COMPLEX_T b)
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