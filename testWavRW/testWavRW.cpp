// testWavRW.cpp: 定义控制台应用程序的入口点。
//
#include <iostream>
using namespace std;
#define DR_WAV_IMPLEMENTATION
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "wavfile.h"

// ref
//https://github.com/Kharabadze/DSD4Winamp	
//https://github.com/amikey/SFBAudioEngine/tree/8e69d2590ea109879cc31b31429a4f57b4f352ef/Decoders
///////////////////////////
// Samples are downloaded from:
// http://www.ayre.com/insights_dsdvspcm.htm
// (link from http://pcaudiophile.ru/index.php?id=31 )
///////////////////////////
// more sample:
// https://www.oppodigital.com/hra/dsd-by-davidelias.aspx
///////////////////////////

struct DSD
{	
	// uint64_t  a 8 bytes,64 bits unsigned int, 
	// uint32_t  a 4 bytes,32 bits unsigned int, 

	char dsd_chunk_header[4];		// DSD mask
	char chunk_size[8];				// size of this chunk [28]
	//uint64_t chunk_size;			// size of this chunk [28] test 
	uint64_t total_size;			// total file size
	char pMetadataChunk[4];			// the pointer point to metadata if exist,otherwise set 0.
									// 8?
	char fmt_chunk_header[4];		// fmt chunk header
	uint64_t fmt_size;				// size of this fmt chunk,usually [52] bytes
	uint32_t fmt_version;			// gersion of this format [1]
	uint32_t fmt_id;				// [0]: DSD raw
	uint32_t channel_type;			// [1]:mono [1]:stereo [3]: 3 channels [4] quad [5] 4chs [6] 5chs [7] 5.1chs
	uint32_t channel_num;			// [1]-[6] : mono~6 chs
	uint32_t sample_rate;			//  [2822400] or [5644800] hz  64fs or 128fs
	uint32_t bits_per_sample;		// [1] or [8]
	uint64_t sample_count;			// samples per channels, n second data: sample count would be fs * n
	uint32_t block_per_channel;		// fixed [4096] Bytes
	uint32_t reverd;				// fill zero

	char	data_chunk_header[4];	// [ d a t a]
	uint64_t data_size;			// equal to [n] + 12 , n is the next [n] bytes contains data 
	// next n bytes is the data
	//char* pSampleData = 0;
	// m bytes meta data chunk if have
	
};
int main()
{
	FILE *fp = NULL;
	fopen_s(&fp, "08 - David Elias - Crossing - Morning Light Western Town (DSD64 2.0).dsf","rb");
	assert(fp != NULL);

	DSD dsdfile;
	cout << sizeof(dsdfile) << endl;
	fpos_t fpos;
	fgetpos(fp, &fpos);
	cout << fpos << endl;
	fread( &dsdfile.dsd_chunk_header, 92, 1, fp );
	fgetpos(fp, &fpos);
	cout << fpos << endl;
	char* pSampleData = 0;
	//fsetpos()
	pSampleData = new char[dsdfile.data_size];
	//fread(pSampleData, dsdfile.data_size, 1, fp);

	char pp[4096] = {};
	fread(pp, 4096, 1, fp);

	for (size_t i = 0; i < 100; i++)
	{
		cout<<pp[i] << endl;
	}
	
	fclose(fp);

	// wavfile write read test case 
	//wav mywav;
	//wavread("audioCut_2.wav", &mywav);


	//wavwrite_s16("test my writter s16 ch1.wav", mywav.pDataS16, mywav.totalPCMFrameCount, 1, mywav.sampleRate);
	//wavwrite_s16("test my writter s16 ch2.wav", mywav.pDataS16, mywav.totalPCMFrameCount, 2, mywav.sampleRate);

	//wavwrite_float("test my writter f16 ch1.wav", mywav.pDataFloat, mywav.totalPCMFrameCount, 1, mywav.sampleRate);
	//wavwrite_float("test my writter f16 ch2.wav", mywav.pDataFloat, mywav.totalPCMFrameCount, 2, mywav.sampleRate);
 //   
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