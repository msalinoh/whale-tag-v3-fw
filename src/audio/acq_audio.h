/*****************************************************************************
 *   @file      acq_audio.h
 *   @brief     audio acquisition code. Note this code just gather audio data 
 *              into RAM, but does not perform any analysis, transformation, or
 *              storage of said data.
 *   @project   Project CETI
 *   @copyright Harvard University Wood Lab
 *   @authors   Michael Salino-Hugg, [TODO: Add other contributors here]
 *****************************************************************************/
#ifndef CETI_ACQ_AUDIO_H
#define CETI_ACQ_AUDIO_H
// libraries
#include <stddef.h>
#include <stdint.h>

// definitions
#define AUDIO_SAMPLE_RATE 96000
#define AUDIO_BITDEPTH 24
#define AUDIO_CHANNEL_COUNT 4

#define AUDIO_CIRCULAR_BUFFER_SIZE_MAX (UINT16_MAX/2)
#define AUDIO_CIRCULAR_BUFFER_SIZE  (AUDIO_CIRCULAR_BUFFER_SIZE_MAX)

#define RETAIN_BUFFER_SIZE_BLOCKS 64


typedef int (* AcqAudioLogCallback)(uint8_t *pData, uint32_t size);

// funcitons
void acq_audio_disable(void);
void acq_audio_enable(void);
void acq_audio_set_log_callback(AcqAudioLogCallback cb);
void acq_audio_task(void);

#endif // CETI_ACQ_AUDIO_H
