#include <stm32f4xx.h>
#include <math.h>
#include "main.h"
#include "config.h"
#include "backend/hw/board.h"
#include "backend/hw/headphone.h"
#include "backend/systime/systime.h"
#include "blocks/sources.h"
#include "blocks/sinks.h"
#include "backend/arm_math.h"
#include "util.h"
#include "backend/printfn/printfn.h"
#include "example.h"
#if SYSMODE == SYSMODE_LMS
#include "lab_lms.h"
#elif SYSMODE == SYSMODE_OFDM
#include "lab_ofdm.h"
#endif
/**
 * This project is intended to act as an easy way of getting started with
 * real-time applied signal processing in hardware. All initialization and
 * backend handling is already implemented using a block-based structute. Some
 * points that are worth keeping in mind are;
 * 	- config.h contains all important compile-time constants, such as the
 * 	global sample-rate and sample blocksize.
 *
 * 	- virtually code can be written (and, arguably, should be written) in the
 * 	lab.h and lab.c file pairs. This allows for easily updating the rest of the
 * 	project, without affecting the code you write, should backend bugs be
 * 	found. The only other modifications required are in this file, where
 * 	initialization and data-processing functions should be called. (For
 * 	example, see the example_test1_init and example_test1 functions).
 *
 * 	- Should the computation time exceed the allowable time per block the
 * 	system will detect this and halt, displaying an error message and blinking
 * 	all status LEDs.
 *
 * 	- By default, only the green and red LED's are used by the backend system,
 * 	meaning that the blue and orange LED's can be freely used with the
 * 	board_set_led() and/or board_toggle_led() functions.
 *
 * 	- The user button state can be read using the board_get_btn() function.
 * 	Alternatively, a function call can be triggered on a button flank by adding
 * 	a line in the main_btn_callback function located in this file.
 *
 * 	- There are several potentially useful utility functions located in util.h
 * 	that can be freely used.
 *
 * 	- There is a printf function that can freely be used in a non-interrupt
 * 	context (IE. in any function called in example.c and lab.c), whose output
 * 	is sent to the serial output. This function supports full use of all
 * 	standard variable types. For example 'printf("foo %f\n", 1.234f);' would
 * 	display 'foo 1.234' and add a newline character to the output. Note that
 * 	only PRINTF_MAX_STRING_SIZE can be printed per printf call and at most
 * 	DEBUG_USART_TX_BUF_LEN characters can be buffered at a time. With the
 * 	default baud rate of 1MBPS around 100000 characters can be printed per
 * 	second. Be sure to configure the terminal application used on the PC for
 * 	viewing the status messages for;
 * 		- Operation at 1 000 000 baud
 * 		- 1 start bit
 * 		- 1 stop bit
 * 		- 8 data bits
 * 		- No parity bit
 * 		- No flow control (neither hardware nor software)
 *
 * 	- Configurable timers can be added using the systime_xx functions. See
 * 	main_idle for a use-example.
 *
 * 	- windows.h contains functions for generating various window functions
 *
 * 	- blocks/gen.h contains functions for generating data vectors
 *
 * 	- Documentation for the arm_xx functions can be found both in the
 * 	CMSIS_doc.tar.gz file as well as online at
 * 	http://www.keil.com/pack/doc/CMSIS/DSP/html/index.html
 *
 * 	- In some situations the hardware may become unresponsive and flashing will
 * 	fail. An alternative flashing method that usually works is to;
 * 		- Copy the file 'firmware.bin' in directory
 * 		'embedded/asp_project/asp_lab/.pioenvs/disco_f407vg/' to the usb mass
 * 		storage device presented by the board.
 * 		- The board (should) be re-flashed with the firmware and will
 * 		(hopefully) work with the flashing method integrated in the IDE.
 * 	If the above procedure should fail the entire chip can be erased; install
 * 	the application in en.stsw-link004.zip (windows only unfortunately) and
 * 	select the chip-erase option.
 *
 * 	- The red LED on the board will light if the microphone input saturates
 * 	(clips). As saturation is highly nonlinear this will very negatively affect
 * 	any data processing that assumes linearity. To resolve this decrease the
 * 	sound amplitude (volume) near the microphone.
 *
 * More details on the backend setup follows. Fundamentally, this project sets
 * up the hardware to;
 * 	- Use an SPI peripheral to sample the PDM microphone. An interrupt is
 * 	triggered every 16 bits, which is fed into a FIFO buffer.
 * 	- Use the DMA peripheral to write the output DAC, reading from a buffer with
 * 	AUDIO_BLOCKSIZE samples for each output (IE. left and right).
 * 	- Use an USART peripheral to write data stored in a FIFO buffer to the
 * 	serial output, which is used for status/debug messages.
 * 	- While not in an interrupt context, the application will idle in the
 * 	main_idle function, waiting for a block of AUDIO_BLOCKSIZE samples from the
 * 	microphone. When this is available the main_audio_callback function is
 * 	called which must then do some arbitrary calculations, that in the end
 * 	generate AUDIO_BLOCKSIZE samples of data that must be sent to the
 * 	blocks_sinks_leftout and blocks_sinks_rightout functions.
 */
systime_t idle_led_timer;

#include "blocks/misc.h"

int main(void){
	//Generate a compiler error on invalid #define statements
	BUILD_BUG_ON(AUDIO_SAMPLE_RATE != 16000 && AUDIO_SAMPLE_RATE != 24000 && AUDIO_SAMPLE_RATE != 48000);
	BUILD_BUG_ON(AUDIO_VOLUME > 85 || AUDIO_VOLUME < 0);
	BUILD_BUG_ON(MIC_VOLUME > 100 || MIC_VOLUME < 0);
	BUILD_BUG_ON(!ISPOW2(OUTRAW_BUFFER_SAMPLES));
	BUILD_BUG_ON(!(SYS_SIGN == 1 || SYS_SIGN == -1));
	SystemInit();
	board_init();

	printf(DEBUG_STARTUPMSG);

	idle_led_timer = systime_get_delay(0);

#if SYSMODE == SYSMODE_TEST1
	example_test1_init();
#elif SYSMODE == SYSMODE_TEST2
	example_test2_init();
	example_test2_btnpress();	//Used to force printing a status message on startup
#elif SYSMODE == SYSMODE_TEST3
	example_test3_init();
#elif SYSMODE == SYSMODE_TEST4
	example_test4_init();
#elif SYSMODE == SYSMODE_LMS
	lab_lms_init();
#elif SYSMODE == SYSMODE_OFDM
	lab_ofdm_init();
#elif SYSMODE == SYSMODE_MONITOR
		example_monitor_init();
#elif SYSMODE == SYSMODE_RADAR || SYSMODE == SYSMODE_RADAR2
		example_radar_init();
#elif SYSMODE == SYSMODE_FFT
		example_fft_init();
#else
#error invalid system mode setup!
#endif
	//Never returns; fills an incoming buffer with microphone samples and drives the stereo analog output
	WavePlayBack();
	//Return statement not strictly needed, included for formality.
	return 0;
}

void main_idle(void){
	/* Generic idle process. Is continuously called from a non-interrupt context
	 * when the callback function is completed. */
	//Toggle the idle LED occasionally
	if(systime_get_delay_passed(idle_led_timer)){
		idle_led_timer = systime_add_delay(MS2US(IDLE_LED_BLINK_PER_ms/2), idle_led_timer);
		board_toggle_led(IDLE_LED);
	}
}

void main_audio_callback(void){
	/* Called when AUDIO_BLOCKSIZE samples need to be supplied to the left and
	 * right audio sinks. At this point we can also safely read AUDIO_BLOCKSIZE
	 * samples from source blocks. Is called from a non-interrupt context. */
#if SYSMODE == SYSMODE_TEST1
	example_test1();
#elif SYSMODE == SYSMODE_TEST2
	example_test2();
#elif SYSMODE == SYSMODE_TEST3
	example_test3();
#elif SYSMODE == SYSMODE_TEST4
	example_test4();
#elif SYSMODE == SYSMODE_LMS
	lab_lms();
#elif SYSMODE == SYSMODE_OFDM
	lab_ofdm();
#elif SYSMODE == SYSMODE_MONITOR
		example_monitor();
#elif SYSMODE == SYSMODE_RADAR || SYSMODE == SYSMODE_RADAR2
		example_radar();
#elif SYSMODE == SYSMODE_FFT
				example_fft();
#else
#error invalid system mode setup!
#endif
}

void main_btn_callback(const bool state, struct btn_debounce_s * const btn_instance){
	/* Called when when the user button is pressed and depressed.
	boolean variable state is true when pressed and false when depressed
	 Is called from a non-interrupt context. */

	(void) btn_instance;	//Cast to void to suppress an unused parameter warning
	if(state){
#if SYSMODE == SYSMODE_TEST2
		example_test2_btnpress();
#elif SYSMODE == SYSMODE_TEST4
		example_test4_btnpress();
#elif SYSMODE == SYSMODE_OFDM
		//TODO: start message transmission
#endif
		//Do something optional here on a button press event
	}else{
		//Do something optional here on a button release event
	}
}
