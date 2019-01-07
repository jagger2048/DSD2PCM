#pragma once
#include <stdio.h>
#include <stdint.h>
#include "noiseshape.c"
#include "dsd2pcm.h"

#include <iostream>
using namespace std;


#define CLIP(min,v,max)  ( (v) < (min) )? (min) : ( ( (v) > (max) )? (max) : (v) )

const float my_ns_coeffs[] = {
	//     b1           b2           a1           a2
	-1.62666423,  0.79410094,  0.61367127,  0.23311013,  // section 1
	-1.44870017,  0.54196219,  0.03373857,  0.70316556   // section 2
};

const int my_ns_soscount = sizeof(my_ns_coeffs) / (sizeof(my_ns_coeffs[0]) * 4);

inline long myround(float x)
{
	return (long)(x + (x >= 0 ? 0.5f : -0.5f));
}


const unsigned int nTaps = 48;
float state[nTaps]{};
// 48 TAPS FIR lpf filter, fc = 30khz
float coeffs[nTaps] = {
0.00314107879527084,
0.00330234574654381,
0.00378306745048105,
0.00457469702499321,
0.00566313575729732,
0.00702898449852343,
0.00864788973563851,
0.0104909781648272,
0.0125253720090032,
0.0147147758791776,
0.0170201247007765,
0.0194002811345400,
0.0218127700368632,
0.0242145368421757,
0.0265627163220509,
0.0288153979898410,
0.0309323744790802,
0.0328758595276377,
0.0346111627422076,
0.0361073090895091,
0.0373375920477472,
0.0382800505368185,
0.0389178611072629,
0.0392396383817332,
0.0392396383817332,
0.0389178611072629,
0.0382800505368185,
0.0373375920477472,
0.0361073090895091,
0.0346111627422076,
0.0328758595276377,
0.0309323744790802,
0.0288153979898410,
0.0265627163220509,
0.0242145368421757,
0.0218127700368632,
0.0194002811345400,
0.0170201247007765,
0.0147147758791776,
0.0125253720090032,
0.0104909781648272,
0.00864788973563851,
0.00702898449852343,
0.00566313575729732,
0.00457469702499321,
0.00378306745048105,
0.00330234574654381,
0.00314107879527084,
};
float fir_smpl_circle_f32(int order, float sample, const float* coeffs, float* buffer, unsigned int* state) {
	float accu = 0.0f;
	int i = order - 1;
	buffer[*state] = sample;
	if (++(*state) >= order) { *state = 0; }
	for (; i >= 0; --i)
	{
		accu += buffer[*state] * coeffs[i];
		if (++(*state) >= order)
			*state = 0;
	}
	return accu;
}

struct DSD
{
	// uint64_t  a 8 bytes,64 bits unsigned int, 
	// uint32_t  a 4 bytes,32 bits unsigned int, 

	char dsd_chunk_header[4];		// DSD mask
	char chunk_size[8];				// size of this chunk [28]
									//uint64_t chunk_size;			// size of this chunk [28] test 
	uint64_t total_size;			// total file size
	char pMetadataChunk[4];			// the pointer point to metadata if exist,otherwise set 0.
									// 8?
	char fmt_chunk_header[4];		// fmt chunk header
	uint64_t fmt_size;				// size of this fmt chunk,usually [52] bytes
	uint32_t fmt_version;			// gersion of this format [1]
	uint32_t fmt_id;				// [0]: DSD raw
	uint32_t channel_type;			// [1]:mono [1]:stereo [3]: 3 channels [4] quad [5] 4chs [6] 5chs [7] 5.1chs
	uint32_t channel_num;			// [1]-[6] : mono~6 chs
	uint32_t sample_rate;			//  [2822400] or [5644800] hz  64fs or 128fs
	uint32_t bits_per_sample;		// [1] or [8]
	uint64_t sample_count;			// samples per channels, n second data: sample count would be fs * n
	uint32_t block_per_channel;		// fixed [4096] Bytes
	uint32_t reverd;				// fill zero

	char	data_chunk_header[4];	// [ d a t a]
	uint64_t data_size;			// equal to [n] + 12 , n is the next [n] bytes contains data 
								// next n bytes is the data
								//char* pSampleData = 0;
								// m bytes meta data chunk if have
	uint8_t* pSampleData = 0;	// a pointer points to the sample data (byte types)
	unsigned int nBytes;
};




int dsd_read(DSD *dsdfile, const char* file_name) {

	FILE *fp = NULL;
#if defined(_MSC_VER) && _MSC_VER >= 1400
	if (fopen_s(&fp, file_name, "rb") != 0) {
		return -1;
	}
#else
	fp = fopen(file_name, "rb");
	if (pFile == NULL) {
		return -1;
	}
#endif

	//fopen_s(&fp, file_name, "rb");
	//fp = fopen(file_name, "rb");
	fread(dsdfile->dsd_chunk_header, 84, 1, fp);
	fread(&dsdfile->data_size, sizeof(dsdfile->data_size), 1, fp);
	unsigned int nSamples = dsdfile->data_size - 12;				// the total size of data, nSamples / nCh
	if (nSamples != dsdfile->sample_count / 8 * dsdfile->channel_num)
	{
		//printf("Samples not match\n");
		nSamples = (dsdfile->sample_count / 8 * dsdfile->channel_num / 4096) * 4096;
	}
	// read the raw DSD data
	dsdfile->pSampleData = new uint8_t[nSamples]{};				// Initialize
	dsdfile->nBytes = nSamples;

	fread(dsdfile->pSampleData, nSamples, 1, fp);				// read

	printf("     DSD file messages:\n");
	printf("================================\n");
	printf("Sample rate	:	%d\n", dsdfile->sample_rate);
	printf("DSD version	:	%d\n", dsdfile->fmt_version);
	printf("Channels	:	%d\n", dsdfile->channel_num);
	printf("Bit per sample	:	%d\n", dsdfile->bits_per_sample);
	printf("Samples	count	:	%d\n", dsdfile->sample_count);
	printf("Total bytes	:	%d\n", dsdfile->nBytes);
	printf("Block size	:	%d\n", dsdfile->block_per_channel);
	printf("Duration(s)	:	%d\n", dsdfile->sample_count / dsdfile->sample_rate);
	printf("================================\n");

	fclose(fp);
	return 0;
}

int dsd_decode(DSD *dsdfile, float *pFloat_out[2], size_t &samples_per_ch) {

	// initialize the noise shape
	noise_shape_ctx_s* pNs[2];
	pNs[0] = (noise_shape_ctx_s *)malloc(sizeof(noise_shape_ctx_s));
	pNs[1] = (noise_shape_ctx_s *)malloc(sizeof(noise_shape_ctx_s));
	noise_shape_init(pNs[0], my_ns_soscount, my_ns_coeffs);
	noise_shape_init(pNs[1], my_ns_soscount, my_ns_coeffs);

	// initialize the decode module
	dsd2pcm_ctx *d2p[2]{};
	d2p[0] = dsd2pcm_init();
	d2p[1] = dsd2pcm_init();
	if (d2p[0] == NULL)
	{
		printf("D2P initialize erroes\n");
		return -1;
	}
	// set the decode parameters
	const int block = 4096;									// default is |4096| for DSF file.
	int channels = dsdfile->channel_num;
	int lsbitfirst = 1;										// default is |1| for DSF file.

	float *fTmp = (float *)malloc(dsdfile->nBytes * sizeof(float));
	pFloat_out[0] = fTmp;
	pFloat_out[1] = fTmp + dsdfile->nBytes / 2;				// right channel

	samples_per_ch = dsdfile->nBytes / 2;
	size_t nFrames = dsdfile->nBytes / (block * channels);
	size_t upIndex[2] = { 0,0 };
	
	// test case : output a 


	// decode to 352khz 
	for (size_t n = 0; n < nFrames; n++)
	{
		for (int c = 0; c < channels; ++c) {

			dsd2pcm_translate(
				d2p[c],
				block,
				dsdfile->pSampleData + n * block * channels + c * block,
				1,
				lsbitfirst,
				pFloat_out[c] + upIndex[c],
				1
			);

			// Noise shaping
			//*(pFloat_out[c] + upIndex[c]) += noise_shape_get(pNs[c]);

			//long smp =CLIP(-32768, myround(*(pFloat_out[c] + upIndex[c]) ) , 32767);

			//noise_shape_update(pNs[c], CLIP(-1, smp - *(pFloat_out[c] + upIndex[c]) , 1) );

			upIndex[c] += block;
		}
	}
	dsd2pcm_destroy(d2p[0]);
	dsd2pcm_destroy(d2p[1]);

	noise_shape_destroy(pNs[0]);
	noise_shape_destroy(pNs[1]);

	samples_per_ch = upIndex[0];
	return 0;
}

struct FIR
{
	int buffer_size;
	int buffer_mask;
	int bpos;
	unsigned int half_order;
	float *buffer;
	float *filter_coeffs;
	float *filter_coeffs_full;
};
void FIR_Init(FIR* handles,float *half_coeffs, unsigned int half_order) {
	//FIR *handles = (FIR*)malloc(sizeof(FIR));
	int k = 1;
	while (k < half_order * 2)
	{
		k <<= 1;
	}
	printf("%d \n", k);
	if (k != half_order *2)
	{
		printf("%d,%d\n", half_order, k);
	}
	handles->buffer_size = k;
	handles->buffer_mask = k - 1;
	//handles->buffer_size = half_order * 2;
	//handles->buffer_mask = half_order * 2 - 1;
	handles->bpos = 0;
	handles->buffer = (float *)malloc(sizeof(float) * handles->buffer_size);
	handles->filter_coeffs = (float *)malloc(sizeof(float) * half_order);
	handles->filter_coeffs_full = (float *)malloc(sizeof(float) * half_order * 2); // test 
	handles->half_order = half_order;
	memset(handles->buffer, 0, sizeof(float) * handles->buffer_size);
	memcpy(handles->filter_coeffs, half_coeffs, sizeof(float) * half_order);
	memcpy(handles->filter_coeffs_full, half_coeffs, sizeof(float) * half_order); // test
	for (size_t i = 0; i < half_order; i++)		// test
	{
		handles->filter_coeffs_full[ half_order*2 - i -1] = handles->filter_coeffs_full[i];
	}
}
//int FIR_Create(FIR * handles,float *half_coeffs, unsigned int half_order) {
//	handles = FIR_Init(half_coeffs,half_order);
//	if (handles == NULL)
//	{
//		return -1;
//	}
//	return 0;
//}
int FIR_Process(FIR *handles,float *in,unsigned int input_offest,float *out,unsigned int out_offest,unsigned int nSample,unsigned nStep) {
	// defalut input_offest out_offest is |0|,nStep is |1|
	if (nSample < nStep)
	{
		return -1;
	}
	for (size_t n = 0; n < nSample; n+= nStep)
	{
		// push data into the FIFO buffer，NOTE that in some cases,buffer_length is not equal to filter order
		// buffer_length must be a power of 2 >= filter order in order to use |&| operator.
		for (size_t step_count = 0; step_count < nStep; step_count++)
		{
			handles->buffer[handles->bpos + step_count] = in[input_offest++];
		}
		handles->bpos = (handles->bpos + nStep) & (handles->buffer_mask);
		//handles->bpos = (handles->bpos + nStep > handles->buffer_mask ) ? 0:(handles->bpos + nStep);

		float sample = 0;
		int rr = handles->bpos;															// right seek index
		int ll = (handles->bpos + handles->half_order * 2 -1) & handles->buffer_mask;	// left seek index	% handles->buffer_mask;

		for (size_t kk = 0; kk < handles->half_order; kk++)
		{
			//sample += handles->filter_coeffs[kk] * (handles->buffer[(rr++)&handles->buffer_mask] + handles->buffer[(ll--)&handles->buffer_mask]);
			// 使用 +- kk 代替 ++ -- 避免 ll rr 数据溢出
			sample += handles->filter_coeffs[kk] * (handles->buffer[(rr + kk)&handles->buffer_mask] + handles->buffer[(ll - kk)&handles->buffer_mask]);
		}
		out[out_offest++] = sample ;
	}
	return 0;
}

// backup 
int FIR_Process_without_folding(FIR *handles, float *in, unsigned int input_offest, float *out, unsigned int out_offest, unsigned int nSample, unsigned nStep) {
	// defalut input_offest out_offest is |0|,nStep is |1|
	if (nSample < nStep)
	{
		return -1;
	}
	for (size_t n = 0; n < nSample; n += nStep)
	{
		// push data into the FIFO buffer
		for (size_t step_count = 0; step_count < nStep; step_count++)
		{
			handles->buffer[handles->bpos + step_count] = in[input_offest++];
		}

		//handles->bpos = (handles->bpos + nStep) & (handles->buffer_mask);// order must be a power of 2 if you want to use this method.
		handles->bpos = (handles->bpos + nStep > handles->buffer_mask) ? 0 : (handles->bpos + nStep);

		float sample = 0;
		int index = handles->bpos;

		// without folding structure
		////for (int kk = handles->half_order * 2 -1; kk >=0; kk--)
		//for (size_t kk = 0; kk < handles->half_order * 2 ; kk++)
		//{
		//	sample += handles->buffer[ index ] * handles->filter_coeffs_full[kk];
		//	index = (index + 1) > (handles->buffer_mask)? 0 : index + 1;
		//	//sample += handles->buffer[ index & handles->buffer_mask ] * handles->filter_coeffs_full[kk];
		//}

		int rr = handles->bpos;			// right seek index
		//int ll0 = handles->bpos - 1;		// left seek index(if buffer len equals FIR order)
		//if (ll0 < 0)
		//{
		//	ll0 = handles->buffer_mask;
		//}
		int ll = (handles->bpos + handles->half_order * 2 - 1);// % handles->buffer_mask;
		if (ll > handles->buffer_size)
		{
			ll = ll - handles->buffer_size;
		}
		for (size_t kk = 0; kk < handles->half_order; kk++)
		{
			sample += handles->filter_coeffs[kk] * (handles->buffer[rr] + handles->buffer[ll]);
			rr = (rr + 1) > (handles->buffer_mask) ? 0 : rr + 1;
			ll = (ll - 1) >= (0) ? ll - 1 : handles->buffer_mask;
		}
		out[out_offest++] = sample;
	}
	return 0;
}

int FIR_Destory(FIR* handles) {
	if (handles != NULL)
	{
		free(handles->buffer);
		free(handles->filter_coeffs);
		free(handles->filter_coeffs_full);
		free(handles);
	}
	return 0;
}

void FIR_test(unsigned int order,float *input,float *output,unsigned int nSample,float *fir_coeffs,float *buffer,int pos){
	// test passed
	for (size_t i = 0; i < order ; i++)
	{
		buffer[i] = 0;
	}
	unsigned int buffer_size = order - 1;
	for (size_t n = 0; n < nSample; n++)
	{
		buffer[pos] = input[n];	// push data in
		//pos = (pos + 1) & buffer_size;	// order must be a power of 2 if you want to use this method.
		if (++ pos >= order)
		{
			pos = 0;
		}
		float sum = 0.0f;
		int p = pos;
		for (int i = order -1 ; i >=0; --i)
		{
			//cout << i << endl;
			sum += fir_coeffs[i] * (buffer[ p ]);
			//p = (p + 1)&buffer_size;
			if (++ p >= order)
			{
				p = 0;
			}
		}
		output[n] = sum;
	}
}

// to be deleted 
/*
struct  Biquad
{
	float coeffs[2][3]{};
	float state[3]{};
};
void biquad(Biquad *handles, float in, float &out) {
	// wrapped methods
	handles->state[2] = handles->state[1];
	handles->state[1] = handles->state[0];
	handles->state[0] = in + (-handles->coeffs[1][1]) * handles->state[1] + (-handles->coeffs[1][2]) * handles->state[2];
	// caculate the output
	out = handles->coeffs[0][0] * handles->state[0] + handles->coeffs[0][1] * handles->state[1] + handles->coeffs[0][2] * handles->state[2];
}

namespace {

	const float my_ns_coeffs[] = {
		//     b1           b2           a1           a2
		-1.62666423,  0.79410094,  0.61367127,  0.23311013,  // section 1
		-1.44870017,  0.54196219,  0.03373857,  0.70316556   // section 2
	};

	const int my_ns_soscount = sizeof(my_ns_coeffs) / (sizeof(my_ns_coeffs[0]) * 4);

	inline long myround(float x)
	{
		return static_cast<long>(x + (x >= 0 ? 0.5f : -0.5f));
	}

	template<typename T>
	struct id { typedef T type; };

	template<typename T>
	inline T clip(
		typename id<T>::type min,
		T v,
		typename id<T>::type max)
	{
		if (v < min) return min;
		if (v > max) return max;
		return v;
	}

	inline void write_intel16(unsigned char * ptr, unsigned word)
	{
		ptr[0] = word & 0xFF;
		ptr[1] = (word >> 8) & 0xFF;
	}

	inline void write_intel24(unsigned char * ptr, unsigned long word)
	{
		ptr[0] = word & 0xFF;
		ptr[1] = (word >> 8) & 0xFF;
		ptr[2] = (word >> 16) & 0xFF;
	}

} // anonymous namespace
*/
