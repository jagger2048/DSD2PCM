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
#include <vector>
#include <string>	//c

#include "noiseshape.hpp"

#include "dsd2pcm.hpp"

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
namespace {

	const float my_ns_coeffs[] = {
		//     b1           b2           a1           a2
		-1.62666423,  0.79410094,  0.61367127,  0.23311013,  // section 1
		-1.44870017,  0.54196219,  0.03373857,  0.70316556   // section 2
	};

	const int my_ns_soscount = sizeof(my_ns_coeffs) / (sizeof(my_ns_coeffs[0]) * 4);

	inline long myround(float x)
	{
		return static_cast<long>(x + (x >= 0 ? 0.5f : -0.5f));
	}

	template<typename T>
	struct id { typedef T type; };

	template<typename T>
	inline T clip(
		typename id<T>::type min,
		T v,
		typename id<T>::type max)
	{
		if (v<min) return min;
		if (v>max) return max;
		return v;
	}

	inline void write_intel16(unsigned char * ptr, unsigned word)
	{
		ptr[0] = word & 0xFF;
		ptr[1] = (word >> 8) & 0xFF;
	}

	inline void write_intel24(unsigned char * ptr, unsigned long word)
	{
		ptr[0] = word & 0xFF;
		ptr[1] = (word >> 8) & 0xFF;
		ptr[2] = (word >> 16) & 0xFF;
	}

} // anonymous namespace

using std::vector;
using std::cin;
using std::cout;
using std::cerr;
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

struct  Biquad
{
	float coeffs[2][3]{};
	float state[3]{};
};
void biquad(Biquad *handles,float in, float &out) {
	// wrapped methods
	handles->state[2] = handles->state[1];
	handles->state[1] = handles->state[0];
	handles->state[0] = in + (-handles->coeffs[1][1]) * handles->state[1] + (-handles->coeffs[1][2]) * handles->state[2];
	// caculate the output
	out = handles->coeffs[0][0] * handles->state[0] + handles->coeffs[0][1] * handles->state[1] + handles->coeffs[0][2] * handles->state[2];
}

void fir(unsigned int nTaps,float *coeffs,float *state,float in,float &out) {

	float sum = 0.0;
	for (size_t i = nTaps - 1; i > 2; --i)		// 需要使用循环队列优化
	{
		state[i] = state[i - 1];
	}
	state[0] = in;

	for (size_t n = 0; n < nTaps; n++)
	{
		sum += coeffs[n] * state[n];
	}
	out = sum;
}

int main()
{
	FILE *fp = NULL;
	fopen_s(&fp, "sine-176400hz-100hz-15s-D64-2.8mhz.dsf","rb");
	assert(fp != NULL);

	DSD dsdfile;
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
	const int block = 16384;		// 4096 * 4 for 352
	int channels = 2;
	int lsbitfirst = 1;		// lsm
	int bits = 16;			// 



	// 生成 88.2khz , 352.8 176, 88.2
	uint16_t  nStep = 32 / 8;	// f64->f2 how man bytes to skip after each sample calc,相当于抽取

	uint8_t *ch1 = new uint8_t[ nSamples / 2]{0};
	uint8_t *ch2 = new uint8_t[ nSamples / 2]{0};

	cout << dsdfile.channel_num << endl;
	cout << dsdfile.block_per_channel << endl;

	for (size_t i = 0; i < nSamples / 2 / 4096; i++)	// chn = 2, 4096 is block size
	{
			ch1[i] = pSampleData[ i * 2 * 4096  ];
			ch2[i] = pSampleData[ i * 2  * 4096 + 1];

	}

	unsigned samples_per_ch = nSamples / 2 / nStep;			// 2 is channel smaples per channel
	uint8_t *pOut_882_u8 = new uint8_t[samples_per_ch ];
	uint8_t *pOut_882_u8_out = new uint8_t[samples_per_ch ];

	// 抽取 
	for (size_t n = 0; n < samples_per_ch; n++)
	{
		pOut_882_u8[n] = ch1[ n * nStep];
	}

	float *pOut_882 = new float[samples_per_ch] {};

	// FIR   转化，从 DSD 到 PCM  待补充


	dsd2pcm_ctx *d2p = dsd2pcm_init();

	//for (size_t i = 0; i < samples_per_ch ; i++)
	//{
		//uint8_t tmp = pOut_882_u8[i];


	int bytespersample = bits / 8;
	vector<dxd> dxds(channels);
	//vector<noise_shaper> ns;
	//if (bits == 16) {
	//	ns.resize(channels, noise_shaper(my_ns_soscount, my_ns_coeffs));
	//}
	vector<unsigned char> dsd_data(block * channels);
	vector<float> float_data(block);
	
	vector<unsigned char> pcm_data(block * channels * bytespersample);
	char * const dsd_in = reinterpret_cast<char*>(&dsd_data[0]);
	char * const pcm_out = reinterpret_cast<char*>(&pcm_data[0]);

	float *float_out[2] = {};
	float_out[0] = new float[nSamples / 2]{};
	float_out[1] = new float[nSamples / 2]{};

	//fread(pSampleData, nSamples, 1, fp);
	//while (cin.read(dsd_in, block * channels)) {
	for (size_t n = 0; n < nSamples;n += block * channels) {


		memcpy(dsd_in, pSampleData + n, block * channels);

		for (int c = 0; c<channels; ++c) {
			dxds[c].translate(block, &dsd_data[0] + c, channels,
				lsbitfirst,
				&float_data[0], 1);

			//float_out.

			memcpy(float_out + c, &float_data[0], block);


			//unsigned char * out = &pcm_data[0] + c * bytespersample;

			//if (bits == 16) {
			//	//for (int s = 0; s<block; ++s) {
			//	//	float r = float_data[s] * 32768 + ns[c].get();
			//	//	long smp = clip(-32768, myround(r), 32767);
			//	//	ns[c].update(clip(-1, smp - r, 1));
			//	//	write_intel16(out, smp);
			//	//	out += channels * bytespersample;
			//	//}
			//}
			//else {
			//	for (int s = 0; s<block; ++s) {
			//		float r = float_data[s] * 8388608;
			//		long smp = clip(-8388608, myround(r), 8388607);
			//		write_intel24(out, smp);
			//		out += channels * bytespersample;
			//	}
			//}
		}
		////cout.write(pcm_out, block*channels*bytespersample);
	}




	//}

	//wavwrite_s16("test DSD mono 882 - v33 .wav", &pOut_882, samples_per_ch , 1, 44100*2);
	//float_out

	wavwrite_float("v2 d2p - DSD mono 882  .wav", float_out, samples_per_ch , 1, 44100*2);


	//fclose(fp);
	dsd2pcm_destroy(d2p);

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