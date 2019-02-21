// 
// Using halfband filter to replace the standard FIR filter in state2.
/*

#include <time.h>
unsigned int nStep = 2;													// 352->88.2 (f8 -> f2) 4:1

FIR_Halfband *halfband_352 = fir_halfband_create(48, fir_halfband_coeffs_352);
FIR_Halfband *halfband_176 = fir_halfband_create(24, fir_halfband_coeffs_176);



float_out_884[0] = (float*)malloc(sizeof(float)*nSamplse_per_ch / nStep);
memset(float_out_884[0], 0, sizeof(float)*nSamplse_per_ch / nStep);
float_out_884[1] = (float*)malloc(sizeof(float)*nSamplse_per_ch / nStep);
memset(float_out_884[1], 0, sizeof(float)*nSamplse_per_ch / nStep);


// halfband version
fir_halfband_process(halfband_352, float_out_352[0], float_out_884[0], nSamplse_per_ch, nStep);		// section one
fir_halfband_process(halfband_176, float_out_884[0], float_out_884[1], nSamplse_per_ch / 2, nStep);	// section two

																									//printf(" %d pints in, %d points out; costs %d ms ( state2 halfband FIR )\n", nSamplse_per_ch, nSamplse_per_ch / 4, te - ts);
																									//cout << nSamplse_per_ch << " points in," << nSamplse_per_ch / 4 << " points out; " << " costs " << (te - ts) << " ms\n";


																									// Destory
fir_halfband_destory(halfband_352);
fir_halfband_destory(halfband_176);

*/