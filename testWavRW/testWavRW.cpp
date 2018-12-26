// testWavRW.cpp: 定义控制台应用程序的入口点。
//
#include <iostream>
using namespace std;
#define DR_WAV_IMPLEMENTATION



#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "wavfile.h"
#include "dsd2pcm.h"
#include <iostream>
//#include <vector>
//#include <string>	//c

#include "noiseshape.hpp"

#include "dsd2pcm.hpp"

#include "tmp.h"
// ref
//https://github.com/Kharabadze/DSD4Winamp	
//https://github.com/amikey/SFBAudioEngine/tree/8e69d2590ea109879cc31b31429a4f57b4f352ef/Decoders
// dsd2pcm
// stm32f7player (ffmpeg)
///////////////////////////
// Samples are downloaded from:
// http://www.ayre.com/insights_dsdvspcm.htm
// (link from http://pcaudiophile.ru/index.php?id=31 )
///////////////////////////
// more sample:
// https://www.oppodigital.com/hra/dsd-by-davidelias.aspx
///////////////////////////

// https://samplerateconverter.com/content/free-samples-dsf-audio-files  dsf demo files.

// ref
// https://github.com/clivem/dsdplay/blob/master/src/dsdplay.c
int dsd_read(DSD *dsdfile, const char* file_name) {

	FILE *fp = NULL;
	fopen_s(&fp, file_name, "rb");
	assert(fp != NULL);
	fread(dsdfile->dsd_chunk_header, 84, 1, fp);
	fread(& dsdfile->data_size, sizeof(dsdfile->data_size), 1, fp);
	unsigned int nSamples = dsdfile->data_size - 12;				// 数据区总共的大小，每通道为 nSamples / nCh
	if (nSamples != dsdfile->sample_count / 8 * dsdfile->channel_num)
	{
		//printf("Samples not match\n");
		nSamples = (dsdfile->sample_count / 8 * dsdfile->channel_num / 4096) * 4096;
	}
	// read the raw DSD data
	dsdfile->pSampleData = new uint8_t[nSamples]{};				// 初始化
	dsdfile->nBytes = nSamples;

	fread(dsdfile->pSampleData, nSamples, 1, fp);				// 读取

	printf("     DSD file messages:\n");
	printf("================================\n");
	printf("Sample rate	:	%d\n", dsdfile->sample_rate);
	printf("DSD version	:	%d\n", dsdfile->fmt_version);
	printf("Channels	:	%d\n", dsdfile->channel_num);
	printf("Bit per sample	:	%d\n", dsdfile->bits_per_sample);
	printf("Samples	count	:	%d\n", dsdfile->sample_count);
	printf("Total bytes	:	%d\n", dsdfile->nBytes);
	printf("Block size	:	%d\n", dsdfile->block_per_channel);
	printf("Duration(s)	:	%d\n", dsdfile->sample_count / dsdfile->sample_rate);
	printf("================================\n");


	fclose(fp);
	return 0;
}
int dsd_decode(DSD *dsdfile, float *pFloat_out[2],size_t &samples_per_ch) {

	// initialize the decode module
	dsd2pcm_ctx *d2p[2]{};
	d2p[0] = dsd2pcm_init();
	d2p[1] = dsd2pcm_init();
	if (d2p[0] == NULL)
	{
		cout << "D2P initialize erroes" << endl;
		return -1;
	}
	// set the decode parameters
	const int block = 4096;							// default is |4096| for DSF file.
	int channels = dsdfile->channel_num;
	int lsbitfirst = 1	;							// default is |1| for DSF file.

	// 生成 88.2khz , 352.8 176, 88.2
	float *fTmp = (float *)malloc( dsdfile->nBytes * sizeof(float ));
	pFloat_out[0] = fTmp;						
	pFloat_out[1] = fTmp + dsdfile->nBytes / 2;		// right channel

	samples_per_ch = dsdfile->nBytes / 2;
	size_t nFrames = dsdfile->nBytes / (block * channels);
	size_t upIndex[2] = { 0,0 };

	for (size_t n = 0; n < nFrames; n++)
	{
		for (int c = 0; c < channels; ++c) {

			dsd2pcm_translate(
				d2p[c], 
				block, 
				dsdfile->pSampleData + n* block * channels + c * block, 
				1,
				lsbitfirst, 
				pFloat_out[c] + upIndex[c], 
				1
			);
			upIndex[c] += block;
		}
	}
	// 30khz lpf
	// 抽取 ，转换到相应的采样率中


	//unsigned char * dsd_data = new unsigned char[block * channels]{};

	//for (size_t n = 0; n < dsdfile->nBytes; n += block * channels) {

	//	memcpy(dsd_data, dsdfile->pSampleData + n, block * channels);		//	dsd_in -> dsd_data,用另一个指针而不是直接 &

	//	for (int c = 0; c < channels; ++c) {

	//		//dxds[c].translate(block  , &dsd_data[0] + c * block , 1,
	//		//	lsbitfirst,
	//		//	float_out_352[c] + upIndex[c], 1);
	//		dsd2pcm_translate(d2p[c], block, &dsd_data[0] + c * block, 1,
	//			lsbitfirst, pFloat_out[c] + upIndex[c], 1);

	//		//memcpy( float_out_352[c]+ index[c], &float_data[0], block * sizeof(float));

	//		upIndex[c] += block;
	//	}
	//}

	dsd2pcm_destroy(d2p[0]);
	dsd2pcm_destroy(d2p[1]);

	samples_per_ch = upIndex[0];
	return 0;
}
int dsd_frame_process(DSD *dsdfile, size_t block) {



	return 0;


}
int main()
{
;

	DSD dsdfile;
	dsd_read(&dsdfile, "2L-125_stereo-2822k-1b_04.dsf");
	//dsd_read(&dsdfile, "sweep-176400hz-0-22050hz-20s-D64-2.8mhz.dsf");
	//dsd_read(&dsdfile, "08 - David Elias - Crossing - Morning Light Western Town (DSD64 2.0).dsf");

	// ========== 以下解码测试 ==========//
	// general a 352.8khz wav file.( f64 -> f8 )
	float *float_out_352[2] = {};							// 双声道 输出
	size_t nSamplse_per_ch = 0;
	dsd_decode(&dsdfile, float_out_352, nSamplse_per_ch);

	// resample
	unsigned int pos = 0;
	float *float_out_884[2]{};
	unsigned int nStep = 1;	// 352->88.4
	float_out_884[0] = (float*)malloc(sizeof(float)*nSamplse_per_ch / nStep);
	float_out_884[1] = (float*)malloc(sizeof(float)*nSamplse_per_ch / nStep);
	float outTmp = 0;
	unsigned int count = 0;
	unsigned int s = 0;
	for (size_t n = 0; n < nSamplse_per_ch; n++)
	{
		fir(nTaps, coeffs, state, float_out_352[0][n], outTmp,pos);
		float_out_884[0][n ] = outTmp;		// 抽取

		//if (++count == nStep)
		//{
		//	float_out_884[0][n / nStep] = outTmp;		// 抽取
		//	count = 0;
		//}
	}

	// output
	wavwrite_float("v4 d2p music - DSD mono 884  .wav", float_out_884, nSamplse_per_ch / nStep, 1, 44100*2);
	//wavwrite_float("v3 d2p music - DSD mono 352  .wav", float_out_352, nSamplse_per_ch, 1, 44100*2);
	free(float_out_352[0]);

	return 0;
}




// ++++++++++++++++++  dsd2pcm 原有的
	//vector<unsigned char> dsd_data(block * channels);		// 这个是每一帧的,帧长为block * channels 

	//vector<unsigned char> pcm_data(block * channels * bytespersample);	// 用于导出 PCM
	//char * const dsd_in = reinterpret_cast<char*>(&dsd_data[0]);
	//char * const pcm_out = reinterpret_cast<char*>(&pcm_data[0]);



			//unsigned char * out = &pcm_data[0] + c * bytespersample;

			//if (bits == 16) {
			//	for (int s = 0; s<block; ++s) {
			//		float r = float_data[s] * 32768 + ns[c].get();
			//		long smp = clip(-32768, myround(r), 32767);
			//		ns[c].update(clip(-1, smp - r, 1));

			//		//write_intel16(out, smp);
			//		write_intel16( (unsigned char*)&pOut_s16 + n + s, smp);

			//		//out += channels * bytespersample;
			//	}
			//}
			//else {
			//	for (int s = 0; s<block; ++s) {
			//		float r = float_data[s] * 8388608;
			//		long smp = clip(-8388608, myround(r), 8388607);

			//		//write_intel24(out, smp);

			//		//out += channels * bytespersample;
			//	}
			//}


// ========== 这些是原先的测试 ==========//
//uint8_t *ch1 = new uint8_t[nSamples / 2]{ 0 };
//uint8_t *ch2 = new uint8_t[nSamples / 2]{ 0 };

//cout << dsdfile.channel_num << endl;
//cout << dsdfile.block_per_channel << endl;

//for (size_t i = 0; i < nSamples / 2 / 4096; i++)	// chn = 2, 4096 is block size
//{
//	ch1[i] = pSampleData[i * 2 * 4096];
//	ch2[i] = pSampleData[i * 2 * 4096 + 1];

//}
// 抽取 
//for (size_t n = 0; n < samples_per_ch; n++)
//{
//	pOut_882_u8[n] = ch1[ n * nStep];
//}

//float *pOut_882 = new float[samples_per_ch] {};

// FIR   转化，从 DSD 到 PCM  待补充





















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