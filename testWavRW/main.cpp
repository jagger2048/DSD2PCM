// testWavRW.cpp: 定义控制台应用程序的入口点。
//
#define DR_WAV_IMPLEMENTATION

#define RESAMPLE
#define JAVA_VERSION

#include <stdio.h>
#include <stdint.h>
//#include <assert.h>
#include "wavfile.h"

#include "dsdfile.h"
#include "state2.h"
#include "fir_halfband.h"
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
	//dsd_read(&dsdfile, "sine-176400hz-100hz-15s-D64-2.8mhz.dsf");		// sine signal
	dsd_read(&dsdfile, "2L-125_stereo-2822k-1b_04.dsf");				// music signal
	//dsd_read(&dsdfile, "sweep-176400hz-0-22050hz-20s-D64-2.8mhz.dsf");	// sweep signal
	//dsd_read(&dsdfile, "08 - David Elias - Crossing - Morning Light Western Town (DSD64 2.0).dsf");	// large dsf file for test

	// ========== Decode test ==========//
	// State 1. ( f64 -> f8 ) 8:1
	// general a 352.8khz wav file.
	float *float_out_352[2] = {};											// stereo output
	size_t nSamplse_per_ch = 0;

	dsd_decode(&dsdfile, float_out_352, nSamplse_per_ch);

	//wavwrite_float("v01 - music 352k - demo .wav", float_out_352, nSamplse_per_ch , 1, 44100 * 8 ); // no output

#ifdef RESAMPLE
	// resample 352.8khz to 88.4khz using a FIR filter
	
	// State 2. (f8 -> f2)
	//FIR *lpf_882 = (FIR*)malloc(sizeof(FIR));
	//FIR_Init(lpf_882, fir_coeffs_state2, 48);				// fir filter half coeffs

	//FIR *lpf_882 = FIR_Create(48,fir_coeffs_state2);
	FIR_Halfband *halfband_352 = fir_halfband_create(48, fir_halfband_coeffs_352);
	FIR_Halfband *halfband_176 = fir_halfband_create(24, fir_halfband_coeffs_176);

	float *float_out_884[2]{};
	unsigned int nStep = 2;									// 352->88.2 (f8 -> f2) 4:1

	float_out_884[0] = (float*)malloc(sizeof(float)*nSamplse_per_ch / nStep);
	memset(float_out_884[0], 0, sizeof(float)*nSamplse_per_ch / nStep);

	float_out_884[1] = (float*)malloc(sizeof(float)*nSamplse_per_ch / nStep);
	memset(float_out_884[1], 0, sizeof(float)*nSamplse_per_ch / nStep);

	// 对比测试
	//FIR_Process(lpf_882, float_out_352[0], 0, float_out_884[0], 0, nSamplse_per_ch, nStep);

	cout << "start to process:\n";

	fir_halfband_process(halfband_352, float_out_352[0], float_out_884[0], nSamplse_per_ch , nStep);


	//fir_halfband_process(halfband_176, float_out_884[0], float_out_884[1], nSamplse_per_ch / nStep, nStep);

	// Destory
	fir_halfband_destory(halfband_352);
	fir_halfband_destory(halfband_176);
	//FIR_Destory(lpf_882);

	cout << "destory\n";

	// output to wav
	wavwrite_float("v004 - halfband - music - full.wav", &float_out_884[0], nSamplse_per_ch /2 , 1, 44100 * 4 );

	// Free resources
	free(float_out_884[0]);
	free(float_out_884[1]);

#endif // RESAMPLE

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