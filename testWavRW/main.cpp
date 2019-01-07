// testWavRW.cpp: 定义控制台应用程序的入口点。
//
#define DR_WAV_IMPLEMENTATION
#include <stdio.h>
#include <stdint.h>
//#include <assert.h>
#include "wavfile.h"

#include "dsdfile.h"
// ref
//	https://code.google.com/archive/p/dsd2pcm    dsd2pcm

//	stm32f7player (ffmpeg)
//	https://github.com/Kharabadze/DSD4Winamp	
//	https://github.com/amikey/SFBAudioEngine/tree/8e69d2590ea109879cc31b31429a4f57b4f352ef/Decoders
//	https://github.com/clivem/dsdplay/blob/master/src/dsdplay.c

// Samples are downloaded from:
// https://samplerateconverter.com/content/free-samples-dsf-audio-files  dsf demo files.
// https://www.oppodigital.com/hra/dsd-by-davidelias.aspx
// http://www.2l.no/hires/index.html

#define RESAMPLE
#define JAVA_VERSION
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

	//wavwrite_float("v01 - music 352k .wav", float_out_352, nSamplse_per_ch , 1, 44100 * 8 ); // no output

#ifdef RESAMPLE
	// resample 352.8khz to 88.4khz using a FIR filter
	float fir_coeffs_state2[48] = { 
-1.33768885000000e-06,
- 3.48297382000000e-06,
- 5.16205946000000e-06,
- 1.69056875000000e-06,
1.54737069800000e-05,
5.77230132900000e-05,
0.000135366349050000,
0.000250800926040000,
0.000390476659910000,
0.000519186769230000,
0.000581243856170000,
0.000512281970900000,
0.000262213960590000,
- 0.000175303742110000,
- 0.000737031734570000,
- 0.00128134808988000,
- 0.00160838203424000,
- 0.00151339001247000,
- 0.000864952623610000,
0.000314465252590000,
0.00179777272355000,
0.00316403730329000,
0.00388856110053000,
0.00350400707295000,
0.00179320983820000,
- 0.00105068213937000,
- 0.00436803832934000,
- 0.00713285331154000,
- 0.00821685345603000,
- 0.00676581108107000,
- 0.00258088650527000,
0.00363150734425000,
0.0102640847703500,
0.0151288677706200,
0.0160582715754000,
0.0116480229900800,
0.00192623920806000,
- 0.0112844420959000,
- 0.0244872239241600,
- 0.0331648084859600,
- 0.0328572061781000,
- 0.0204074581519200,
0.00496575740322000,
0.0410980578417700,
0.0830274624774600,
0.123887229327860,
0.156358732836560,
0.174329857121580,
};
	//for (size_t n = 0; n < 48; n++)
	//{
	//	// convert java version coeffs to C version's ranking
	//	float coeff_tmp = fir_coeffs_state2[n];
	//	fir_coeffs_state2[n] = fir_coeffs_state2[48 - n - 1];
	//	fir_coeffs_state2[48 - n -1] = coeff_tmp;
	//}

	// State 2. (f8 -> f2)
	FIR *lpf_882 = (FIR*)malloc(sizeof(FIR));
	FIR_Init(lpf_882, fir_coeffs_state2, 48);				// fir filter half coeffs
	float *float_out_884[2]{};
	unsigned int nStep = 4;									// 352->88.2 (f8 -> f2) 4:1

	float_out_884[0] = (float*)malloc(sizeof(float)*nSamplse_per_ch / nStep);
	memset(float_out_884[0], 0, sizeof(float)*nSamplse_per_ch / nStep);
	float_out_884[1] = (float*)malloc(sizeof(float)*nSamplse_per_ch / nStep);
	memset(float_out_884[1], 0, sizeof(float)*nSamplse_per_ch / nStep);

	FIR_Process(lpf_882, float_out_352[0], 0, float_out_884[0], 0, nSamplse_per_ch, nStep);
	//FIR_Process(lpf_882, float_out_352[1], 0, float_out_884[1], 0, nSamplse_per_ch, nStep);

	// test fir
	//float * tmp = (float*)malloc(sizeof(float)*nSamplse_per_ch);
	//FIR_test(48, float_out_352[0], tmp, nSamplse_per_ch, coeffs, state, 0);// test passed

	FIR_Destory(lpf_882);
	cout << "destory\n";

	// output to wav
	wavwrite_float("v01 java - music - 882 - stereo.wav", float_out_884, nSamplse_per_ch/nStep , 1, 44100 * 2);

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