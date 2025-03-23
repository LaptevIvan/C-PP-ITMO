#include "BufferSamples.h"
#include "return_codes.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>

#include <stdio.h>
#include <string.h>
#define START_SIZE 1000
#define NUMBER_CODEC_TYPE 5
enum AVCodecID CODE_TYPE[] = { AV_CODEC_ID_FLAC, AV_CODEC_ID_MP2, AV_CODEC_ID_MP3, AV_CODEC_ID_OPUS, AV_CODEC_ID_AAC };

typedef struct
{
	AVFormatContext* contextFile;
	const AVCodec* codec;
	AVCodecContext* contextCodec;
	AVCodecParameters* parametersCodec;
	AVPacket* packet;
	AVFrame* frame;
	uint8_t ind_channel;
	uint8_t ind_stream;
} Container;

int resizeBufferSample(BufferSamples* buffSampl, int32_t newSize)
{
	uint8_t* samples = buffSampl->samples;
	uint8_t* newSamples;
	if (!(newSamples = (uint8_t*)realloc(samples, newSize)))
	{
		fprintf(stderr, "Not enough memory\n");
		return ERROR_NOTENOUGH_MEMORY;
	}
	buffSampl->samples = newSamples;
	buffSampl->len = newSize;
	return SUCCESS;
}

int checkResize(BufferSamples* buffSampl, uint8_t** arr, int32_t currentInd, int32_t* len)
{
	int32_t currentLen = *len;
	if (buffSampl && currentInd >= currentLen)
	{
		uint64_t doubleInd = currentInd * 2;
		int32_t newLen = doubleInd < INT_MAX ? doubleInd : INT_MAX;
		if (resizeBufferSample(buffSampl, newLen) != SUCCESS)
			return ERROR_NOTENOUGH_MEMORY;
		*arr = buffSampl->samples;
		*len = newLen;
	}
	return SUCCESS;
}

int convertFrame(SwrContext* swr, AVFrame* frame, AVFrame** outFrame, uint16_t* nb_samples, uint8_t nb_channels, int32_t out_sample_rate)
{
	AVFrame* outLocalFrame = av_frame_alloc();
	if (!(outLocalFrame))
	{
		fprintf(stderr, "Not enough memory\n");
		return ERROR_NOTENOUGH_MEMORY;
	}
	*outFrame = outLocalFrame;
	outLocalFrame->sample_rate = out_sample_rate;
	outLocalFrame->format = frame->format;
	uint16_t cur_nb_samples = *nb_samples;
	int32_t new_nb_samples = swr_get_out_samples(swr, cur_nb_samples);
	if (new_nb_samples < 0)
	{
		fprintf(stderr, "Unknown failure with doing swr convert\n");
		return ERROR_UNKNOWN;
	}
	uint8_t** outData = outLocalFrame->data;
	if (av_samples_alloc(outData, NULL, nb_channels, new_nb_samples, AV_SAMPLE_FMT_U8P, 0) < 0)
	{
		fprintf(stderr, "Not enough memory\n");
		return ERROR_NOTENOUGH_MEMORY;
	}
	uint8_t** inData = frame->data;
	new_nb_samples = swr_convert(swr, outData, new_nb_samples, inData, cur_nb_samples);
	if (new_nb_samples < 0)
	{
		fprintf(stderr, "Unknown failure with doing swr convert\n");
		return ERROR_UNKNOWN;
	}
	*nb_samples = new_nb_samples;
	return SUCCESS;
}

int decodeSamplesFromFrame(SwrContext* swr, AVPacket* packet, AVCodecContext* contextCodec, AVFrame* frame, BufferSamples* buffSampl1, BufferSamples* buffSampl2, int32_t out_sample_rate)
{
	int32_t assert = avcodec_send_packet(contextCodec, packet);
	if (assert < 0)
	{
		fprintf(stderr, "Error while sending a packet to the decoder\n");
		return ERROR_UNKNOWN;
	}
	int32_t curInd = buffSampl1->currentInd;
	int32_t len1 = buffSampl1->len, len2 = buffSampl2 ? buffSampl2->len : -1;
	uint8_t *samples1 = buffSampl1->samples, *samples2 = buffSampl2 ? buffSampl2->samples : NULL;
	while (assert >= 0)
	{
		assert = avcodec_receive_frame(contextCodec, frame);
		if (assert == AVERROR(EAGAIN))
			continue;
		else if (assert == AVERROR_EOF || assert == AVERROR(ENOMEM) || assert < 0)
		{
			fprintf(stderr, "Error while receiving a frame from the decoder\n");
			return ERROR_UNKNOWN;
		}
		AVFrame* localFrame = NULL;
		uint16_t nb_samples = frame->nb_samples;
		if ((assert = convertFrame(swr, frame, &localFrame, &nb_samples, samples2 ? 2 : 1, out_sample_rate)) != SUCCESS)
			return assert;
		uint8_t** data = localFrame->data;
		if ((assert = checkResize(buffSampl1, &samples1, curInd + nb_samples, &len1)) != SUCCESS ||
			(assert = checkResize(buffSampl2, &samples2, curInd + nb_samples, &len2)) != SUCCESS)
			return assert;
		memcpy(samples1 + curInd, data[0], nb_samples);
		if (samples2)
			memcpy(samples2 + curInd, data[1], nb_samples);
		curInd += nb_samples;
		av_frame_unref(frame);
		av_frame_unref(localFrame);
		av_frame_free(&localFrame);
	}
	buffSampl1->currentInd = curInd;
	if (buffSampl2)
		buffSampl2->currentInd = curInd;
	return SUCCESS;
}

int decodeSamplesFromPacket(Container* cont, BufferSamples* buffSampl1, BufferSamples* buffSampl2, int32_t out_sample_rate)
{
	AVCodecContext* contextCodec = cont->contextCodec;
	if (avcodec_open2(contextCodec, cont->codec, NULL) < 0)
	{
		fprintf(stderr, "Decoder isn't supported\n");
		return ERROR_DATA_INVALID;
	}
	uint8_t ind_stream = cont->ind_stream;
	AVFormatContext* contextFile = cont->contextFile;
	AVPacket* packet = cont->packet;
	AVFrame* frame = cont->frame;
	int32_t assert;
	SwrContext* swr = NULL;
	if (swr_alloc_set_opts2(
			&swr,
			&contextCodec->ch_layout,
			AV_SAMPLE_FMT_U8P,
			out_sample_rate,
			&contextCodec->ch_layout,
			contextCodec->sample_fmt,
			contextCodec->sample_rate,
			0,
			NULL) != 0)
	{
		fprintf(stderr, "Not enough memory\n");
		return ERROR_NOTENOUGH_MEMORY;
	}
	if (swr_init(swr) < 0)
	{
		fprintf(stderr, "Unknown failure with doing swr convert\n");
		return ERROR_UNKNOWN;
	}
	while (av_read_frame(contextFile, packet) >= 0)
	{
		if (packet->stream_index == ind_stream &&
			(assert = decodeSamplesFromFrame(swr, packet, contextCodec, frame, buffSampl1, buffSampl2, out_sample_rate)) != SUCCESS)
			return assert;
		av_packet_unref(packet);
	}
	swr_free(&swr);
	buffSampl1->sample_rate = out_sample_rate;
	return SUCCESS;
}

int openFile(char* file, Container* cont)
{
	AVFormatContext* contextFile = avformat_alloc_context();
	if (!contextFile)
	{
		fprintf(stderr, "Not enough memory\n");
		return ERROR_NOTENOUGH_MEMORY;
	}
	if (avformat_open_input(&contextFile, file, NULL, NULL) != 0 || avformat_find_stream_info(contextFile, NULL) < 0)
	{
		fprintf(stderr, "Could not get the stream info\n");
		return ERROR_CANNOT_OPEN_FILE;
	}
	cont->contextFile = contextFile;
	return SUCCESS;
}

int openCodec(Container* cont)
{
	AVFormatContext* format = cont->contextFile;
	uint8_t ind_stream = av_find_best_stream(format, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, -1);
	if (ind_stream < 0 || ind_stream > format->nb_streams)
	{
		fprintf(stderr, "There isn't audio stream in file\n");
		return ERROR_FORMAT_INVALID;
	}
	AVStream** streams = format->streams;
	AVCodecParameters* parametersCodec = streams[ind_stream]->codecpar;
	enum AVCodecID idFoundCodec = parametersCodec->codec_id;
	uint8_t assert = 0;
	for (uint8_t i = 0; i < NUMBER_CODEC_TYPE; ++i)
		if (CODE_TYPE[i] == idFoundCodec)
		{
			assert = 1;
			break;
		}
	if (!assert)
	{
		fprintf(stderr, "Not supported audio codec\n");
		return ERROR_FORMAT_INVALID;
	}
	const AVCodec* codec = avcodec_find_decoder(idFoundCodec);
	AVCodecContext* codecContext = avcodec_alloc_context3(codec);
	if (!codecContext)
	{
		fprintf(stderr, "Not enough memory to AVCodecContext\n");
		return ERROR_NOTENOUGH_MEMORY;
	}
	if (avcodec_parameters_to_context(codecContext, parametersCodec) < 0)
	{
		fprintf(stderr, "Failed to copy codec parameters to codec context\n");
		return ERROR_DATA_INVALID;
	}
	AVPacket* packet = av_packet_alloc();
	AVFrame* frame = av_frame_alloc();
	if (!packet || !frame)
	{
		fprintf(stderr, "Not enough memory\n");
		return ERROR_NOTENOUGH_MEMORY;
	}
	cont->codec = codec;
	cont->ind_stream = ind_stream;
	cont->contextCodec = codecContext;
	cont->parametersCodec = parametersCodec;
	cont->packet = packet;
	cont->frame = frame;
	return SUCCESS;
}

int finishingZeros(BufferSamples* buffSampl, int32_t newLen)
{
	int32_t assert;
	if ((assert = resizeBufferSample(buffSampl, newLen)) != SUCCESS)
		return assert;
	int32_t currentInd = buffSampl->currentInd;
	if (currentInd < newLen)
	{
		uint8_t* samples = buffSampl->samples;
		while (currentInd != newLen)
			samples[currentInd++] = 0;
		buffSampl->currentInd = currentInd;
	}
	return SUCCESS;
}

void cleanGarbage(Container* cont)
{
	if (cont)
	{
		cont->contextFile = NULL;
		cont->codec = NULL;
		cont->contextCodec = NULL;
		cont->parametersCodec = NULL;
		cont->packet = NULL;
		cont->frame = NULL;
	}
}

void freeContainer(Container* cont)
{
	if (cont)
	{
		avformat_close_input(&cont->contextFile);
		if (cont->contextCodec)
			avcodec_free_context(&cont->contextCodec);
		av_packet_free(&cont->packet);
		av_frame_free(&cont->frame);
		free(cont);
	}
}

int endOfDecode(int returnsCode, Container* cont1, Container* cont2, BufferSamples* buffSampl1, BufferSamples* buffSampl2)
{
	freeContainer(cont1);
	freeContainer(cont2);
	freeBufferSamples(buffSampl1);
	freeBufferSamples(buffSampl2);
	return returnsCode;
}

int decodeAudioFiles(char* file1, char* file2, BufferSamples* out1, BufferSamples* out2)
{
	av_log_set_level(AV_LOG_QUIET);
	Container *cont1 = malloc(sizeof(Container)), *cont2 = file2 ? malloc(sizeof(Container)) : NULL;
	if (!cont1 || (file2 && !cont2))
	{
		fprintf(stderr, "Not enough memory\n");
		return endOfDecode(ERROR_NOTENOUGH_MEMORY, cont1, cont2, NULL, NULL);
	}
	cleanGarbage(cont1);
	cleanGarbage(cont2);
	int32_t assert;
	if ((assert = openFile(file1, cont1)) != SUCCESS || (file2 && (assert = openFile(file2, cont2)) != SUCCESS))
		return endOfDecode(assert, cont1, cont2, NULL, NULL);
	if ((assert = openCodec(cont1)) != SUCCESS || (file2 && (assert = openCodec(cont2)) != SUCCESS))
		return endOfDecode(assert, cont1, cont2, NULL, NULL);
	if (!file2 && cont1->parametersCodec->ch_layout.nb_channels != 2)
	{
		fprintf(stderr, "One non-two-channel audio file has been entered\n");
		return endOfDecode(ERROR_FORMAT_INVALID, cont1, NULL, NULL, NULL);
	}
	int32_t sampl1 = cont1->contextCodec->sample_rate, sampl2 = file2 ? cont2->contextCodec->sample_rate : sampl1;
	int32_t samplMax = sampl1 > sampl2 ? sampl1 : sampl2;
	if ((assert = createBufferSamples(out1, START_SIZE, sampl1)) != SUCCESS ||
		(assert = createBufferSamples(out2, START_SIZE, sampl2)) != SUCCESS)
		return endOfDecode(assert, cont1, cont2, out1, out2);

	if ((assert = decodeSamplesFromPacket(cont1, out1, file2 ? NULL : out2, samplMax)) != SUCCESS ||
		(file2 && (assert = decodeSamplesFromPacket(cont2, out2, NULL, samplMax)) != SUCCESS))
		return endOfDecode(assert, cont1, cont2, out1, out2);

	int32_t len1 = out1->currentInd, len2 = out2->currentInd, lenMax = len1 > len2 ? len1 : len2;
	if ((assert = finishingZeros(out1, lenMax)) != SUCCESS || (assert = finishingZeros(out2, lenMax)) != SUCCESS)
		return endOfDecode(assert, cont1, cont2, out1, out2);

	return endOfDecode(SUCCESS, cont1, cont2, NULL, NULL);
}
