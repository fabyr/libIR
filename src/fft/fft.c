#include "fft.h"
#include "math.h"
#include <math.h>
#include <string.h>

void fft_to_buffer(fft_direction dir, uint8_t m, complex data[], complex output[])
{
    memcpy(output, data, (1 << m) * sizeof(complex));
    fft(dir, m, output);
}

void fft(fft_direction dir, uint8_t m, complex data[])
{
    int32_t n, i, i1, j, k, i2, l, l1, l2;
    float c1, c2, tx, ty, t1, t2, u1, u2, z, inv_n;
    
    // Calculate the number of points
    n = 1 << m;
    inv_n = 1.0f / n;

    // Do the bit reversal
    i2 = n >> 1;
    j = 0;
    for (i = 0; i < n - 1; i++)
    {
        if (i < j)
        {
            tx = data[i].X;
            ty = data[i].Y;
            data[i].X = data[j].X;
            data[i].Y = data[j].Y;
            data[j].X = tx;
            data[j].Y = ty;
        }
        k = i2;

        while (k <= j)
        {
            j -= k;
            k >>= 1;
        }
        j += k;
    }

    // Compute the FFT 
    c1 = -1.0f;
    c2 = 0.0f;
    l2 = 1;
    for (l = 0; l < m; l++)
    {
        l1 = l2;
        l2 <<= 1;
        u1 = 1.0f;
        u2 = 0.0f;
        for (j = 0; j < l1; j++)
        {
            for (i = j; i < n; i += l2)
            {
                i1 = i + l1;
                t1 = u1 * data[i1].X - u2 * data[i1].Y;
                t2 = u1 * data[i1].Y + u2 * data[i1].X;
                data[i1].X = data[i].X - t1;
                data[i1].Y = data[i].Y - t2;
                data[i].X += t1;
                data[i].Y += t2;
            }
            z = u1 * c1 - u2 * c2;
            u2 = u1 * c2 + u2 * c1;
            u1 = z;
        }
        c2 = sqrtf((1.0f - c1) * 0.5f) * dir;
        c1 = sqrtf((1.0f + c1) * 0.5f);
    }

    // Scaling for forward transform 
    if (dir == FFT_FORWARD)
    {
        for (i = 0; i < n; i++)
        {
            data[i].X *= inv_n;
            data[i].Y *= inv_n;
        }
    }
}