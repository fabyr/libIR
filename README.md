# libIR
Small C-Library for performing convolution reverb

## Building
System dependencies:
- fftw3
  - Installing on debian/ubuntu based distros: `sudo apt install fftw3-dev`

```
mkdir build
cd build

export SINGLE_PRECISION=1
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. # export compile commands for clangd (development)
cmake --build . --config Release
```

## Usage
libIR performs a partitioned convolution.
Create a `convolve_schedule` as follows:
```C
// ... = { block_sizes, count_of_block_sizes }
convolve_schedule sched = { (int32_t[]){ 8192 }, 1 }; // Uniform block sizes

convolve_schedule sched2 = { (int32_t[]){ 1024, 1024, 2048, 4096 }, 4 }; // Non-Uniform (Fully Partitioned) block sizes
```
**NOTE:** When creating Non-Uniform convolve schedules, they always have to be of the form
`{ x, x, 2 * x, 4 * x, 8 * x, ..., 2^(n - 2) * x }` where `n` is the total number of entries in the list and `x` must always be a power of 2. 
If this is not the case, it may lead to wrong results and/or undefined behaviour of the program.

Valid example schedules:
```
8192
1024, 1024, 2048
1024, 1024, 2048, 4096
512, 512, 1024, 2048, 4096, 8192, 16384
...
```

The base block size is always determined by the smallest element in the schedule (i.e. the first element).
You always must feed audio blocks of that size to `block_convolve` or `block_convolve_fft`.

## Overview of necessary steps
```C
#include "convolve.h"

int main(void)
{
    int32_t num_impulse_response_samples;
    float* impulse_response = (float*)malloc(num_impulse_response_samples * sizeof(float));
    // ... fill impulse response with audio data ...

    // create schedule
    convolve_schedule sched = { (int32_t[]){ 1024, 1024, 2048, 4096 }, 4 }; 

    // make the "plan"
    convolve_data cData = create_convolve_data(sched, num_impulse_response_samples, IR_NO_FLAGS);

    // Transform raw impulse response audio into complex array
    IR_COMPLEX_T* complexIr = malloc(num_impulse_response_samples * sizeof(IR_COMPLEX_T));
    memset(complexIr, 0, num_impulse_response_samples * sizeof(IR_COMPLEX_T));
    for(size_t i = 0; i < num_impulse_response_samples; i++)
    {
        complexIr[i][0] = impulse_response[i];
    }

    // Preprocess impulse response (Apply FFT) for convolution
    IR_COMPLEX_T* processedIr = malloc(cData.fftbuffer_size * sizeof(IR_COMPLEX_T));
    ir_fft(&cData, complexIr, processedIr);

    // Continually feed signal blocks to block_convolve.
    // Those signal blocks must match the size of the first entry in this convolve_schedule.
    // In this case: 1024

    // ...

    IR_COMPLEX_T signal[1024];
    IR_COMPLEX_T output[1024];
    block_convolve(&cData, processedIr, signal, output);

    // repeat as desired or in a loop

    // ...
        
    free(processedIr);
    free(complexIr);
    free(impulse_response);
    free_convolve_data(&cData);
}
```

**Note:** You may omit the part of preprocessing `complexIr` to `processedIr` if you specify the flag
`IR_ALLOCATE_FFT_BUFFER` when creating the `convolve_data`. Then you can directly pass `complexIr` to the block convolving routine. (You also have to use `block_convolve_fft` in that case instead of `block_convolve`).
Specifying this flag will calculate the FFT of the impulse response as needed during convolution.
This may however affect performance. Ideally, the impulse response only has to be processed this way once.