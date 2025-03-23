#include "CrossCorrelation.h"

#include "return_codes.h"

#include <fftw3.h>
#include <stdio.h>

typedef enum
{
	r2c,
	c2r
} flagTransform;

void executeRealComplexPlan(int32_t len, double* real, fftw_complex* complex, flagTransform flag)
{
	fftw_plan plan =
		flag == r2c ? fftw_plan_dft_r2c_1d(len, real, complex, FFTW_ESTIMATE) : fftw_plan_dft_c2r_1d(len, complex, real, FFTW_ESTIMATE);
	fftw_execute(plan);
	fftw_destroy_plan(plan);
}

int toFftwArr(BufferSamples* inp, double** out)
{
	int32_t len = inp->len;
	double* arrFftw = fftw_alloc_real(len);
	if (!arrFftw)
	{
		fprintf(stderr, "Not enough memory\n");
		return ERROR_NOTENOUGH_MEMORY;
	}
	*out = arrFftw;
	uint8_t* arr = inp->samples;
	for (int32_t i = 0; i < len; ++i)
		arrFftw[i] = (double)arr[i];
	return SUCCESS;
}

void freeComponents(double* real1, double* real2, fftw_complex* complex1, fftw_complex* complex2)
{
	fftw_free(real1);
	fftw_free(real2);
	fftw_free(complex1);
	fftw_free(complex2);
}

int crossCorrelation(BufferSamples* inp1, BufferSamples* inp2, double** res)
{
	int32_t len = inp1->len;
	double *arr1 = NULL, *arr2 = NULL;
	int32_t assert;
	if ((assert = toFftwArr(inp1, &arr1)) != SUCCESS || (assert = toFftwArr(inp2, &arr2)) != SUCCESS)
	{
		freeComponents(arr1, arr2, NULL, NULL);
		return assert;
	}
	fftw_complex* comInput1 = (fftw_complex*)fftw_alloc_complex(len);
	fftw_complex* comInput2 = (fftw_complex*)fftw_alloc_complex(len);
	if (!comInput1 || !comInput2)
	{
		fprintf(stderr, "Not enough memory\n");
		freeComponents(arr1, arr2, comInput1, comInput2);
		return ERROR_NOTENOUGH_MEMORY;
	}
	executeRealComplexPlan(len, arr2, comInput2, r2c);
	fftw_free(arr2);
	executeRealComplexPlan(len, arr1, comInput1, r2c);
	for (int32_t i = 0; i < len; ++i)
	{
		double a = comInput1[i][0], b = comInput1[i][1], c = comInput2[i][0], d = comInput2[i][1];
		comInput1[i][0] = a * c + b * d;
		comInput1[i][1] = -a * d + b * c;
	}
	fftw_free(comInput2);
	executeRealComplexPlan(len, arr1, comInput1, c2r);
	fftw_free(comInput1);
	*res = arr1;
	return SUCCESS;
}
