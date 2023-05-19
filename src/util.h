#include <stdint.h>

typedef struct {
    float X;
    float Y;
} complex;

complex complex_mul(complex a, complex b);
void complex_mul_i(complex* a, complex b);

complex complex_add(complex a, complex b);
void complex_add_i(complex* a, complex b);

complex complex_mul_real(complex a, float b);
void complex_mul_real_i(complex* a, float b);

int32_t pos_modulo(int32_t value, uint32_t m);