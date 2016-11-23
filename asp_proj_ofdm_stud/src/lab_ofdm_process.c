/*
 * lab_ofdm_process.c
 *
 *  Created on: 6 Sep. 2016
 *
 *  modified NOV 21, 2016
 *      Author: mckelvey
 */
#include <stdlib.h>
#include "lab_ofdm_process.h"
#include "backend/arm_math.h"
#include "blocks/sources.h"
#include "blocks/sinks.h"
#include "util.h"
#include "macro.h"
#include "config.h"
#include "backend/systime/systime.h"
#include "backend/printfn/printfn.h"
#include "arm_math.h"
#include "arm_const_structs.h"

#if SYSMODE == SYSMODE_OFDM

char message[LAB_OFDM_CHAR_MESSAGE_SIZE] = "Hello World!AAA";
char pilot_message[LAB_OFDM_CHAR_MESSAGE_SIZE] = "Pilot Signal!";
float ofdm_buffer[2*LAB_OFDM_BLOCKSIZE];
float ofdm_pilot_message[2*LAB_OFDM_BLOCKSIZE];
float bb_transmit_buffer_pilot[2*(LAB_OFDM_BLOCK_W_CP_SIZE)];
float bb_transmit_buffer_message[2*(LAB_OFDM_BLOCK_W_CP_SIZE)];
float bb_transmit_buffer[2*(LAB_OFDM_BB_FRAME_SIZE)];
float bb_receive_buffer[2*(LAB_OFDM_BB_FRAME_SIZE)];
float bb_receive_buffer_message[2*(LAB_OFDM_BLOCK_W_CP_SIZE)];
float bb_receive_buffer_pilot[2*(LAB_OFDM_BLOCK_W_CP_SIZE)];
float ofdm_rx_message[2*LAB_OFDM_BLOCKSIZE];
float ofdm_rx_pilot[2*LAB_OFDM_BLOCKSIZE];
float ofdm_received_message[2*LAB_OFDM_BLOCKSIZE];
float hhat_conj[2*LAB_OFDM_BLOCKSIZE];
float soft_symb[2*LAB_OFDM_BLOCKSIZE];
char rec_message[LAB_OFDM_CHAR_MESSAGE_SIZE];

// LP filter with cutoff frequency = fs/LAB_OFDM_UPSAMPLE_RATE/2
// In Matlab designed with command
// R=8;[B,err] = firpm(63,[0 1/R 1/R*1.6 1],[1 1 0 0]);

float lp_filter[LAB_OFDM_FILTER_LENGTH] = { // LP filter with cutoff 1/(2*R)
  -2.496956319982458e-03f,
   1.061123057910895e-03f,
   1.799575981839800e-03f,
   2.677675432046559e-03f,
   3.246366234977765e-03f,
   3.090521427050170e-03f,
   1.954953083021346e-03f,
  -1.303892962678101e-04f,
  -2.795326496320816e-03f,
  -5.356144072505601e-03f,
  -6.972193079604380e-03f,
  -6.867088217447664e-03f,
  -4.610028643185672e-03f,
  -3.312566693466183e-04f,
   5.175765824001333e-03f,
   1.054753460776610e-02f,
   1.411890548432881e-02f,
   1.436065007374038e-02f,
   1.032985773994797e-02f,
   2.132543581602375e-03f,
  -8.919519113938091e-03f,
  -2.035820813689276e-02f,
  -2.895083648554942e-02f,
  -3.135674961111528e-02f,
  -2.489424533023874e-02f,
  -8.266237880079758e-03f,
   1.794823774717696e-02f,
   5.118254654955430e-02f,
   8.719283811339594e-02f,
   1.207736510603734e-01f,
   1.467272709948181e-01f,
   1.608677439150870e-01f,
   1.608677439150870e-01f,
   1.467272709948181e-01f,
   1.207736510603734e-01f,
   8.719283811339594e-02f,
   5.118254654955430e-02f,
   1.794823774717696e-02f,
  -8.266237880079758e-03f,
  -2.489424533023874e-02f,
  -3.135674961111528e-02f,
  -2.895083648554942e-02f,
  -2.035820813689276e-02f,
  -8.919519113938091e-03f,
   2.132543581602375e-03f,
   1.032985773994797e-02f,
   1.436065007374038e-02f,
   1.411890548432881e-02f,
   1.054753460776610e-02f,
   5.175765824001333e-03f,
  -3.312566693466183e-04f,
  -4.610028643185672e-03f,
  -6.867088217447664e-03f,
  -6.972193079604380e-03f,
  -5.356144072505601e-03f,
  -2.795326496320816e-03f,
  -1.303892962678101e-04f,
   1.954953083021346e-03f,
   3.090521427050170e-03f,
   3.246366234977765e-03f,
   2.677675432046559e-03f,
   1.799575981839800e-03f,
   1.061123057910895e-03f,
  -2.496956319982458e-03f,
   };

/* Data structures for OFDM processing */
arm_fir_decimate_instance_f32 S_decim;
float pState_decim[LAB_OFDM_TX_FRAME_SIZE+(LAB_OFDM_FILTER_LENGTH)-1];
arm_fir_interpolate_instance_f32 S_intp;
float pState_intp[(LAB_OFDM_BB_FRAME_SIZE)+(LAB_OFDM_FILTER_LENGTH/LAB_OFDM_UPSAMPLE_RATE)-1];

/* Scratch buffers for temporary storage*/
float br_tx[LAB_OFDM_TX_FRAME_SIZE], bi_tx[LAB_OFDM_TX_FRAME_SIZE];
float br_bb[LAB_OFDM_BB_FRAME_SIZE], bi_bb[LAB_OFDM_BB_FRAME_SIZE];
float pTmp[2*LAB_OFDM_BLOCKSIZE];

// volume for transmitted signal
float volume = 4;

void lab_ofdm_process_init(void){
	arm_fir_decimate_init_f32 (&S_decim, LAB_OFDM_FILTER_LENGTH, LAB_OFDM_UPSAMPLE_RATE, lp_filter, pState_decim, LAB_OFDM_TX_FRAME_SIZE);
	arm_fir_interpolate_init_f32 (&S_intp, LAB_OFDM_UPSAMPLE_RATE, LAB_OFDM_FILTER_LENGTH, lp_filter, pState_intp, LAB_OFDM_BB_FRAME_SIZE);
  printf("OFDM initialized!\n");
}

void lab_ofdm_process_qpsk_encode(char * pMessage, float * pDst, int Mlen){
  /*
   * Encode the character string in pMessage[] of length Mlen
   * as a complex signal pDst[] with
   * QPSK encoding
   * Note that Each character requires 8 bits = 4 QPS symbols = 4 complex
   * numbers = 8 places in the pDest array
   * Hence pDest must have a length of at least 8 * Mlen
   * Even bit numbers correspond to real part odd bit numbers correspond to imginary part
   */
	int i, ii=0, idx=0;
	char x;
	for (i=0; i<Mlen; i++){
		x = pMessage[i];
		for (ii=0; ii<8; ii++){
			if (x & 0x01){ // test the LSB value in character
				pDst[idx++] = 1;
			} else {
				pDst[idx++] = -1;
			}
			x = x >> 1; // shift right to make next bit LSB
		}
	}
}
void lab_ofdm_process_qpsk_decode(float * pSrc, char * pMessage,  int Mlen){
  /*
  * Decode QPSK signal pSrc[] into a character string pMessage[] of length Mlen
  */
	int i, ii=0, idx=0;
	char x;
	for (i=0; i<Mlen; i++){
		x = 0;
		for (ii=0; ii<8; ii++){
			x = x >> 1;             // shift right 1 step
			if (pSrc[idx++] > 0) {  // Set MSB bit if signal value positive
				x |=  0x80;
			}
		}
		pMessage[i]= x;
	}
}

void add_cyclic_prefix(float * pSrc, float * pDst, int length, int cp_length){
  /* Add a cyclic prefix of length cp_length to complex signal in pSrc[]
   * with length length and cp_size refers to number of complex samples
   * and place result in pDst[]. Note that pDst[] must have a size of
   * at least length+cp_length
   * */
	int i,idx=0;
	int ii = length*2-cp_length*2;
	for (i=0; i<2*cp_length; i++){
		pDst[idx++] = pSrc[ii++];
	}
	for (i=0; i<2*length; i++) {
		pDst[idx++] = pSrc[i];
	}
}

void remove_cyclic_prefix(float *pSrc, float * pDst, int slen, int cp_size){
  /* Remove a cyclic prefix to complex signal in pSrc
   * slen and cp_size refers to number of complex samples of signal w/o prefix
   * and prefix part respectively
   * */
	int i;
	int ii = cp_size*2;
	for (i=0; i<2*slen; i++) {
		pDst[i] = pSrc[ii++];
	}
}

void ofdm_modulate(float * pRe, float * pIm, float* pDst, float f , int length){
  /*
   * Modulates a discrete time signal with the complex exponential exp(i*2*pi*f)
   * and saves the real part of the signal in vector pDst
   */
	int i;
	float inc,omega=0;
	inc = 2*f*M_PI;
	for(i=0; i< length; i++ ){
		pDst[i] = pRe[i] * arm_cos_f32(omega) - pIm[i] * arm_sin_f32(omega);
		omega += inc;
	}
}
void ofdm_demodulate(float * pSrc, float * pRe, float * pIm,  float f, int length ){
  /*
   * Demodulate a real signal (pSrc) into a complex signal (pRe and pPim)
   * with modulation center frequency f and the signal length is length
   */
	int i;
	float inc,omega=0;

	/* Add code here */

}

void cnvt_cmplx_2_re_im( float * pCmplx, float * pRe, float * pIm, int length ){
  /*
  * Convert a complex signal (pCmplx) into two vectors. One vector with the real part (pRe) and
  * one vector with the complex part (pIm). The length of the signal is length.
  */
  int i;
  for ( i = 0; i < length ;i++) {
    pRe[i] = pCmplx[2*i];
    pIm[i] = pCmplx[2*i+1];
  }
}
void cnvt_re_im_2_cmplx( float * pRe, float * pIm, float * pCmplx, int length ){
  /*
  * Converts a complex signal in the form of two vectors (pRe and pIm)
  * into one float vector of size 2*length where the real and imaginary parts are
  * interleaved. i.e [pCmplx[0]=pRe[0], pCmplx[1]=pIm[0],pCmplx[2]=pRe[1], pCmplx[3]=pIm[1]
  * etc.
  */
  int i;
  for ( i = 0; i < length ;i++) {
    /* Add code here */
  }
}
void concat(float * pSrc1, float * pSrc2, float * pDst, int length){
  /*
  * Concatenate vectors pSrc1[] and pSrc2[] and place result in
  * pDst[]. Length of pSrc1 and pSrc2 are assumed to be length and it is
  * required that pDst has length 2*length
  */
  int i,ii=0;
	for (i=0; i<length; i++){
		pDst[ii++] = pSrc1[i];
	}
	for (i=0; i<length; i++){
		pDst[ii++] = pSrc2[i];
	}
}
void split(float * pSrc, float * pDst1, float * pDst2, int length){
  /*
  * Split vecor pSrc into two vectors pDst1 and pDst2 each with a length length
  */
	int i,ii=0;
	for (i=0; i<length; i++){
		pDst1[i] = pSrc[ii++];
	}
	for (i=0; i<length; i++){
		pDst2[i] = pSrc[ii++];
	}
}

void ofdm_conj_equalize(float * prxMes, float * prxPilot,
		float * ptxPilot, float * pEqualized, float * hhat_conj, int length){
/*
*   Equalize the channel by multiplying with the conjugate of the channel
*  INP:
*   prxMes[] - complex vector with received data message in frequency domain (FD)
*   prxPilot[] - complex vector with received pilot in FD
*   ptxPilot[] - complex vector with transmitted pilot in FD
*   lenght  - number of complex OFDM symbols
*  OUT:
*   pEqualized[] - complex vector with equalized data message (Note: only phase
*   is equalized)
*   hhat_conj[] -  complex vector with estimated conjugated channel gain
*/
	int i;
	/* Add code here */
/*   Estimate the conjugate of channel by multiplying the conjugate of prxPilot with
 *   ptxPilot and scale with 0.5
 *   use a combination of the functions
	arm_cmplx_conj_f32()
	arm_cmplx_mult_cmplx_f32()
  the reference page for these DSP functions can be found here:
  http://www.keil.com/pack/doc/CMSIS/DSP/html/index.html
  // Estimate the message by multiplying prxMes with the conjugate channel
  // and store it in pEqualized
*/
}

void ofdm_soft_symb(float * prxMes, float * hhat_conj, float * soft_symb, int length){
  /*
  * Estimate message symbols by dividing with estimated channel
  * Note that s_k / H_k = s_k * conj(H_k) / abs(H_k)
  */
  int i;
  float pTmp[LAB_OFDM_BLOCKSIZE];
	arm_cmplx_mult_cmplx_f32( hhat_conj, prxMes, soft_symb, length);
	arm_cmplx_mag_squared_f32(hhat_conj, pTmp, length);
	for (i=0; i<length; i++){
		soft_symb[2*i]  = soft_symb[2*i]/pTmp[i];
		soft_symb[2*i+1]  = soft_symb[2*i+1]/pTmp[i];
	}
}

void lab_ofdm_process_tx(float * real_tx){
  /* Create one frame including an ofdm pilot and ofdm message message block
  */
	/* Encode pilot string to qpsk symbols */
	lab_ofdm_process_qpsk_encode( pilot_message , ofdm_buffer, LAB_OFDM_CHAR_MESSAGE_SIZE);
	/* perform IFFT on ofdm_buffer */
	BUILD_BUG_ON(LAB_OFDM_BLOCKSIZE != 64);
	arm_cfft_f32(&arm_cfft_sR_f32_len64, ofdm_buffer, LAB_OFDM_IFFT_FLAG, LAB_OFDM_DO_BITREVERSE);
	// Add cyclic prefix
	add_cyclic_prefix(ofdm_buffer, bb_transmit_buffer_pilot, LAB_OFDM_BLOCKSIZE, LAB_OFDM_CYCLIC_PREFIX_SIZE);
	/* Encode message string to qpsk sybols */
	lab_ofdm_process_qpsk_encode( message , ofdm_buffer, LAB_OFDM_CHAR_MESSAGE_SIZE);
	/* perform IFFT on ofdm_buffer */
	BUILD_BUG_ON(LAB_OFDM_BLOCKSIZE != 64);
	arm_cfft_f32(&arm_cfft_sR_f32_len64, ofdm_buffer, LAB_OFDM_IFFT_FLAG, LAB_OFDM_DO_BITREVERSE);
	// Add cyclic prefix
	add_cyclic_prefix(ofdm_buffer, bb_transmit_buffer_message, LAB_OFDM_BLOCKSIZE, LAB_OFDM_CYCLIC_PREFIX_SIZE);
	// Add Pilot + Message to create a full base band frame
	concat(bb_transmit_buffer_pilot, bb_transmit_buffer_message, bb_transmit_buffer, 2*LAB_OFDM_BLOCK_W_CP_SIZE);
  // Split complex signal into real and imaginary parts
  cnvt_cmplx_2_re_im(bb_transmit_buffer, br_bb, bi_bb, LAB_OFDM_BB_FRAME_SIZE);
  // Interpolate to the audio sampling frequency
  arm_fir_interpolate_f32 (&S_intp, br_bb , br_tx, LAB_OFDM_BB_FRAME_SIZE);
  arm_fir_interpolate_f32 (&S_intp, bi_bb , bi_tx, LAB_OFDM_BB_FRAME_SIZE);
	 // Modulate
	ofdm_modulate(br_tx, bi_tx, real_tx, LAB_OFDM_CENTER_FREQUENCY/AUDIO_SAMPLE_RATE, LAB_OFDM_TX_FRAME_SIZE);
  // Change volume on tranmitted signal
	arm_scale_f32(real_tx, volume, real_tx, LAB_OFDM_TX_FRAME_SIZE);
	 // buffer real_tx now ready for transmission
}

void lab_ofdm_process_rx(float * real_rx_buffer){
	int i;
  ofdm_demodulate(real_rx_buffer, br_tx, bi_tx, LAB_OFDM_CENTER_FREQUENCY/AUDIO_SAMPLE_RATE, LAB_OFDM_TX_FRAME_SIZE);
  // Decimate using arm_fir_decimate_f32() function
  arm_fir_decimate_f32 (&S_decim, br_tx , br_bb, LAB_OFDM_TX_FRAME_SIZE);
  arm_fir_decimate_f32 (&S_decim, bi_tx , bi_bb, LAB_OFDM_TX_FRAME_SIZE);

  // Convert from real and imaginary vectors to a complex vector
  cnvt_re_im_2_cmplx(br_bb, bi_bb, bb_receive_buffer, LAB_OFDM_BB_FRAME_SIZE);

  // Split and Remove Cyclic prefix
	split(bb_receive_buffer,bb_receive_buffer_pilot, bb_receive_buffer_message, 2*LAB_OFDM_BLOCK_W_CP_SIZE);
	remove_cyclic_prefix(bb_receive_buffer_pilot, ofdm_rx_pilot, LAB_OFDM_BLOCKSIZE, LAB_OFDM_CYCLIC_PREFIX_SIZE);
	remove_cyclic_prefix(bb_receive_buffer_message, ofdm_rx_message, LAB_OFDM_BLOCKSIZE, LAB_OFDM_CYCLIC_PREFIX_SIZE);

	//  Perform FFT
	arm_cfft_f32(&arm_cfft_sR_f32_len64, ofdm_rx_pilot, LAB_OFDM_FFT_FLAG, LAB_OFDM_DO_BITREVERSE);
	arm_cfft_f32(&arm_cfft_sR_f32_len64, ofdm_rx_message, LAB_OFDM_FFT_FLAG, LAB_OFDM_DO_BITREVERSE);

	lab_ofdm_process_qpsk_encode( pilot_message , ofdm_pilot_message, LAB_OFDM_CHAR_MESSAGE_SIZE);
	ofdm_conj_equalize(ofdm_rx_message, ofdm_rx_pilot, ofdm_pilot_message, ofdm_received_message, hhat_conj, LAB_OFDM_BLOCKSIZE);

	/* Decode qpsk */
	lab_ofdm_process_qpsk_decode(ofdm_received_message,  rec_message,  LAB_OFDM_CHAR_MESSAGE_SIZE);
	/* Determine SNR here by also calculating the "soft symbols", i.e. by dividing
   * with the channel estimate. */
	ofdm_soft_symb(ofdm_rx_message, hhat_conj, soft_symb, LAB_OFDM_BLOCKSIZE);
  // Here we calulate the "correct" symbols in the message
  lab_ofdm_process_qpsk_encode( message , ofdm_buffer, LAB_OFDM_CHAR_MESSAGE_SIZE);
  // Determine RMSE for the symbols
	float tmp[2*LAB_OFDM_BLOCKSIZE];
	float err_norm=0;
	arm_sub_f32( soft_symb, ofdm_buffer, tmp, 2*LAB_OFDM_BLOCKSIZE);
	arm_cmplx_mag_squared_f32(tmp, tmp, LAB_OFDM_BLOCKSIZE );
	for ( i=0; i< LAB_OFDM_BLOCKSIZE; i++){
		err_norm += tmp[i];
	}
	err_norm = sqrtf(err_norm/LAB_OFDM_BLOCKSIZE);
	printf("Transmitted String: %s\n", message);
	printf("Received String: %s\n", rec_message);
	printf("QPSK symbol RMSE  %f \n\n", err_norm);

}

#endif
