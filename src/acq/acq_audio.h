/*****************************************************************************
 *   @file   acq/acq_audio.h
 *   @brief  audio acquisition code. Note this code just gather audio data 
 *           into RAM, but does not perform any analysis, transformation, or
 *           storage of said data.
 *   @author Michael Salino-Hugg (msalinohugg@seas.harvard.edu)
 *****************************************************************************/
#ifndef CETI_ACQ_AUDIO_H
#define CETI_ACQ_AUDIO_H
// libraries
#include <stddef.h>
#include <stdint.h>

// definitions
#define AUDIO_CIRCULAR_BUFFER_SIZE_MAX (UINT16_MAX/2)
#define AUDIO_CIRCULAR_BUFFER_SIZE  (AUDIO_CIRCULAR_BUFFER_SIZE_MAX)

#define RETAIN_BUFFER_SIZE_BLOCKS 64

// funcitons
void acq_audio_disable(void);
void acq_audio_enable(void);
void acq_audio_get_flushable_region(uint8_t **ppData, size_t *pSize);
void acq_audio_flush(void);

#endif // CETI_ACQ_AUDIO_H
