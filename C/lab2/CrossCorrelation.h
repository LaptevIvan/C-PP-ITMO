#pragma once
#include "BufferSamples.h"

#include <fftw3.h>

int crossCorrelation(BufferSamples* inp1, BufferSamples* inp2, double** res);

void freeComponents(double* real1, double* real2, fftw_complex* complex1, fftw_complex* complex2);
