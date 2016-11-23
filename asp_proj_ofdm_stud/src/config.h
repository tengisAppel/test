/** @file Contains all compile-time constants and general system configuration */

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include "macro.h"
#include "backend/hw/board.h"

/** @brief Enumerates all possible system modes
 * SYSMODE_TEST1; will compile the project in a basic loop-back mode.
 * Will output the waveform to the left channel and output the microphone input
 * to the right channel. Can be used to verify the functionality of the entire
 * board and build environment.
 * SYSMODE_TEST2; will compile the project to apply one of several simple audio
 * DSP effects to either the internal waveform or the microphone input,
 * switching between effects depending on keys pressed on the keyboard and
 * between audio sources when pressing the user button.
 * SYSMODE_TEST3; will compile the project to apply a frequency shift to the
 * microphone input and play back the result on both output channels. The
 * frequency shift is implemented using the FFT, where the size of the FFT is
 * set to AUDIO_BLOCKSIZE.
 * SYSMODE_TEST4; will compile the project to output a QPSK-modulated ASCII
 * bitstream on the left channel when the button is pressed and implement a ML
 * receiver on the microphone input, displaying received characters on the
 * terminal.
 * SYSMODE_MONITOR will compile the project for an illustration of the use
 * of the monitor functionality.
 * SYSMODE_LMS; will compile the project for the LMS adaptive filter project.
 * Will call the lab_dsp_lms() function when AUDIO_BLOCKSIZE samples are
 * available from the microphone.
 * SYSMODE_OFDM; will compile the project for the OFDM project.
 * Will call the lab_ofdm() function when AUDIO_BLOCKSIZE samples are
 * available from the microphone.
 * Will call the lab_ofdm() function when AUDIO_BLOCKSIZE samples are
 * available from the microphone.*/
#define SYSMODE_TEST1				(1)
#define SYSMODE_TEST2				(2)
#define SYSMODE_TEST3				(3)
#define SYSMODE_TEST4				(4)
#define SYSMODE_MONITOR     (5)
#define SYSMODE_LMS					(6)
#define SYSMODE_OFDM				(7)
#define SYSMODE_RADAR				(8)
#define SYSMODE_FFT  				(9)


/******************************************************************************
 *
 * 			   The following parameters may be freely modified
 *
 *****************************************************************************/

/** @brief Sets the desired system mode */
#define SYSMODE 					SYSMODE_OFDM


/** @brief Global audio sample rate [Hz]
 * Due to hardware limitations, the sample-rate must be evenly divisible by
 * 48kHz and no less than 16kHz. This gives a total of three valid sample-rates;
 * 48kHz, 24kHz, and 16kHz.
 * Note that higher sample rates will reduce the computational capacity
 * available as follows;
 * 	48kHz approx. 60% CPU time available for application-level code
 * 	24kHz approx. 80% CPU time available for application-level code
 * 	16kHz approx. 90% CPU time available for application-level code */
#if SYSMODE == SYSMODE_TEST1 || SYSMODE == SYSMODE_TEST2 || SYSMODE == SYSMODE_FFT
/** @brief Sample-rate for examples that are relatively computationally lightweight */
#define AUDIO_SAMPLE_RATE 			(48000)
#elif SYSMODE == SYSMODE_TEST3 || SYSMODE == SYSMODE_RADAR || SYSMODE == SYSMODE_RADAR2
/** @brief Sample-rate for examples that are relatively computationally heavy */
#define AUDIO_SAMPLE_RATE			(24000)
#elif SYSMODE == SYSMODE_TEST4
/** @brief For test 4, the sample rate must be a power of two in order to
 * generate a sinusoid that perfectly fits in AUDIO_BLOCKTIME */
#define AUDIO_SAMPLE_RATE			(16000)
#elif SYSMODE == SYSMODE_LMS
/** @brief The LMS lab has no real sample-rate requirements, so arbitrarily
 * select the lowest sample-rate */
#define AUDIO_SAMPLE_RATE			(16000)
#elif SYSMODE == SYSMODE_OFDM || SYSMODE == SYSMODE_MONITOR
/** @brief For the OFDM lab we need as much time as possible for data
 * processing, so select a low sample rate */
#define AUDIO_SAMPLE_RATE			(16000)
#else
#error Invalid system mode selected
#endif

/** @brief Block size for each speaker callback function.
 * Speaker callbacks will expect this many samples to be present on each call.
 * Larger values will increase the net audio path latency, but result in lower
 * task-switching overhead. Reasonable values range from 64 to 1024 with 256
 * being a reasonable default.
 * For speed, this must be a power of two. */
#if SYSMODE == SYSMODE_TEST1 || SYSMODE == SYSMODE_TEST2
/** @brief Arbitrarily select a 256-sample block size */
#define AUDIO_BLOCKSIZE				(256)
#elif SYSMODE == SYSMODE_FFT
/** @brief Select a 192-sample block size  to fit 64 length filter and
* 256 samples fft  */
#define AUDIO_BLOCKSIZE				(256)
#elif SYSMODE == SYSMODE_TEST3
/** @brief Force a large block size for the FFT example in order to have narrow
 * frequency bins and reduce the AM effect from windowing. */
#define AUDIO_BLOCKSIZE				(1024)
#elif SYSMODE == SYSMODE_TEST4
/** @brief In test 4, the symbol length is directly set by the block size.
 * 256 samples gives a reasonable (but relatively long) symbol length. */
#define AUDIO_BLOCKSIZE				(256)
#elif SYSMODE == SYSMODE_LMS
/** @brief Arbitrarily select a 256-sample block size for the LMS lab. This value is not particularly critical */
#define AUDIO_BLOCKSIZE				(256)
#elif SYSMODE == SYSMODE_OFDM || SYSMODE == SYSMODE_MONITOR || SYSMODE == SYSMODE_RADAR || SYSMODE == SYSMODE_RADAR2
/** @brief Arbitrarily select a large block size as this gives us a longer time
 * to perform signal processing calculations before the microphone/output DMA
 * buffers need to be handled */
#define AUDIO_BLOCKSIZE				(1024)
#else
#error Invalid system mode selected
#endif

/** @brief Microphone prefilter low-pass cutoff frequency.
 * Set to zero to disable and slightly reduce the computational load. */
#define MIC_LP_FC					(0)
/** @brief Microphone prefilter high-pass cutoff frequency.
 * Set to zero to disable and slightly reduce the computational load.
 * Recommended to keep at 20-50Hz. */
#define MIC_HP_FC					(20)

/** @brief Master logarithmic output volume (amplitude) scaling factor.
 * Valid range are integers 0 - 85.
 * Value must be <= 85 to ensure output clipping does not occur with input
 * signals in the range [-1, 1]. Output amplitude is proportional to
 * 10^AUDIO_VOLUME. */
#define AUDIO_VOLUME				(70)

/** @brief Microphone gain.
 * Valid range is integers 0 - 100 */
#define MIC_VOLUME					(100)

/** @brief Set to 1 or -1 to control the net sign through the speakers and microphone */
#define SYS_SIGN					(-1)

/******************************************************************************
 *
 * The following parameters may be freely changed, but are set to (hopefully)
 * reasonable defaults that shound't need to be modified.
 *
 *****************************************************************************/

#define BACKEND_VERSION				1.2.0
#define EXAMPLES_VERSION			1.2.0
#define LAB_VERSION					  1.0.0

/** @brief Sets the period for printing various debug data  */
#define DEBUG_PRINT_PERIOD_ms		(250)

/** @brief Debug USART interface baud rate */
#define DEBUG_USART_BAUDRATE		(921600ULL)
//#define DEBUG_USART_BAUDRATE		(1000000ULL)

/** @brief Number of elements for debug USART transmit buffer */
#define DEBUG_USART_TX_BUF_LEN		(8192)
/** @brief Number of elements for debug USART recieve buffer */
#define DEBUG_USART_RX_BUF_LEN		(16)

/** @brief Message to print at startup */
#define DEBUG_STARTUPMSG 			"\n\nCompiled on " __DATE__ " at " __TIME__" with;"	\
"\n\tBackend version "xstr(BACKEND_VERSION)												\
"\n\tExample version "xstr(EXAMPLES_VERSION)											\
"\n\tLab version "xstr(LAB_VERSION)														\
"\nSystem setup;"																		\
"\n\tSample rate           "xstr(AUDIO_SAMPLE_RATE)" Hz"								\
"\n\tBlocksize;            "xstr(AUDIO_BLOCKSIZE)" samples"								\
"\n\tMic. lowpass  cutoff; "xstr(MIC_LP_FC)" Hz"										\
"\n\tMic. highpass cutoff; "xstr(MIC_HP_FC)" Hz\n\n"

/** @brief Error message to print on a processing time overrun */
#define DEBUG_TIME_OVERFLOWMSG		"\nERROR! Execution time exceeded maximum permissible time per sample! Halting =(\n"

/** @brief Error message to print on a microphone buffer overflow */
#define DEBUG_MICBUG_OVERFLOWMSG	"\nERROR! Microphone input buffer overflow! Halting =(\n"

/** @brief Systick timer frequency */
#define BOARD_SYSTICK_FREQ_Hz		(1000)

/** Maximum string size allowed for printf family [bytes]. */
#define PRINTF_MAX_STRING_SIZE		(4096)

/** @brief LED to use for clipped input indication */
#define DEBUG_LED_CLIP				(board_led_red)

/** @brief Period for idle LED indicator */
#define IDLE_LED_BLINK_PER_ms		(500)

/** @brief The button debounce interval.
 * The button must be kept at this value for at least this time to be regarded
 * as stable */
#define DEBOUNCE_TIME_MIN_ms		(50)

/** @brief LED to use for system-alive indiation */
#define IDLE_LED					(board_led_green)

/** @brief The time duration each block corresponds to */
#define AUDIO_BLOCKTIME_s 			((AUDIO_BLOCKSIZE*1.0f)/(AUDIO_SAMPLE_RATE*1.0f))

/** @brief Number of samples (left and right) for each output DMA buffer */
#define OUTRAW_BUFFER_SAMPLES 		(AUDIO_BLOCKSIZE*2)

#endif /* SRC_CONFIG_H_ */
