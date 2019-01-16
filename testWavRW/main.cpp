// testWavRW.cpp: 定义控制台应用程序的入口点。
//
#define DR_WAV_IMPLEMENTATION

#define JAVA_VERSION

#include <stdio.h>
#include <stdint.h>
#include "wavfile.h"

#include "dsdfile.h"
#include "state2.h"
#include "fir_halfband.h"
#include <time.h>
// ref
//	https://code.google.com/archive/p/dsd2pcm    dsd2pcm C version
// dsd2pcm java version

//	stm32f7player (ffmpeg)
//	https://github.com/Kharabadze/DSD4Winamp	
//	https://github.com/amikey/SFBAudioEngine/tree/8e69d2590ea109879cc31b31429a4f57b4f352ef/Decoders
//	https://github.com/clivem/dsdplay/blob/master/src/dsdplay.c

// Samples are downloaded from:
// https://samplerateconverter.com/content/free-samples-dsf-audio-files  dsf demo files.
// https://www.oppodigital.com/hra/dsd-by-davidelias.aspx
// http://www.2l.no/hires/index.html


;;;
int main()
{
	DSD dsdfile;
	dsd_read(&dsdfile, "2L-125_stereo-2822k-1b_04.dsf");				// music signal
	//dsd_read(&dsdfile, "sweep-176400hz-0-22050hz-20s-D64-2.8mhz.dsf");	// sweep signal

	// ========== Decode test ==========//
	// State 1. ( f64 -> f8 ) 8:1
	// general 352.8khz signal
	float *float_out_352[2] = {};											// stereo output
	size_t nSamplse_per_ch = 0;

	clock_t t1 = clock();
	dsd_decode(&dsdfile, float_out_352, nSamplse_per_ch);
	clock_t t2 = clock();
	printf(" State 1 cost %d ms (%d opints output)\n", t2 - t1, nSamplse_per_ch);
	//wavwrite_float("v001 - sweep - to processed.wav", float_out_352, nSamplse_per_ch , 1, 44100 * 8 ); // not output
	
	// State 2. (f8 -> f2) 4:1
	// resample 352.8khz to 88.4khz using a FIR filter

	FIR *lpf_882 = FIR_Create( 48, fir_coeffs_state2);

	FIR_Halfband *halfband_352 = fir_halfband_create(48, fir_halfband_coeffs_352);
	FIR_Halfband *halfband_176 = fir_halfband_create(24, fir_halfband_coeffs_176);

	float *float_out_884[2]{};
	unsigned int nStep = 2;									// 352->88.2 (f8 -> f2) 4:1

	float_out_884[0] = (float*)malloc(sizeof(float)*nSamplse_per_ch / nStep);
	memset(float_out_884[0], 0, sizeof(float)*nSamplse_per_ch / nStep);
	float_out_884[1] = (float*)malloc(sizeof(float)*nSamplse_per_ch / nStep);
	memset(float_out_884[1], 0, sizeof(float)*nSamplse_per_ch / nStep);

	// 对比测试
	printf( "start to process:\n");
	clock_t ts = clock();
	FIR_Process(lpf_882, float_out_352[0], 0, float_out_884[0], 0, nSamplse_per_ch, 4);
	clock_t te = clock();

	//cout <<  nSamplse_per_ch <<" points in,"<< nSamplse_per_ch  /4 <<" points out; "<< " costs " <<( te - ts) << " ms  - state2 \n";
	printf(" %d pints in, %d points out; costs %d ms ( state2 FIR )\n", nSamplse_per_ch, nSamplse_per_ch / 4, te - ts);
	// output to wav
	wavwrite_float("v001 - music v1.wav", &float_out_884[0], nSamplse_per_ch / 4, 1, 44100 * 2);

	//============//
	ts = clock();
	fir_halfband_process(halfband_352, float_out_352[0], float_out_884[0], nSamplse_per_ch, nStep);		// section one
	fir_halfband_process(halfband_176, float_out_884[0], float_out_884[1], nSamplse_per_ch / 2, nStep);	// section two
	te = clock();

	printf(" %d pints in, %d points out; costs %d ms ( state2 halfband FIR )\n", nSamplse_per_ch, nSamplse_per_ch / 4, te - ts);
	//cout << nSamplse_per_ch << " points in," << nSamplse_per_ch / 4 << " points out; " << " costs " << (te - ts) << " ms\n";


	// Destory
	fir_halfband_destory(halfband_352);
	fir_halfband_destory(halfband_176);
	FIR_Destory(lpf_882);

	// output to wav
	wavwrite_float("v001 - music v2.wav", &float_out_884[1], nSamplse_per_ch /4 , 1, 44100 * 2 );

	// Free resources
	free(float_out_884[0]);
	free(float_out_884[1]);
	free(float_out_352[0]);

	return 0;
}




/*
Usage:
About the DSD read method:
	dsd2pcm.c
	dsd2pcm.h
	dsdfile.h

1.	read dsd file:

		DSD dsdfile;				// initialize the dsd structure
		dsd_read(&dsdfile, "dsd filename.dsd");	//use the | int dsd_read(DSD *dsdfile, const char* file_name) |  function

2.	decode dsd data,note that the library decode the dsd data to 352khz float data.

		float *float_out_352[2] = {};	// initialize the output pointer
		dsd_decode(&dsdfile, float_out_352, nSamplse_per_ch);	// decode the dsd data

3. optional:
	Resample to other samplerate as showed above.

About the WAV read /write method:
	dr_wav.h
	wavfile.h

1.	Wav file read:
		WAV wavfile;				// initialize the wav structure
		wavread("your wav file.wav",&wavfile); // use the wavread function | wavread(const char* filename, wav* wavfile) |
		// the decoded PCM data(int16_t / float) can be assessed through the wav structure
		// wavfile.pDataS16[channel_num][sample_num]
		// wavfile.pDataFloat[channel_num][sample_num]
2. Wav file write:
	we provide two functions convert int16_t / float pcm data into 16-bits wav file.

	int wavwrite_s16(const char* filename, int16_t * const *pDataS16, size_t nSamples, unsigned int nChannels, unsigned int sampleRate) 
	int wavwrite_float(const char* filename, float * const *pDataFloat, size_t nSamples, unsigned int nChannels, unsigned int sampleRate) 

*/