#include "sources.h"
#include "config.h"
#include <math.h>
#include "backend/hw/headphone.h"
#include "util.h"
#include "waveform.h"
#include "arm_math.h"

/** @brief One period of the secret "mystery" disturbance signal */
#if AUDIO_SAMPLE_RATE == 48000
const float dist_sig[96] = {0.000000e+00, 1.827277e-01, 3.585557e-01, 5.210407e-01, 6.646149e-01, 7.849329e-01, 8.791190e-01, 9.458957e-01, 9.855875e-01, 1.000000e+00, 9.921919e-01, 9.661606e-01, 9.264733e-01, 8.778768e-01, 8.249215e-01, 7.716310e-01, 7.212445e-01, 6.760503e-01, 6.373189e-01, 6.053363e-01, 5.795259e-01, 5.586413e-01, 5.410056e-01, 5.247697e-01, 5.081618e-01, 4.897029e-01, 4.683675e-01, 4.436777e-01, 4.157226e-01, 3.851086e-01, 3.528493e-01, 3.202119e-01, 2.885402e-01, 2.590770e-01, 2.328066e-01, 2.103351e-01, 1.918222e-01, 1.769702e-01, 1.650699e-01, 1.550957e-01, 1.458388e-01, 1.360584e-01, 1.246350e-01, 1.107062e-01, 9.376717e-02, 7.372670e-02, 5.091041e-02, 2.601219e-02, 1.670829e-16, -2.601219e-02, -5.091041e-02, -7.372670e-02, -9.376717e-02, -1.107062e-01, -1.246350e-01, -1.360584e-01, -1.458388e-01, -1.550957e-01, -1.650699e-01, -1.769702e-01, -1.918222e-01, -2.103351e-01, -2.328066e-01, -2.590770e-01, -2.885402e-01, -3.202119e-01, -3.528493e-01, -3.851086e-01, -4.157226e-01, -4.436777e-01, -4.683675e-01, -4.897029e-01, -5.081618e-01, -5.247697e-01, -5.410056e-01, -5.586413e-01, -5.795259e-01, -6.053363e-01, -6.373189e-01, -6.760503e-01, -7.212445e-01, -7.716310e-01, -8.249215e-01, -8.778768e-01, -9.264733e-01, -9.661606e-01, -9.921919e-01, -1.000000e+00, -9.855875e-01, -9.458957e-01, -8.791190e-01, -7.849329e-01, -6.646149e-01, -5.210407e-01, -3.585557e-01, -1.827277e-01};
#elif AUDIO_SAMPLE_RATE == 24000
const float dist_sig[48] = {0.000000e+00, 3.613774e-01, 6.698451e-01, 8.860373e-01, 9.933436e-01, 1.000000e+00, 9.337642e-01, 8.314132e-01, 7.269204e-01, 6.423343e-01, 5.840865e-01, 5.452631e-01, 5.121608e-01, 4.720534e-01, 4.189941e-01, 3.556261e-01, 2.908109e-01, 2.346387e-01, 1.933318e-01, 1.663689e-01, 1.469865e-01, 1.256159e-01, 9.450508e-02, 5.131105e-02, 1.683978e-16, -5.131105e-02, -9.450508e-02, -1.256159e-01, -1.469865e-01, -1.663689e-01, -1.933318e-01, -2.346387e-01, -2.908109e-01, -3.556261e-01, -4.189941e-01, -4.720534e-01, -5.121608e-01, -5.452631e-01, -5.840865e-01, -6.423343e-01, -7.269204e-01, -8.314132e-01, -9.337642e-01, -1.000000e+00, -9.933436e-01, -8.860373e-01, -6.698451e-01, -3.613774e-01};
#elif AUDIO_SAMPLE_RATE == 16000
const float dist_sig[32] = { 0.000000e+00, 5.210407e-01, 8.791190e-01, 1.000000e+00, 9.264733e-01, 7.716310e-01, 6.373189e-01, 5.586413e-01, 5.081618e-01, 4.436777e-01, 3.528493e-01, 2.590770e-01, 1.918222e-01, 1.550957e-01, 1.246350e-01, 7.372670e-02, 4.902874e-17, -7.372670e-02, -1.246350e-01, -1.550957e-01, -1.918222e-01, -2.590770e-01, -3.528493e-01, -4.436777e-01, -5.081618e-01, -5.586413e-01, -6.373189e-01, -7.716310e-01, -9.264733e-01, -1.000000e+00, -8.791190e-01, -5.210407e-01};
#else
#error disturbance signal not available at configured sample-rate
#endif

/** @brief The next element from wavetable based waveforms */
int_fast32_t dist_idx = 0;
int_fast32_t waveform_idx = 0;

/** @brief The current angles for sine/cosine sources */
float sin_ang = 0;
float cos_ang = 0;

/** @brief The current frequency to use for sine/cosine outputs, default to 1kHz */
float trig_freq = 1e3;

void blocks_sources_zeros(float * sample_block){
	arm_fill_f32(0.0f, sample_block, AUDIO_BLOCKSIZE);
}

void blocks_sources_ones(float * sample_block){
	arm_fill_f32(1.0f, sample_block, AUDIO_BLOCKSIZE);
}

void blocks_sources_trig_setfreq(float frequency){
	ATOMIC(trig_freq = frequency);
}

void blocks_sources_sin(float * sample_block){
	int_fast32_t i;
	float ang = sin_ang;
	volatile float freq;	//Declare volatile to ensure the math operations are not re-ordered into the atomic block
	ATOMIC(freq = trig_freq);
	const float delta_ang = M_TWOPI * freq / AUDIO_SAMPLE_RATE;
	for(i = 0; i < AUDIO_BLOCKSIZE; i++){
		sample_block[i] = arm_sin_f32(ang);
		ang += delta_ang;
		if(sin_ang > M_TWOPI){
			sin_ang -= M_TWOPI;
		}
	}
}

void blocks_sources_cos(float * sample_block){
	int_fast32_t i;
	float ang = sin_ang;
	volatile float freq;	//Declare volatile to ensure the math operations are not re-ordered into the atomic block
	ATOMIC(freq = trig_freq);
	const float delta_ang = M_TWOPI * freq / AUDIO_SAMPLE_RATE;
	for(i = 0; i < AUDIO_BLOCKSIZE; i++){
		sample_block[i] = arm_cos_f32(ang);
		ang += delta_ang;
		if(sin_ang > M_TWOPI){
			sin_ang -= M_TWOPI;
		}
	}
}

void blocks_sources_waveform(float * sample_block){
	int_fast32_t i;
	for(i = 0; i < AUDIO_BLOCKSIZE; i++){
		sample_block[i] = waveform[waveform_idx++] * (1.0f/INT16_MAX);
		if(waveform_idx > NUMEL(waveform)){
			waveform_idx = 0;
		}
	}
}

void blocks_sources_disturbance(float * sample_block){
	int_fast32_t i;
	for (i = 0; i < AUDIO_BLOCKSIZE; i++) {
		sample_block[i] = dist_sig[dist_idx++] * (1.0f / INT16_MAX);
		if (dist_idx > NUMEL(dist_sig)) {
			dist_idx = 0;
		}
	}
}

void blocks_sources_microphone(float * sample_block){
	arm_copy_f32(processed_micdata, sample_block, AUDIO_BLOCKSIZE);
}

void blocks_sources_update(void){
	//Compute the relative change in angle between each sample for the requested frequency
	volatile float freq;	//Declare volatile to ensure the math operations are not re-ordered into the atomic block
	ATOMIC(freq = trig_freq);
	const float delta_ang = M_TWOPI * freq / AUDIO_SAMPLE_RATE;
	sin_ang += delta_ang * AUDIO_BLOCKSIZE;
	cos_ang += delta_ang * AUDIO_BLOCKSIZE;
}
