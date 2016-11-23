#ifndef EXAMPLE_H_
#define EXAMPLE_H_

#include "config.h"

#if SYSMODE == SYSMODE_TEST1
// Function pair for the microphone and disturbance example
void example_test1_init(void);
void example_test1(void);
#elif SYSMODE == SYSMODE_TEST2
// Function triplet for the audio effects example
void example_test2_init(void);
void example_test2(void);
void example_test2_btnpress(void);				//!<- Function to call to switch between audio sources
#elif SYSMODE == SYSMODE_TEST3
// Function pair for the frequency shift example
void example_test3_init(void);
void example_test3(void);
#elif SYSMODE == SYSMODE_TEST4
// Function triplet for the crude QPSK example
void example_test4_init(void);
void example_test4(void);
void example_test4_btnpress(void);	//!<- Function to call to trigger a transmission
#elif SYSMODE == SYSMODE_MONITOR
// Function triplet for monitorexample
void example_monitor_init(void);
void example_monitor(void);
void example_monitor_btnpress(void);	//!<- Function to call to trigger a transmission
#elif SYSMODE == SYSMODE_RADAR
// Function pair for the microphone and disturbance example
#define RADAR_F_START (1000.0f)
#define RADAR_F_STOP (3000.0f)
#define RADAR_OFFSET (200)
#define RADAR_SIZE (256)
#define CORR_SIZE  (2*AUDIO_BLOCKSIZE-1)
void example_radar_init(void);
void example_radar(void);
#elif SYSMODE == SYSMODE_RADAR2
// Function pair for the microphone and disturbance example
#define RADAR_F_START (1000.0f)
#define RADAR_F_STOP (3000.0f)
#define RADAR_OFFSET (200)
#define RADAR_SIZE (256)
#define RADAR_PRINT_DELAY (10)
#define CORR_SIZE  (2*AUDIO_BLOCKSIZE-1)
void example_radar_init(void);
void example_radar(void);
#elif SYSMODE == SYSMODE_FFT
// Function pair for the microphone and disturbance example
void example_fft_init(void);
void example_fft(void);
#endif

#endif /* SRC_EXAMPLE_H_ */
