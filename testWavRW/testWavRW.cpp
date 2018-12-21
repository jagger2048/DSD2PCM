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

// https://samplerateconverter.com/content/free-samples-dsf-audio-files  dsf demo files.

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
	fopen_s(&fp, "sine-176400hz-100hz-15s-D64-2.8mhz.dsf","rb");
	assert(fp != NULL);

	DSD dsdfile;
	cout << sizeof(dsdfile) << endl;
	fpos_t fpos;
	fread( &dsdfile.dsd_chunk_header, 84, 1, fp );
	fgetpos(fp, &fpos);

	fread(&dsdfile.data_size, sizeof(dsdfile.data_size), 1, fp);


	uint8_t* pSampleData = 0;
	//fsetpos()
	unsigned int nSamples = dsdfile.data_size - 12;

	if (nSamples != dsdfile.sample_count / 8 * dsdfile.channel_num)
	{
		printf("Samples not match\n");
		return -1;
	}

	cout << "Has total " << nSamples << " bytes to be read.\n";

	pSampleData = new uint8_t[nSamples]; // chn = 2

	fread(pSampleData, nSamples, 1, fp);

	// general a 352.8khz 8 bit-per-sample wav file.

	// 生成 88.2khz  ,352.8 176, 88.2
	uint16_t  nStep = 32 / 8;	// f64->f2 how man bytes to skip after each sample calc,相当于抽取

	uint8_t *ch1 = new uint8_t[ nSamples / 2]{0};
	uint8_t *ch2 = new uint8_t[ nSamples / 2]{0};

	cout << dsdfile.channel_num << endl;
	cout << dsdfile.block_per_channel << endl;

	for (size_t i = 0; i < nSamples / 2 / 4096; i++)	// chn 2, 4096 is block size
	{
			ch1[i] = pSampleData[ i * 2 * 4096  ];
			ch2[i] = pSampleData[ i * 2  * 4096 + 1];

	}

	unsigned samples_per_ch = nSamples / 2 / nStep;	// 2 is channel
	uint8_t *pOut_882_u8 = new uint8_t[samples_per_ch ];

	// 抽取 
	for (size_t n = 0; n < samples_per_ch; n++)
	{
		pOut_882_u8[n] = ch1[ n * nStep];
	}
	int16_t *pOut_882 = new int16_t[samples_per_ch ];

	drwav_u8_to_s16(pOut_882, pOut_882_u8, samples_per_ch );


	for (size_t i = 100; i > 0; --i)
	{
		cout << pOut_882[samples_per_ch - i] << endl;
	}


	//drwav_data_format format;
	//format.container = drwav_container_riff;     // <-- drwav_container_riff = normal WAV files, drwav_container_w64 = Sony Wave64.
	//format.format = DR_WAVE_FORMAT_PCM;          // <-- Any of the DR_WAVE_FORMAT_* codes.
	//format.channels = 1;
	//format.sampleRate = 44100 * 2;
	//format.bitsPerSample = 16;

	//drwav* pWav = drwav_open_file_write(" test DSD mono 882 - v2 .wav", &format);
	////drwav_uint64 samplesWritten = drwav_write_raw(pWav, nSamples /2 , ch1);	// convert
	//drwav_uint64 samplesWritten = drwav_write_raw(pWav, samples_per_ch / 4, pOut_882);	// convert
	//cout << "Written " << samplesWritten << endl;

	wavwrite_s16("test DSD mono 882 - v2 .wav", &pOut_882, samples_per_ch , 1, 44100*2);


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