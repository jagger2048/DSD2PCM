// testWavRW.cpp: 定义控制台应用程序的入口点。
//
#include <iostream>
using namespace std;
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>

struct wav
{
	unsigned int channels;
	unsigned int sampleRate;
	drwav_uint64 totalPCMFrameCount;	// nSamples per cahnnels
	drwav_uint64 totalSampleCount;		// left samples + ritht samples
	int16_t *pDataS16[2];
	float *pDataFloat[2];
};
int wavread(const char* filename, wav* wavfile ) {

	float *pSampleFloat = drwav_open_file_and_read_pcm_frames_f32(filename, &wavfile->channels,&wavfile->sampleRate, &wavfile->totalPCMFrameCount);
	int16_t *pSampleS16 = drwav_open_file_and_read_pcm_frames_s16(filename, &wavfile->channels, &wavfile->sampleRate, &wavfile->totalPCMFrameCount);
	// The pointer of audio data which is [LRLRLRLR ...] format

	wavfile->totalSampleCount = wavfile->totalPCMFrameCount * wavfile->channels;

	if (pSampleFloat == NULL || pSampleS16 == NULL)
	{
		return -1;
	}
	//	change audio data into [[LLLLLL][RRRRRR]]
	if (wavfile->channels >1)
	{
		wavfile->pDataS16[0] = (int16_t*)malloc(sizeof(int16_t) * wavfile->totalSampleCount);
		wavfile->pDataFloat[0] = (float*)malloc(sizeof(float) * wavfile->totalSampleCount);

		wavfile->pDataS16[1] = wavfile->pDataS16[0] + wavfile->totalPCMFrameCount;	// totalPCMFrameCount = totalSampleCount / 2
		wavfile->pDataFloat[1] = wavfile->pDataFloat[0] + wavfile->totalPCMFrameCount;

		for (size_t n = 0; n < wavfile->totalPCMFrameCount; n++)
		{
			wavfile->pDataS16[0][n] = pSampleS16[n * 2];
			wavfile->pDataS16[1][n] = pSampleS16[n * 2 + 1];

			wavfile->pDataFloat[0][n] = pSampleFloat[n * 2];
			wavfile->pDataFloat[1][n] = pSampleFloat[n * 2 + 1];

		}
		free(pSampleS16);
		free(pSampleFloat);
	}
	else
	{
	wavfile->pDataS16[0] = pSampleS16;
	wavfile->pDataS16[1] = NULL;
	wavfile->pDataFloat[0] = pSampleFloat;
	wavfile->pDataFloat[1] = NULL;

	}
	return 0;
}
int wavwrite_s16(const char* filename, int16_t * const *pDataS16,size_t nSamples,unsigned int nChannels,unsigned int sampleRate) {
	
	drwav_data_format format;
	format.container = drwav_container_riff;     // <-- drwav_container_riff = normal WAV files, drwav_container_w64 = Sony Wave64.
	format.format = DR_WAVE_FORMAT_PCM;          // <-- Any of the DR_WAVE_FORMAT_* codes.
	format.channels = nChannels;
	format.sampleRate = sampleRate;
	format.bitsPerSample = 16;
	if (nChannels > 1 && pDataS16[1] == NULL)
	{
		printf(" Channel 2 data not found\n");
		return -1;
	}
	drwav* pWav = drwav_open_file_write(filename, &format);
	if (pWav == NULL)
	{
		return -1;
	}
	if (nChannels > 1 )
	{
		int16_t *data = (int16_t *) malloc( sizeof(int16_t) * nSamples * 2 ); // nSamples is number per channels 

		for (size_t n = 0; n < nSamples ; n++)
		{
			data[n * 2] = pDataS16[0][n];		// even part left channel
			data[n * 2 + 1] = pDataS16[1][n];	//  odd part, ritht channel
		}
		//drwav_uint64 samplesWritten = drwav_write_pcm_frames(pWav, nSamples, data);
		drwav_uint64 samplesWritten = drwav_write_raw(pWav, nSamples * nChannels * 16/8, data);
		if (samplesWritten != nSamples * nChannels * 16 / 8)
		{
			printf("written 2ch failed\n");
		}
		free(data);
	}
	else
	{
		//drwav_uint64 samplesWritten = drwav_write_pcm_frames(pWav, nSamples, pDataS16[0]);	// convert
		drwav_uint64 samplesWritten = drwav_write_raw(pWav, nSamples * 2 , pDataS16[0]);	// convert
		if (samplesWritten != nSamples * 2)
		{
			printf("written 1ch failed\n");
			//return -1;
		}
	}

	drwav_close(pWav);
	return 0;

}
int wavwrite_float(const char* filename, float * const *pDataFloat, size_t nSamples, unsigned int nChannels, unsigned int sampleRate) {

	if (pDataFloat == NULL )
	{
		printf("Input data pointer failed.\n");
		return -1;
	}
	if (pDataFloat[1] == NULL && nChannels >1)
	{
		printf(" Channel 2 data not found\n");
		return -1;
	}
	int16_t * tmp = (int16_t *)malloc(sizeof(int16_t) * nSamples * nChannels);


	int16_t *pDataS16[2];
	pDataS16[0] = tmp;
	//FloatToS16(pDataFloat[0], nSamples, pDataS16[0]);	//drwav_f32_to_s16
	drwav_f32_to_s16(pDataS16[0], pDataFloat[0], nSamples);

	if (nChannels > 1 )
	{
		pDataS16[1] = tmp + nSamples;
		//FloatToS16(pDataFloat[1], nSamples, pDataS16[1]);
		drwav_f32_to_s16(pDataS16[1], pDataFloat[1], nSamples);

	}
	else
	{
		pDataS16[1] = NULL;
	}


	int error = wavwrite_s16(filename, pDataS16, nSamples, nChannels, sampleRate);

	free(tmp);

	if (error != 0)
	{
		printf("Output float wav failed\n");
		return -1;
	}
	return 0;
}
int main()
{
	// test case 
	wav mywav;
	wavread("08 - David Elias - Crossing - Morning Light Western Town (DSD64 2.0).wav", &mywav);


	wavwrite_s16("test my writter s16 ch1.wav", mywav.pDataS16, mywav.totalPCMFrameCount, 1, mywav.sampleRate);
	wavwrite_s16("test my writter s16 ch2.wav", mywav.pDataS16, mywav.totalPCMFrameCount, 2, mywav.sampleRate);

	wavwrite_float("test my writter f16 ch1.wav", mywav.pDataFloat, mywav.totalPCMFrameCount, 1, mywav.sampleRate);
	wavwrite_float("test my writter f16 ch2.wav", mywav.pDataFloat, mywav.totalPCMFrameCount, 2, mywav.sampleRate);
    
	return 0;
}


static inline float S16ToFloat_(int16_t v) {
	static const float kMaxInt16Inverse = 1.f / 32767;
	static const float kMinInt16Inverse = 1.f / -32768;
	return v * (v > 0 ? kMaxInt16Inverse : -kMinInt16Inverse);
}
int S16ToFloat(int16_t *pS16Samples, size_t nSamples, float *pFloatSamples) {
	if (pFloatSamples == NULL || nSamples < 0 || pS16Samples == NULL)
	{
		return -1;
	}
	for (size_t n = 0; n < nSamples; n++)
	{
		pFloatSamples[n] = S16ToFloat_(pS16Samples[n]);
	}
	return 0;
}

static inline int16_t FloatToS16_(float v) {
	//S16:      int16_t[-32768, 32767]
	if (v > 0)
		return v >= 1 ? 32767
		: (int16_t)(v * 32767 + 0.5f);
	return v <= -1 ? -32768
		: (int16_t)(-v * -32768 - 0.5f);
}
int FloatToS16(float * pFloatSamples, size_t nSamples, int16_t *pS16Samples) {

	if (pFloatSamples == NULL || nSamples < 0 || pS16Samples == NULL)
	{
		return -1;
	}
	for (size_t n = 0; n < nSamples; n++)
	{
		pS16Samples[n] = FloatToS16_(pFloatSamples[n]);
	}
	return 0;
}



//FILE *fp = NULL;
//fopen_s(&fp, "dukou_noReverb.wav","rb");
//assert(fp != NULL);
//Header header;

//fread(&header, sizeof(header), 1, fp);
//fseek(fp, sizeof(header), 0);

//short data[10];
//fread(data, 10 * sizeof(short), 1,fp);

//for (size_t i = 0; i < 10; i++)
//{
//	cout << data[i] << endl;
//}
//cout << header.sample_rate;

//fclose(fp);
//typedef struct Header {
//struct Header {
//	/* WAV-header parameters				Memory address - Occupied space - Describes */
//	char chunk_ID[4];                           // 0x00 4 byte - RIFF string
//	uint32_t  chunk_size;                  // 0x04 4 byte - overall size of
//	char format[4];                             // 0x08 4 byte - WAVE string
//	char fmt_chunk_marker[4];                   // 0x0c 4 byte - fmt string with trailing null char
//	uint32_t length_of_fmt;					  // 0x10 4 byte - length of the format data,the next part
//	uint16_t  format_type;					// 0x14 2 byte - format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
//	uint16_t  channels;						 // 0x16 2 byte - nunbers of channels
//	uint32_t  sample_rate;					// 0x18 4 byte - sampling rate (blocks per second)
//	uint32_t  byte_rate;                   // 0x1c 4 byte - SampleRate * NumChannels * BitsPerSample/8 [比特率]
//	uint16_t  block_align;                 // 0x20 2 byte - NumChannels * BitsPerSample/8 [块对齐=通道数*每次采样得到的样本位数/8]
//	uint16_t  bits_per_sample;				 // 0x22 2 byte - bits per sample, 8- 8bits, 16- 16 bits etc [位宽]
//	char data_chunk_header[4];				// 0x24 4 byte - DATA string or FLLR string
//	uint32_t  data_size;					 // 0x28 4 byte - NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk 
//											 //				that will be read,that is the size of PCM data.
//};