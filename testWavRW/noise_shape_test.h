#pragma once
#include "noiseshape.h"

#define CLIP(min,v,max)  ( (v) < (min) )? (min) : ( ( (v) > (max) )? (max) : (v) )

const float my_ns_coeffs[] = {
	//     b1           b2           a1           a2
	-1.62666423,  0.79410094,  0.61367127,  0.23311013,  // section 1
	-1.44870017,  0.54196219,  0.03373857,  0.70316556   // section 2
};

const int my_ns_soscount = sizeof(my_ns_coeffs) / (sizeof(my_ns_coeffs[0]) * 4);

inline long myround(float x)
{
	return (long)(x + (x >= 0 ? 0.5f : -0.5f));
}