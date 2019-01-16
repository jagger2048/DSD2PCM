#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
// the dsd2pcm state2 processing

struct FIR
{
	int buffer_size;
	int buffer_mask;
	int bpos;
	unsigned int half_order;	// the half taps of FIR filter, e.g. for a 47 order FIR, it has 48 coefficients, then |half_order| is 24.
	float *buffer;
	float *filter_coeffs;		// half FIR coefficients
};
void FIR_Init(FIR* handles, float *half_coeffs, unsigned int half_order) {
	int k = 1;
	while (k < half_order * 2)
	{
		k <<= 1;
	}

	handles->buffer_size = k;
	handles->buffer_mask = k - 1;
	handles->bpos = 0;
	handles->buffer = (float *)malloc(sizeof(float) * handles->buffer_size);
	handles->filter_coeffs = (float *)malloc(sizeof(float) * half_order);
	handles->half_order = half_order;
	memset(handles->buffer, 0, sizeof(float) * handles->buffer_size);
	memcpy(handles->filter_coeffs, half_coeffs, sizeof(float) * half_order);

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

		float sample = 0;
		int rr = handles->bpos;															// right seek index
		int ll = (handles->bpos + handles->half_order * 2 - 1) & handles->buffer_mask;	// left seek index	% handles->buffer_mask;

		for (size_t kk = 0; kk < handles->half_order; kk++)
		{
			//sample += handles->filter_coeffs[kk] * (handles->buffer[(rr++)&handles->buffer_mask] + handles->buffer[(ll--)&handles->buffer_mask]);
			// 使用 + / - kk 代替 ++ -- 避免 ll rr 数据溢出
			sample += handles->filter_coeffs[kk] * (handles->buffer[(rr + kk)&handles->buffer_mask] + handles->buffer[(ll - kk)&handles->buffer_mask]);
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
		free(handles);
	}
	return 0;
}


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