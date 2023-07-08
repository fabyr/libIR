# libIR
Small C-Library for performing convolution reverb

## Table of contents
- [Building](#building)
  - [Linux](#linux)
  - [Windows](#windows)
- [Usage](#usage)
- [Overview of necessary steps](#overview-of-necessary-steps)
- [C#/Dotnet interop](#cdotnet-interop)
  - [Generating test-output.wav](#generating-test-outputwav)


## Building
You can also use the provided `make-*.sh` scripts to build on linux or windows.
Make sure you have installed all dependencies.

### Linux
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

### Windows
Building on windows is ideally done with MinGW/MSYS2. (see https://www.msys2.org/ and https://www.msys2.org/docs/cmake/)

1. Install [FFTW3](http://www.fftw.org/install/windows.html) on Windows
  - Download `fftw-3.x.x-dll64.zip` (or the 32-bit version)
  - Extract to `C:\Program Files\fftw3` (create this folder manually)
  - Add `C:\Program Files\fftw3` to your system-wide PATH
  - Note: This is for being able to run libIR outside of MinGW/MSYS2
2. Open a mingw-terminal by searching `MSYS2 MINGW64` in windows
4. Run `pacman --needed --noconfirm -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-fftw` (note: this installs necessary files for compilation with fftw3)
5. Run `cd "C:\path\to\this\repository"`
6. Run `bash make-windows.sh`

## Usage
**libIR** performs a partitioned convolution.
Create a `convolve_schedule` as follows:
```C
// ... = { block_sizes, count_of_block_sizes }
convolve_schedule sched = { (int32_t[]){ 8192 }, 1 }; // Uniform block sizes

convolve_schedule sched2 = { (int32_t[]){ 1024, 1024, 2048, 4096 }, 4 }; // Non-Uniform (Fully Partitioned) block sizes
```
**NOTE:** When creating Non-Uniform convolve schedules, they always have to be of the form
`{ x, x, 2 * x, 4 * x, 8 * x, ..., 2^(n - 2) * x }` where `n` is the total number of entries in the list. `x` must always be a power of 2. 
If any of those conditions is not met, it may lead to wrong results and/or undefined behaviour of the program.

You may also not use any size greater than `16384`. As the precision limitations will
return wrong results at anything beyond that value.

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
// This may not be functional code
// For demonstrative purposes
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

**Note:** You can omit the part of preprocessing `complexIr` to `processedIr` if you specify the flag
`IR_ALLOCATE_FFT_BUFFER` when creating the `convolve_data`. Then you can directly pass `complexIr` to the block convolving routine. (You also have to use `block_convolve_fft` in that case instead of `block_convolve`).
Specifying this flag will calculate the FFT of the impulse response as needed during convolution.
This may however affect performance. Ideally, the impulse response only has to be processed this way once.

## C#/Dotnet interop
(You need to have the new [.NET SDK 6](https://dotnet.microsoft.com/en-us/download/dotnet/6.0) installed for the libirsharp projects)

You can also easily interop with this library from C#. (For example for use in game engines such as `Unity`)

Add [LibIR.cs](/libirsharp/LibIR.cs) to your project and use the `Convolver`-class from the namespace `LibIR`. Or add a project reference to `libirsharp.csproj`.
Make sure the C#-Application can find the compiled native C library.

Refer to [Program.cs](/libirsharp.examples/Program.cs) for a demonstrative example.

And make sure that the [/libirsharp/libs](/libirsharp/libs/) folder contains the compiled library before trying to build the dotnet application. (Using the `make-*.sh` script to build will automatically copy the built library to that location)

### Generating test-output.wav
Build *libIR* for your corresponding platform using the provided `make-*.sh` script (see [Building](#building)) and run the `libirsharp.example` application as follows:
```
cd libirsharp.examples/
dotnet run
```
If the output is `Done!`, the file [test-files/test-output.wav](/test-files/test_output.wav) has successfully been generated.
You should ideally save the original `test-output.wav` to be able to compare any
differences. If everything went right they should be identical.