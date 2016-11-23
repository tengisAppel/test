/** @brief Implements all source blocks */

#ifndef SOURCES_H_
#define SOURCES_H_

/** @brief Peak magnitude of amplitude of mystery signal */
#define DIST_MAXAMP 0.1f

/** @brief Updates all internally-generated time-varying signals
 * To be called by the backend subsystem only. */
void blocks_sources_update(void);

/** @brief Writes the AUDIO_BLOCKSIZE oldest ready mystery signal samples to sample_block.
 * @param sample_block Pointer to array of AUDIO_BLOCKSIZE floats to write
 * oldest unread mystery signal samples to, scaled to the range
 * [-DIST_MAXAMP, DIST_MAXAMP]. (Note that this is done as it allows for
 * conveniently directly outputting the signal to the speakers at reasonable
 * volume levels and allows the LMS filter some headroom in its'calculated
 * output amplitudes) */
void blocks_sources_disturbance(float * sample_block);

/** @brief Gets the AUDIO_BLOCKSIZE oldest unread microphone samples
 * @return Pointer to the AUDIO_BLOCKSIZE oldest unread microphone samples,
 * scaled to the range [-1, 1]. */
void blocks_sources_microphone(float * sample_block);

/** @brief Gets the AUDIO_BLOCKSIZE next samples from the stored waveform */
void blocks_sources_waveform(float * sample_block);

/** @brief Sets the desired frequency for the sine/cosine sources */
void blocks_sources_trig_setfreq(float frequency);

/** @brief Gets the AUDIO_BLOCKSIZE next samples for a sine of set frequency
 * Will default to a 1kHz signal if no frequency has been set. */
void blocks_sources_sin(float * sample_block);

/** @brief Gets the AUDIO_BLOCKSIZE next samples for a cosine of set frequency
 * Will default to a 1kHz signal if no frequency has been set. */
void blocks_sources_cos(float * sample_block);

/** @brief Gets a blocks of AUDIO_BLOCKSIZE zeros */
void blocks_sources_zeros(float * sample_block);

/** @brief Gets a blocks of AUDIO_BLOCKSIZE ones */
void blocks_sources_ones(float * sample_block);

#endif /* SOURCES_H_ */
