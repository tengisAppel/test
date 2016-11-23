#ifndef LAB_OFDM_PROCESS_H_
#define LAB_OFDM_PROCESS_H_

extern char message[];
extern char rec_message[];
extern char stat_message[];

/* #define LAB_BLOCKSIZE  (4096) */
#define LAB_OFDM_CYCLIC_PREFIX_SIZE (32) /* Complex */
#define LAB_OFDM_BLOCKSIZE (64) /* Complex Note must be aligned with FFT size*/
#define LAB_OFDM_UPSAMPLE_RATE (8) // Also used as downsample rate
#define LAB_OFDM_NUM_FRAMES (2)
#define LAB_OFDM_CHAR_MESSAGE_SIZE (LAB_OFDM_BLOCKSIZE / 4)
#define LAB_OFDM_BLOCK_W_CP_SIZE   (LAB_OFDM_CYCLIC_PREFIX_SIZE + LAB_OFDM_BLOCKSIZE) /* Complex */
#define LAB_OFDM_BB_FRAME_SIZE   (LAB_OFDM_NUM_FRAMES * LAB_OFDM_BLOCK_W_CP_SIZE) /* Complex */
#define LAB_OFDM_TX_FRAME_SIZE   ((LAB_OFDM_BB_FRAME_SIZE)*(LAB_OFDM_UPSAMPLE_RATE)) /* Complex */
#define LAB_OFDM_FFT_FLAG (0)
#define LAB_OFDM_IFFT_FLAG (1)
#define LAB_OFDM_DO_BITREVERSE (1)
#define LAB_OFDM_FILTER_LENGTH (64)
#define LAB_OFDM_CENTER_FREQUENCY (4000.0f)

// /** @brief Storage element for generic complex vector */
// struct Cplx_Signal {
// 	float * pRe;		//!<- Pointer to real component of vector
// 	float * pIm;		//!<- Pointer to complex component of vector
// 	int length;			//!<- Number of elements in vector
// };

void lab_ofdm_process_qpsk_encode(char * pMessage, float * pDst, int Mlen);
void lab_ofdm_process_qpsk_decode(float * pSrc, char * pMessage,  int Mlen);
void lab_ofdm_process_tx(float * tx_data);
void lab_ofdm_process_rx(float * rx_data);
void lab_ofdm_process_init(void);

#endif /* LAB_OFDM_PROCESS_H_ */
