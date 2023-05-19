#include <stdio.h>
#include "fft/fft.h"

int main(void)
{
    complex data[512];

    fft(FFT_FORWARD, 9, data);
    fft(FFT_BACKWARD, 9, data);

    printf("Test3!!\n");
    return 0;
}