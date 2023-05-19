#include "../util.h"
#include <stdint.h>

typedef enum {
    FFT_FORWARD = -1,
    FFT_BACKWARD = 1
} fft_direction;

void fft(fft_direction dir, uint8_t m, complex data[]);
void fft_to_buffer(fft_direction dir, uint8_t m, complex data[], complex output[]);