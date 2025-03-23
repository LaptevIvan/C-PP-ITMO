#pragma once

#include <stdint.h>

typedef struct
{
	uint8_t* samples;
	int32_t currentInd;
	int32_t len;
	int32_t sample_rate;
} BufferSamples;

int createBufferSamples(BufferSamples* buffSampl, int32_t len, int32_t sample_rate);

void freeBufferSamples(BufferSamples* buffSampl);
