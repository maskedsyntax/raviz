#ifndef FFTW3_H
#define FFTW3_H

#include <stddef.h>

typedef double fftw_complex[2];
typedef struct fftw_plan_s *fftw_plan;

#define FFTW_ESTIMATE (1U << 6)
#define FFTW_REAL_TO_COMPLEX 1

void *fftw_malloc(size_t n);
void fftw_free(void *p);
fftw_plan fftw_plan_dft_r2c_1d(int n, double *in, fftw_complex *out, unsigned flags);
void fftw_execute(const fftw_plan p);
void fftw_destroy_plan(fftw_plan p);

#endif
