#pragma once
#include "dsdfile.h"
#ifdef __cplusplus
extern "C" {
#endif // _cplusplus

	DSD* dsd2pcm_impl_read(const char * filename);
	void dsd2pcm_impl_decode_352();
	void dsd2pcm_impl_decode_176();// state 2 
	void dsd2pcm_impl_write();

#ifdef __cplusplus
}
#endif // _cplusplus