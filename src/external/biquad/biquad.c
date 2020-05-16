// code taken from, stripped down and simplified for EM400:
//
// (c) Copyright 2016, Sean Connelly (@voidqk), http://syntheti.cc
// MIT License
// Project Home: https://github.com/voidqk/sndfilter

#include "external/biquad/biquad.h"
#include <math.h>

// biquad filtering is based on a small sliding window, where the different filters are a result of
// simply changing the coefficients used while processing the samples
//
// the biquad filter processes a sound using 10 parameters:
//   b0, b1, b2, a1, a2      transformation coefficients
//   xn0, xn1, xn2           the unfiltered sample at position x[n], x[n-1], and x[n-2]
//   yn1, yn2                the filtered sample at position y[n-1] and y[n-2]
void sf_biquad_process(sf_biquad_state_st *state, int size, float *input,
	float *output){

	// pull out the state into local variables
	float b0 = state->b0;
	float b1 = state->b1;
	float b2 = state->b2;
	float a1 = state->a1;
	float a2 = state->a2;
	float xn1 = state->xn1;
	float xn2 = state->xn2;
	float yn1 = state->yn1;
	float yn2 = state->yn2;

	// loop for each sample
	for (int n = 0; n < size; n++){
		// get the current sample
		float xn0 = input[n];

		// the formula is the same for each channel
		float L =
			b0 * xn0 +
			b1 * xn1 +
			b2 * xn2 -
			a1 * yn1 -
			a2 * yn2;

		// save the result
		output[n] = (float){ L };

		// slide everything down one sample
		xn2 = xn1;
		xn1 = xn0;
		yn2 = yn1;
		yn1 = output[n];
	}

	// save the state for future processing
	state->xn1 = xn1;
	state->xn2 = xn2;
	state->yn1 = yn1;
	state->yn2 = yn2;
}

// each type of filter just has some magic math to setup the coefficients
//
// the math is quite complicated to understand, but the *implementation* is quite simple
//
// I have no insight into the genius of the math -- you're on your own for that.  You might find
// some help in some of the articles here:
//   http://www.musicdsp.org/showmany.php
//
// formulas extracted and massaged from Chromium source, Biquad.cpp, here:
//   https://git.io/v10H2

// clear the samples saved across process boundaries
static inline void state_reset(sf_biquad_state_st *state){
	state->xn1 = 0;
	state->xn2 = 0;
	state->yn1 = 0;
	state->yn2 = 0;
}

// set the coefficients so that the output is the input scaled by `amt`
static inline void state_scale(sf_biquad_state_st *state, float amt){
	state->b0 = amt;
	state->b1 = 0.0f;
	state->b2 = 0.0f;
	state->a1 = 0.0f;
	state->a2 = 0.0f;
}

// set the coefficients so that the output is an exact copy of the input
static inline void state_passthrough(sf_biquad_state_st *state){
	state_scale(state, 1.0f);
}

// set the coefficients so that the output is zeroed out
static inline void state_zero(sf_biquad_state_st *state){
	state_scale(state, 0.0f);
}

// initialize the biquad state to be a lowpass filter
void sf_lowpass(sf_biquad_state_st *state, int rate, float cutoff, float resonance){
	state_reset(state);
	float nyquist = rate * 0.5f;
	cutoff /= nyquist;

	if (cutoff >= 1.0f)
		state_passthrough(state);
	else if (cutoff <= 0.0f)
		state_zero(state);
	else{
		resonance = powf(10.0f, resonance * 0.05f); // convert resonance from dB to linear
		float theta = (float)M_PI * 2.0f * cutoff;
		float alpha = sinf(theta) / (2.0f * resonance);
		float cosw  = cosf(theta);
		float beta  = (1.0f - cosw) * 0.5f;
		float a0inv = 1.0f / (1.0f + alpha);
		state->b0 = a0inv * beta;
		state->b1 = a0inv * 2.0f * beta;
		state->b2 = a0inv * beta;
		state->a1 = a0inv * -2.0f * cosw;
		state->a2 = a0inv * (1.0f - alpha);
	}
}

void sf_highpass(sf_biquad_state_st *state, int rate, float cutoff, float resonance){
	state_reset(state);
	float nyquist = rate * 0.5f;
	cutoff /= nyquist;

	if (cutoff >= 1.0f)
		state_zero(state);
	else if (cutoff <= 0.0f)
		state_passthrough(state);
	else{
		resonance = powf(10.0f, resonance * 0.05f); // convert resonance from dB to linear
		float theta = (float)M_PI * 2.0f * cutoff;
		float alpha = sinf(theta) / (2.0f * resonance);
		float cosw  = cosf(theta);
		float beta  = (1.0f + cosw) * 0.5f;
		float a0inv = 1.0f / (1.0f + alpha);
		state->b0 = a0inv * beta;
		state->b1 = a0inv * -2.0f * beta;
		state->b2 = a0inv * beta;
		state->a1 = a0inv * -2.0f * cosw;
		state->a2 = a0inv * (1.0f - alpha);
	}
}

