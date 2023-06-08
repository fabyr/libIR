#include <fftw3.h>
#ifdef SINGLE_PRECISION
typedef float IR_FLOAT_T;
typedef fftwf_complex IR_COMPLEX_T;
#define ir_fftw_malloc fftwf_malloc
#define ir_fftw_free fftwf_free
#define ir_fftw_plan_dft_1d fftwf_plan_dft_1d
#define ir_fftw_plan fftwf_plan
#define ir_fftw_destroy_plan fftwf_destroy_plan
#define ir_fftw_execute fftwf_execute
#else
typedef double IR_FLOAT_T;
typedef fftw_complex IR_COMPLEX_T;
#define ir_fftw_malloc fftw_malloc
#define ir_fftw_free fftw_free
#define ir_fftw_plan_dft_1d fftw_plan_dft_1d
#define ir_fftw_plan fftw_plan
#define ir_fftw_destroy_plan fftw_destroy_plan
#define ir_fftw_execute fftw_execute
#endif