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

	wavwrite_float("v01 - music 352k .wav", float_out_352, nSamplse_per_ch , 1, 44100 * 8 );

#ifdef RESAMPLE
	// resample 352.8khz to 88.4khz using a FIR filter
	float fir_coeffs[24] = { 
0.112652250850214,
0.107032289815474,
0.0963892125541776,
0.0818323747814125,
0.0648289218586615,
0.0470040360336655,
0.0299283330453901,
0.0149256643933968,
0.00292983911011809,
- 0.00559075617177340,
- 0.0106340875176825,
- 0.0125973353140726,
- 0.0121591060685235,
- 0.0101356123884851,
- 0.00733767381809999,
- 0.00445254411515090,
- 0.00196751481952750,
- 0.000142627935256290,
0.000970290707851763,
0.00147365784890315,
0.00155206306576435,
0.00140042203044790,
0.00116817492668589,
0.000929727126408359 };
	FIR *lpf_882 = (FIR*)malloc(sizeof(FIR));
	FIR_Init(lpf_882, fir_coeffs, 24);

	float *float_out_884[2]{};
	unsigned int nStep = 4;	// 352->88.4
	float_out_884[0] = (float*)malloc(sizeof(float)*nSamplse_per_ch / nStep);
	float_out_884[1] = (float*)malloc(sizeof(float)*nSamplse_per_ch / nStep);

	FIR_Process(lpf_882, float_out_352[0], 0, float_out_884[0], 0, nSamplse_per_ch, nStep);

	//for (size_t i = 0; i < 100; i++)
	//{
	//	printf("%f \n", float_out_884[0][i]);
	//}
	//FIR_Destory(lpf_882);



	//unsigned int pos = 0;
	//float outTmp = 0;
	//unsigned int count = 0;
	//unsigned int s = 0;
	//for (size_t n = 0; n < nSamplse_per_ch; n++)
	//{
	//	// 48 taps fir lpf filter,cut-off frequency = 30khz
	//	outTmp = fir_smpl_circle_f32(48, float_out_352[0][n], coeffs, state, &pos);
	//	if (++count == nStep)
	//	{
	//		float_out_884[0][n / nStep] = outTmp;		// 抽取
	//		count = 0;
	//	}
	//}
	// output to wav
	//wavwrite_float("v1 - sweep 882 .wav", float_out_884, nSamplse_per_ch / nStep, 1, 44100*2);
	wavwrite_float("v6 - music 882 .wav", float_out_884, nSamplse_per_ch / nStep, 1, 44100 * 2);

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