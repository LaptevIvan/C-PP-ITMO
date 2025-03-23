#include "CrossCorrelation.h"
#include "Decode.h"
#include "return_codes.h"

#include <stdio.h>

int main(int argc, char** argv)
{
	if (argc != 2 && argc != 3)
	{
		fprintf(stderr, "Arguments is invalid\n");
		return ERROR_ARGUMENTS_INVALID;
	}
	BufferSamples amplitudes1, amplitudes2;
	int32_t assert;
	if ((argc == 2 && (assert = decodeAudioFiles(argv[1], NULL, &amplitudes1, &amplitudes2)) != SUCCESS) ||
		(argc == 3 && (assert = decodeAudioFiles(argv[1], argv[2], &amplitudes1, &amplitudes2)) != SUCCESS))
		return assert;
	double* res = NULL;
	assert = crossCorrelation(&amplitudes1, &amplitudes2, &res);
	if (assert == SUCCESS)
	{
		int32_t indMax = 0;
		double maxVal = res[0];
		for (int32_t i = 1; i < amplitudes1.len; ++i)
			if (maxVal < res[i])
			{
				maxVal = res[i];
				indMax = i;
			}
		int32_t diffSampl = indMax > amplitudes1.len / 2 ? indMax - amplitudes1.len : indMax;
		int32_t sample_rate = amplitudes1.sample_rate;
		int32_t diffTime = diffSampl * 1000 / sample_rate;
		printf("delta: %i samples\nsample rate: %i Hz\ndelta time: %i ms\n", diffSampl, sample_rate, diffTime);
	}
	freeBufferSamples(&amplitudes1);
	freeBufferSamples(&amplitudes2);
	freeComponents(res, NULL, NULL, NULL);
	return assert;
}
