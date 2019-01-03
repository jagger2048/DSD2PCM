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
	dsd_read(&dsdfile, "2L-125_stereo-2822k-1b_04.dsf");				// music signal
	//dsd_read(&dsdfile, "sweep-176400hz-0-22050hz-20s-D64-2.8mhz.dsf");	// sweep signal

	// ========== Decode test ==========//
	// general a 352.8khz wav file.( f64 -> f8 )
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
0.0392396383817332 };

	FIR *lpf_882 = (FIR*)malloc(sizeof(FIR));
	FIR_Init(lpf_882, fir_coeffs, 24);	// fir filter half coeffs

	float *float_out_884[2]{};
	unsigned int nStep = 4;				// 352->88.2

	float_out_884[0] = (float*)malloc(sizeof(float)*nSamplse_per_ch / nStep);
	memset(float_out_884[0], 0, sizeof(float)*nSamplse_per_ch / nStep);
	float_out_884[1] = (float*)malloc(sizeof(float)*nSamplse_per_ch / nStep);

	float * tmp = (float*)malloc(sizeof(float)*nSamplse_per_ch );
	//FIR_Process(lpf_882, float_out_352[0], 0, float_out_884[0], 0, nSamplse_per_ch, nStep);



	FIR_Process(lpf_882, float_out_352[0], 0, tmp, 0, nSamplse_per_ch, 1 );
	cout << "Fir\n";
	size_t count = 0,jj=0;
	for (size_t i = 0; i < nSamplse_per_ch ; i++)
	{
		if (++count == nStep)
		{
			float_out_884[0][jj++] = tmp[i];		// 抽取
			count = 0;
		}
		//float_out_884[0][i] = tmp[i * 4];		// 抽取
	}
	FIR_Destory(lpf_882);
	cout << "destory\n";
	// other version fir filter
	//unsigned int pos = 0;
	//float outTmp = 0;
	//unsigned int count = 0;
	//unsigned int s = 0;
	//for (size_t n = 0; n < nSamplse_per_ch; n++)
	//{
	//	// 48 taps fir lpf filter,cut-off frequency = 30khz
	//	outTmp = fir_smpl_circle_f32(48, float_out_352[1][n], coeffs, state, &pos);
	//	if (++count == nStep)
	//	{
	//		float_out_884[1][n / nStep] = outTmp;		// 抽取
	//		count = 0;
	//	}
	//	//float_out_884[1][n / nStep] = outTmp;		// 抽取

	//}
	// output to wav
	//wavwrite_float("v10 java - music 882 .wav", &tmp, nSamplse_per_ch/nStep , 1, 44100 * 2);
	wavwrite_float("v11 java - music 882 .wav", &float_out_884[0], nSamplse_per_ch/nStep , 1, 44100 * 2);
	//wavwrite_float("v1 java - music 882 .wav", &tmp, nSamplse_per_ch , 1, 44100 * 2);
	//wavwrite_float("v2  mc  - music 882 .wav", &float_out_884[1], nSamplse_per_ch / nStep, 1, 44100 * 2);

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