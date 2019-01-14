#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

// FIR Halfband filter
struct FIR_Halfband
{
	unsigned int half_order;
	unsigned int nTaps;
	int buffer_size;	// FIFO buffer size
	int buffer_mask;	// FIFO buffer mask
	int bpos;			// FIFO POS
	float *buffer;		// FIFO buffer
	float *half_coeffs;
	float *full_coeffs;
};

void fir_halfband_init(FIR_Halfband *handles ,unsigned int half_order, float *half_coeffs) {
	// half_coeffs: 0,b1,0,b3 ...... b_n-1
	// https://fr.mathworks.com/help/dsp/ref/dsp.firhalfbanddecimator-system-object.html
	// [1] Harris, F.J. Multirate Signal Processing for Communication Systems, Prentice Hall, 2004, chapter 8. pp. 208–209.

	// 
	int k = 1;
	//int full_order = 0;
	//if (half_order %2 == 0)
	//{
	//	full_order = half_order * 2;
	//}
	//else
	//{
	//	full_order = half_order * 2 + 1;
	//}
	while (k < half_order * 2 + 1)
	{
		k <<= 1;
	}
	handles->nTaps = 2 * half_order + 1;
	handles->half_order = half_order ;
	handles->buffer_size = k;		// 未 polyphase 优化版本，后续换上长度为 2 的倍数的buffer
	handles->buffer_mask = k-1 ;
	handles->bpos = 0;

	handles->buffer = (float *)malloc(sizeof(float)* handles->buffer_size);

	handles->half_coeffs = (float *)malloc(sizeof(float)* half_order / 2 );
	handles->full_coeffs = (float *)malloc(sizeof(float) *(half_order * 2 + 1));

	for (size_t n = 0; n < half_order / 2 ; n++)
	{
		handles->half_coeffs[n] = half_coeffs[ n*2 + 1];	// 去掉 0 
	}

	// 记录完整系数 , 转化后注意检查是否正确！

	memcpy(handles->full_coeffs, half_coeffs, sizeof(float) * half_order);

	for (size_t n = 0; n < half_order; n++)
	{
		handles->full_coeffs[half_order + n + 1] = half_coeffs[half_order - 1 - n];
	}
	handles->full_coeffs[half_order] = 0.5;
}

FIR_Halfband * fir_halfband_create(unsigned int half_order, float *half_coeffs) {
	FIR_Halfband * handles = (FIR_Halfband *)malloc(sizeof(FIR_Halfband));
	if (handles !=NULL)
	{
		fir_halfband_init(handles, half_order, half_coeffs);
	}
	return handles;
}
void fir_halfband_process(FIR_Halfband *handles,float *input,float*output,int nSamples,int nStep) {
	// 先使用传统对称的形式实现一遍，然后再使用 polyphase 结构来改造

	size_t out_index = 0;
	size_t in_index = 0;
	for (size_t n = 0; n < nSamples ; n+=nStep)
	{
		// push data into the FIFO buffer
		for (size_t nPush = 0; nPush < nStep; nPush++)
		{
			handles->buffer[handles->bpos + nPush] = input[in_index ++];

		}
		handles->bpos = (handles->bpos + nStep )& handles->buffer_mask;

		// process the data in the buffer

		//int index = (handles->bpos + handles->nTaps - 1 ) &handles->buffer_mask;
		int index = handles->bpos;
		float sum = 0;
		for (size_t k = 0; k < handles->half_order * 2 + 1; k++)
		{
			// 原始未优化方案：
			//sum += handles->full_coeffs[k] * handles->buffer[ ( index + k ) & handles->buffer_mask];	

			// 对称优化方案：
			sum += handles->full_coeffs[k] * (
				handles->buffer[ ( index + k ) & handles->buffer_mask]	+
				handles->buffer[(index + k) & handles->buffer_mask] 
				);	
			// half band 优化方案
		}
		// output
		output[out_index++] = sum ;
	}



}
int fir_halfband_destory(FIR_Halfband * handles) {
	if (handles !=NULL)
	{
		free(handles->buffer);
		free(handles->half_coeffs);
		free(handles->full_coeffs);
		free(handles);
		return 0;
	}
	else
	{
		return -1;
	}
}

float fir_halfband_coeffs_352[48] = {
	// half coefficients of 96-tap halfband filter
	// b1 = firhalfband(96,10^(-130/20),'dev');
0,
-1.15971333347458e-06,
0,
4.47226163129147e-06,
0,
-1.26688376323832e-05,
0,
3.00886201610525e-05,
0,
-6.35814855405609e-05,
0,
0.000123301229927384,
0,
-0.000223588050626650,
0,
0.000383877521995262,
0,
-0.000629601557205521,
0,
0.000993073562497948,
0,
-0.00151440675016558,
0,
0.00224261690181426,
0,
-0.00323723661985477,
0,
0.00457107346161312,
0,
-0.00633530218691844,
0,
0.00864917823847376,
0,
-0.0116790103963147,
0,
0.0156765710036502,
0,
-0.0210616724006100,
0,
0.0286172586160242,
0,
-0.0400211695547623,
0,
0.0596555677591845,
0,
-0.103653163003111,
0,
0.317485323265219,
};

float fir_halfband_coeffs_176[24] = {
	// half coefficients of 96-tap halfband filter
	// b1 = firhalfband(48,10^(-130/20),'dev');
0,
-7.85343785936415e-06,
0,
5.39859047938935e-05,
0,
-0.000221378903047724,
0,
0.000687644979897456,
0,
-0.00177478722465878,
0,
0.00400039597129836,
0,
-0.00813884900448511,
0,
0.0153534178633703,
0,
-0.0276321801915270,
0,
0.0494948696362693,
0,
-0.0969658882072563,
0,
0.315150464499322
};