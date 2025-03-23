#include "BufferSamples.h"

#include "return_codes.h"

#include <malloc.h>
#include <stdio.h>

int createBufferSamples(BufferSamples* buffSampl, int32_t len, int32_t sample_rate)
{
	uint8_t* arr = (uint8_t*)malloc(len);
	if (!arr)
	{
		fprintf(stderr, "Not enough memory");
		return ERROR_NOTENOUGH_MEMORY;
	}
	buffSampl->samples = arr;
	buffSampl->currentInd = 0;
	buffSampl->len = len;
	buffSampl->sample_rate = sample_rate;
	return SUCCESS;
}

void freeBufferSamples(BufferSamples* buffSampl)
{
	if (buffSampl)
		free(buffSampl->samples);
}
