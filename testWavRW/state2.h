#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
// the dsd2pcm state2 processing

// state2 coefficients in Java version dsd2pcm
float fir_coeffs_state2[48] = {
-1.33768885000000e-06,
-3.48297382000000e-06,
-5.16205946000000e-06,
-1.69056875000000e-06,
1.54737069800000e-05,
5.77230132900000e-05,
0.000135366349050000,
0.000250800926040000,
0.000390476659910000,
0.000519186769230000,
0.000581243856170000,
0.000512281970900000,
0.000262213960590000,
-0.000175303742110000,
-0.000737031734570000,
-0.00128134808988000,
-0.00160838203424000,
-0.00151339001247000,
-0.000864952623610000,
0.000314465252590000,
0.00179777272355000,
0.00316403730329000,
0.00388856110053000,
0.00350400707295000,
0.00179320983820000,
-0.00105068213937000,
-0.00436803832934000,
-0.00713285331154000,
-0.00821685345603000,
-0.00676581108107000,
-0.00258088650527000,
0.00363150734425000,
0.0102640847703500,
0.0151288677706200,
0.0160582715754000,
0.0116480229900800,
0.00192623920806000,
-0.0112844420959000,
-0.0244872239241600,
-0.0331648084859600,
-0.0328572061781000,
-0.0204074581519200,
0.00496575740322000,
0.0410980578417700,
0.0830274624774600,
0.123887229327860,
0.156358732836560,
0.174329857121580,
};
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
void FIR_Init(FIR* handles, float *half_coeffs, unsigned int half_order) {
	//FIR *handles = (FIR*)malloc(sizeof(FIR));
	int k = 1;
	while (k < half_order * 2)
	{
		k <<= 1;
	}
	printf("%d \n", k);
	if (k != half_order * 2)
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
		handles->filter_coeffs_full[half_order * 2 - i - 1] = handles->filter_coeffs_full[i];
	}
}
FIR * FIR_Create(unsigned int half_order,float *half_coeffs ) {
	FIR * handles = (FIR *) malloc(sizeof(FIR ));
	if (handles != NULL)
	{
		FIR_Init(handles, half_coeffs, half_order);
	}
	return handles;
}
int FIR_Process(FIR *handles, float *in, unsigned int input_offest, float *out, unsigned int out_offest, unsigned int nSample, unsigned nStep) {
	// defalut input_offest out_offest is |0|,nStep is |1|
	if (nSample < nStep)
	{
		return -1;
	}
	for (size_t n = 0; n < nSample; n += nStep)
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
		int ll = (handles->bpos + handles->half_order * 2 - 1) & handles->buffer_mask;	// left seek index	% handles->buffer_mask;

		for (size_t kk = 0; kk < handles->half_order; kk++)
		{
			//sample += handles->filter_coeffs[kk] * (handles->buffer[(rr++)&handles->buffer_mask] + handles->buffer[(ll--)&handles->buffer_mask]);
			// 使用 +- kk 代替 ++ -- 避免 ll rr 数据溢出
			sample += handles->filter_coeffs[kk] * (handles->buffer[(rr + kk)&handles->buffer_mask] + handles->buffer[(ll - kk)&handles->buffer_mask]);
		}
		out[out_offest++] = sample;
	}
	return 0;
}

//  for backup 
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

void FIR_test(unsigned int order, float *input, float *output, unsigned int nSample, float *fir_coeffs, float *buffer, int pos) {
	// test passed 
	for (size_t i = 0; i < order; i++)
	{
		buffer[i] = 0;
	}
	unsigned int buffer_size = order - 1;
	for (size_t n = 0; n < nSample; n++)
	{
		buffer[pos] = input[n];	// push data in
		//pos = (pos + 1) & buffer_size;	// order must be a power of 2 if you want to use this method.
		if (++pos >= order)
		{
			pos = 0;
		}
		float sum = 0.0f;
		int p = pos;
		for (int i = order - 1; i >= 0; --i)
		{
			//cout << i << endl;
			sum += fir_coeffs[i] * (buffer[p]);
			//p = (p + 1)&buffer_size;
			if (++p >= order)
			{
				p = 0;
			}
		}
		output[n] = sum;
	}
}
