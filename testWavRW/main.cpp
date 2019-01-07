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
int main()
{
	DSD dsdfile;
	//dsd_read(&dsdfile, "sine-176400hz-100hz-15s-D64-2.8mhz.dsf");		// sine signal
	//dsd_read(&dsdfile, "2L-125_stereo-2822k-1b_04.dsf");				// music signal
	//dsd_read(&dsdfile, "sweep-176400hz-0-22050hz-20s-D64-2.8mhz.dsf");	// sweep signal
	dsd_read(&dsdfile, "08 - David Elias - Crossing - Morning Light Western Town (DSD64 2.0).dsf");	// large dsf file for test

	// ========== Decode test ==========//
	// State 1. ( f64 -> f8 ) 8:1
	// general a 352.8khz wav file.
	float *float_out_352[2] = {};											// stereo output
	size_t nSamplse_per_ch = 0;

	dsd_decode(&dsdfile, float_out_352, nSamplse_per_ch);

	//wavwrite_float("v01 - music 352k .wav", float_out_352, nSamplse_per_ch , 1, 44100 * 8 ); // no output

#ifdef RESAMPLE
	// resample 352.8khz to 88.4khz using a FIR filter
	float fir_coeffs[24] = { 
		
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
0.0392396383817332

		/*
-0.000789295417171684,
- 0.000852570705304739,
- 0.000850635076797449,
- 0.000746333304678965,
- 0.000487305945911603,
- 2.33112189471681e-05,
0.000670399783260846,
0.00156915975205662,
0.00258101933095544,
0.00354125216892497,
0.00422377815262290,
0.00437103235870096,
0.00374003157440216,
0.00215855365617283,
- 0.000417783956121909,
- 0.00385848516209242,
- 0.00783195090329421,
- 0.0118086294542546,
- 0.0150980422851308,
- 0.0169188523568738,
- 0.0164953774671216,
- 0.0131687418769496,
- 0.00650715970988813,
0.00360159462092648,
0.0168912457003904,
0.0327034594323406,
0.0500255040231985,
0.0675776609004184,
0.0839414461870813,
0.0977132578280429,
0.107663706449660,
0.112881372921383
*/
};
	// State 2. (f8 -> f2)
	FIR *lpf_882 = (FIR*)malloc(sizeof(FIR));
	FIR_Init(lpf_882, fir_coeffs, 24);				// fir filter half coeffs
	float *float_out_884[2]{};
	unsigned int nStep = 4;							// 352->88.2 (f8 -> f2) 4:1

	float_out_884[0] = (float*)malloc(sizeof(float)*nSamplse_per_ch / nStep);
	memset(float_out_884[0], 0, sizeof(float)*nSamplse_per_ch / nStep);
	float_out_884[1] = (float*)malloc(sizeof(float)*nSamplse_per_ch / nStep);
	memset(float_out_884[1], 0, sizeof(float)*nSamplse_per_ch / nStep);

	FIR_Process(lpf_882, float_out_352[0], 0, float_out_884[0], 0, nSamplse_per_ch, nStep);
	FIR_Process(lpf_882, float_out_352[1], 0, float_out_884[1], 0, nSamplse_per_ch, nStep);

	// test fir
	//float * tmp = (float*)malloc(sizeof(float)*nSamplse_per_ch);
	//FIR_test(48, float_out_352[0], tmp, nSamplse_per_ch, coeffs, state, 0);// test passed

	FIR_Destory(lpf_882);
	cout << "destory\n";

	// output to wav
	wavwrite_float("v01 java - music - large 882 - stereo.wav", float_out_884, nSamplse_per_ch/nStep , 2, 44100 * 2);

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