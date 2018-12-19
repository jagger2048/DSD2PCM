// testWavRW.cpp: 定义控制台应用程序的入口点。
//
#include <iostream>
using namespace std;
#define DR_WAV_IMPLEMENTATION
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "wavfile.h"

int main()
{
	// test case 
	wav mywav;
	wavread("audioCut_2.wav", &mywav);


	wavwrite_s16("test my writter s16 ch1.wav", mywav.pDataS16, mywav.totalPCMFrameCount, 1, mywav.sampleRate);
	wavwrite_s16("test my writter s16 ch2.wav", mywav.pDataS16, mywav.totalPCMFrameCount, 2, mywav.sampleRate);

	wavwrite_float("test my writter f16 ch1.wav", mywav.pDataFloat, mywav.totalPCMFrameCount, 1, mywav.sampleRate);
	wavwrite_float("test my writter f16 ch2.wav", mywav.pDataFloat, mywav.totalPCMFrameCount, 2, mywav.sampleRate);
    
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