/*****************************************************************************
 *   @file      audio/acq_audio.c
 *   @brief     audio acquisition code. Note this code just gather audio data 
 *              into RAM, but does not perform any analysis, transformation, or
 *              storage of said data.
 *   @project   Project CETI
 *   @copyright Harvard University Wood Lab
 *   @authors   Michael Salino-Hugg, [TODO: Add other contributors here]
 *****************************************************************************/
#include "acq_audio.h"

#include "led/led_ctl.h"
#include "config.h"

// Middleware
#include "main.h"
#include <app_filex.h>
#include <sai.h>
#include <spi.h>
#include <stm32u5xx_hal.h>
#include <stm32u5xx_hal_sai.h>

// Standard libraries
#include <stdbool.h>

// Supported Hardware
#define AUDIO_CHANNEL_COUNT (AUDIO_CH_0_EN + AUDIO_CH_1_EN + AUDIO_CH_2_EN + AUDIO_CH_3_EN)

#define ADC_AD7768 0
#define AUDIO_ADC_PART_NUMBER ADC_AD7768
#if AUDIO_SAMPLE_BITDEPTH == 24
//#define AUDIO_SAI_CHANNEL_MASK (0b1110)
#define AUDIO_SAI_CHANNEL_MASK (0b1110)
#elif AUDIO_SAMPLE_BITDEPTH == 16
#define AUDIO_SAI_CHANNEL_MASK (0b0110)
#endif

#define AUDIO_SAI_SLOT_MASK (((AUDIO_CH_0_EN * AUDIO_SAI_CHANNEL_MASK) << 0) \
    | ((AUDIO_CH_1_EN * AUDIO_SAI_CHANNEL_MASK) << 4)                          \
    | ((AUDIO_CH_2_EN * AUDIO_SAI_CHANNEL_MASK) << 8)                          \
    | ((AUDIO_CH_3_EN * AUDIO_SAI_CHANNEL_MASK) << 12))

#if AUDIO_ADC_PART_NUMBER == ADC_AD7768
#include "ad7768.h" // this hardware is only used here, so no need for a header
extern SAI_HandleTypeDef hsai_BlockA1;
extern SPI_HandleTypeDef AUDIO_hspi;
#endif

extern DMA_HandleTypeDef handle_GPDMA1_Channel0;

#define AUDIO_CIRCULAR_BUFFER_SIZE (AUDIO_CIRCULAR_BUFFER_SIZE_MAX)

extern FX_MEDIA sdio_disk;

static uint8_t s_audio_enabled = 0;
static uint8_t s_audio_running = 0;
static uint8_t s_audio_circular_buffer[2][AUDIO_CIRCULAR_BUFFER_SIZE];
static uint8_t s_circular_write_block = 0;

/* Retention buffer
 * This is the buffer that holds the data prior to it being processed/pushed to
 * the SDCard
 */
#define RETAIN_BUFFER_BLOCK_TO_HALF(block)                                     \
  ((block) / (RETAIN_BUFFER_SIZE_BLOCKS / 2))
#define AUDIO_WRITE_INTERVAL_S                                                 \
  ((double)(AUDIO_CIRCULAR_BUFFER_SIZE * RETAIN_BUFFER_SIZE_BLOCKS / 2) /      \
   (double)(AUDIO_SAMPLERATE_SPS * (AUDIO_SAMPLE_BITDEPTH / 8) *               \
            AUDIO_CHANNEL_COUNT))

#if RETAIN_BUFFER_SIZE_BLOCKS != 2
static uint8_t s_circular_read_block = 0;
static union {
  uint8_t block[RETAIN_BUFFER_SIZE_BLOCKS]
               [AUDIO_CIRCULAR_BUFFER_SIZE]; // accessed for DMA
  uint8_t half[2][AUDIO_CIRCULAR_BUFFER_SIZE * RETAIN_BUFFER_SIZE_BLOCKS /
                  2]; // access for SD card write
} s_ret_buffer;
static uint8_t s_ret_buffer_write_block = 0;
static uint8_t sd_card_writing = 0;
static uint8_t sd_write_position = 0;
#endif
static uint8_t sd_read_position = 0;

static uint8_t s_audio_capture_overflow = 0;

static AcqAudioLogCallback s_acq_audio_log_callback = NULL;

ad7768_dev audio_adc = {
    .spi_handler = &AUDIO_hspi,
    .spi_cs_port = AUDIO_NCS_GPIO_Output_GPIO_Port,
    .spi_cs_pin = AUDIO_NCS_GPIO_Output_Pin,
    .channel_standby = {
#if AUDIO_CH_0_EN == 1
        .ch[0] = AD7768_ENABLED,
#else
        .ch[0] = AD7768_STANDBY,
#endif
#if AUDIO_CH_1_EN == 1
        .ch[1] = AD7768_ENABLED,
#else 
        .ch[1] = AD7768_STANDBY,
#endif
#if AUDIO_CH_2_EN == 1
        .ch[2] = AD7768_ENABLED,
#else 
        .ch[2] = AD7768_STANDBY,
#endif
#if AUDIO_CH_3_EN == 1
        .ch[3] = AD7768_ENABLED,
#else 
        .ch[3] = AD7768_STANDBY,
#endif
    },
    .channel_mode[AD7768_MODE_A] = {
        .filter_type = AD7768_FILTER_SINC,
#if (AUDIO_SAMPLERATE_SPS == 96000) && (AUDIO_PRIORITY == AUDIO_PRIORITIZE_NOISE)
            .dec_rate = AD7768_DEC_X64,
#elif (AUDIO_SAMPLERATE_SPS == 96000) && (AUDIO_PRIORITY == AUDIO_PRIORITIZE_POWER)
            .dec_rate = AD7768_DEC_X32,
#elif AUDIO_SAMPLERATE_SPS == 192000
            .dec_rate = AD7768_DEC_X32,
#endif
    },
    .channel_mode[AD7768_MODE_B] = {
#if AUDIO_FILTER_TYPE == AUDIO_FILTER_SINC
        .filter_type = AD7768_FILTER_SINC,
#else
		.filter_type = AD7768_FILTER_WIDEBAND,
#endif
#if (AUDIO_SAMPLERATE_SPS == 96000) && (AUDIO_PRIORITY == AUDIO_PRIORITIZE_NOISE)
            .dec_rate = AD7768_DEC_X64,
#elif (AUDIO_SAMPLERATE_SPS == 96000) && (AUDIO_PRIORITY == AUDIO_PRIORITIZE_POWER)
            .dec_rate = AD7768_DEC_X32,
#elif AUDIO_SAMPLERATE_SPS == 192000
            .dec_rate = AD7768_DEC_X32,
#endif
    },
    .channel_mode_select = {
#if AUDIO_CH_0_EN == 1
        .ch[0] = AD7768_MODE_B,
#else 
        .ch[0] = AD7768_MODE_A,
#endif
#if AUDIO_CH_1_EN == 1
        .ch[1] = AD7768_MODE_B,
#else 
        .ch[1] = AD7768_MODE_A,
#endif
#if AUDIO_CH_2_EN == 1
        .ch[2] = AD7768_MODE_B,
#else 
        .ch[2] = AD7768_MODE_A,
#endif
#if AUDIO_CH_3_EN == 1
        .ch[3] = AD7768_MODE_B,
#else 
        .ch[3] = AD7768_MODE_A,
#endif
    },
    .power_mode = {
#if (AUDIO_SAMPLERATE_SPS == 96000) && (AUDIO_PRIORITY == AUDIO_PRIORITIZE_NOISE)
            .sleep_mode = AD7768_ACTIVE,
            .power_mode = AD7768_FAST,
            .lvds_enable = false,
            .mclk_div = AD7768_MCLK_DIV_4,
#elif (AUDIO_SAMPLERATE_SPS == 96000) && (AUDIO_PRIORITY == AUDIO_PRIORITIZE_POWER)
            .sleep_mode = AD7768_ACTIVE,
            .power_mode = AD7768_MEDIAN,
            .lvds_enable = false,
            .mclk_div = AD7768_MCLK_DIV_8,
#elif AUDIO_SAMPLERATE_SPS == 192000
            .sleep_mode = AD7768_ACTIVE,
            .power_mode = AD7768_FAST,
            .lvds_enable = false,
            .mclk_div = AD7768_MCLK_DIV_4,
#endif
        },
    .interface_config = {
#if AUDIO_SAMPLERATE_SPS == 192000
        .crc_select = AD7768_CRC_NONE,
        .dclk_div = AD7768_DCLK_DIV_1,
#elif AUDIO_SAMPLERATE_SPS == 96000
        .crc_select = AD7768_CRC_NONE,
        .dclk_div = AD7768_DCLK_DIV_1,
#endif
        },
    .pin_spi_ctrl = AD7768_SPI_CTRL,
};

#if RETAIN_BUFFER_SIZE_BLOCKS != 2
void audio_retain_completeCallback(DMA_HandleTypeDef *hdma) {
  s_circular_read_block ^= 1;
  // check if retain buffer half full
  if (sd_read_position !=
          RETAIN_BUFFER_BLOCK_TO_HALF(s_ret_buffer_write_block) &&
      !sd_card_writing) {
    // signal sd card write
    sd_card_writing = 1;
    sd_write_position ^= 1;
    // return to main loop so SD card can be written to
    HAL_PWR_DisableSleepOnExit();
  }
  // return to main loop to perform audio SDWriteComplete
}
#endif

void audio_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai) {
#if RETAIN_BUFFER_SIZE_BLOCKS != 2
  HAL_DMA_Start_IT(&handle_GPDMA1_Channel0,
                   (uint32_t)s_audio_circular_buffer[s_circular_write_block],
                   (uint32_t)s_ret_buffer.block[s_ret_buffer_write_block],
                   AUDIO_CIRCULAR_BUFFER_SIZE_MAX);
  s_ret_buffer_write_block =
      (s_ret_buffer_write_block + 1) % RETAIN_BUFFER_SIZE_BLOCKS;
  // check for overflows
  if ((s_circular_write_block ^ 1) == s_circular_read_block) {
    s_audio_capture_overflow = 1;
    HAL_PWR_DisableSleepOnExit();
    return;
  }
  s_circular_write_block ^= 1;

#else
  // check for overflows
  if ((s_circular_write_block ^ 1) == sd_read_position) {
    while (1) {
      ;
    }
  }
  s_circular_write_block ^= 1;
  HAL_PWR_DisableSleepOnExit();
#endif
}

void acq_audio_configure_gpios(void) {

}

void acq_audio_sai_init(void) {
	  hsai_BlockA1.Instance = SAI1_Block_A;
	  hsai_BlockA1.Init.Protocol = SAI_FREE_PROTOCOL;
	  hsai_BlockA1.Init.AudioMode = SAI_MODESLAVE_RX;
	  hsai_BlockA1.Init.DataSize = SAI_DATASIZE_8;
	  hsai_BlockA1.Init.FirstBit = SAI_FIRSTBIT_MSB;
	  hsai_BlockA1.Init.ClockStrobing = SAI_CLOCKSTROBING_FALLINGEDGE;
	  hsai_BlockA1.Init.Synchro = SAI_ASYNCHRONOUS;
	  hsai_BlockA1.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
	  hsai_BlockA1.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
	  hsai_BlockA1.Init.MckOverSampling = SAI_MCK_OVERSAMPLING_DISABLE;
	  hsai_BlockA1.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
	  hsai_BlockA1.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
	  hsai_BlockA1.Init.MckOutput = SAI_MCK_OUTPUT_ENABLE;
	  hsai_BlockA1.Init.MonoStereoMode = SAI_STEREOMODE;
	  hsai_BlockA1.Init.CompandingMode = SAI_NOCOMPANDING;
	  hsai_BlockA1.Init.TriState = SAI_OUTPUT_NOTRELEASED;
	  hsai_BlockA1.Init.PdmInit.Activation = DISABLE;
	  hsai_BlockA1.Init.PdmInit.MicPairsNbr = 1;
	  hsai_BlockA1.Init.PdmInit.ClockEnable = SAI_PDM_CLOCK1_ENABLE;
	  hsai_BlockA1.FrameInit.FrameLength = 128;
	  hsai_BlockA1.FrameInit.ActiveFrameLength = 1;
	  hsai_BlockA1.FrameInit.FSDefinition = SAI_FS_STARTFRAME;
	  hsai_BlockA1.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
	  hsai_BlockA1.FrameInit.FSOffset = SAI_FS_FIRSTBIT;
	  hsai_BlockA1.SlotInit.FirstBitOffset = 0;
	  hsai_BlockA1.SlotInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
	  hsai_BlockA1.SlotInit.SlotNumber = 16;
	  hsai_BlockA1.SlotInit.SlotActive = AUDIO_SAI_SLOT_MASK;
	  if (HAL_SAI_Init(&hsai_BlockA1) != HAL_OK)
	  {
	    Error_Handler();
	  }
}

int acq_audio_init(void) {
  if (s_audio_enabled) {
    // nothing to do
    return 0;
  }

  // configure MCU hardware for serial audio interface
  acq_audio_sai_init();
  MX_SPI1_Init();

  /* turn on power to audio front-end */
  HAL_GPIO_WritePin(AUDIO_VP_EN_GPIO_Output_GPIO_Port, AUDIO_VP_EN_GPIO_Output_Pin, GPIO_PIN_RESET);
  HAL_Delay(200);
  HAL_GPIO_WritePin(Audio_VN_NEN_GPIO_Output_GPIO_Port, Audio_VN_NEN_GPIO_Output_Pin, GPIO_PIN_RESET);
  HAL_Delay(200);
  HAL_GPIO_WritePin(AUDIO_VP_EN_GPIO_Output_GPIO_Port, AUDIO_VP_EN_GPIO_Output_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AUDIO_NRST_GPIO_Output_GPIO_Port, AUDIO_NRST_GPIO_Output_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AUDIO_NCS_GPIO_Output_GPIO_Port, AUDIO_NCS_GPIO_Output_Pin, GPIO_PIN_RESET);

  // ToDo: setup MCU to handle audio
  // Configure external hardware (ADC)
  int result = ad7768_setup(&audio_adc);

  // after ad7768 is configured we no longer need the spi peripheral until we
  // shut it down
  HAL_SPI_DeInit(&AUDIO_hspi);
  __HAL_RCC_SPI3_CLK_DISABLE();

  // ToDo: recofigure spi_gpio as analog

  // Dummy delay
  HAL_Delay(1000);

  // Register callbacks
  HAL_SAI_RegisterCallback(&hsai_BlockA1, HAL_SAI_RX_HALFCOMPLETE_CB_ID, audio_SAI_RxCpltCallback);
  HAL_SAI_RegisterCallback(&hsai_BlockA1, HAL_SAI_RX_COMPLETE_CB_ID, audio_SAI_RxCpltCallback);
#if RETAIN_BUFFER_SIZE_BLOCKS != 2
  result |= HAL_DMA_RegisterCallback(&handle_GPDMA1_Channel0, HAL_DMA_XFER_CPLT_CB_ID, audio_retain_completeCallback);
#endif

  s_audio_enabled = 1;
  return result;
}

void acq_audio_disable(void) {
  // Deregister callbacks
  HAL_DMA_UnRegisterCallback(&handle_GPDMA1_Channel0, HAL_DMA_XFER_CPLT_CB_ID);
  HAL_SAI_UnRegisterCallback(&hsai_BlockA1, HAL_SAI_RX_HALFCOMPLETE_CB_ID);
  HAL_SAI_UnRegisterCallback(&hsai_BlockA1, HAL_SAI_RX_COMPLETE_CB_ID);

  // ToDo: Deconfigure hardware

  // ToDo: Disable unused MCU resources
  HAL_SAI_DeInit(&hsai_BlockA1);

  /* turn off power to audio front-end */
  HAL_GPIO_WritePin(AUDIO_VP_EN_GPIO_Output_GPIO_Port,
                    AUDIO_VP_EN_GPIO_Output_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(Audio_VN_NEN_GPIO_Output_GPIO_Port,
                    Audio_VN_NEN_GPIO_Output_Pin, GPIO_PIN_SET);

  s_audio_enabled = 0;
}

void acq_audio_start(void) {
    if(s_audio_running) {
        return;
    }
    // initiate transfers
    HAL_SAI_Receive_DMA(&hsai_BlockA1, s_audio_circular_buffer[0], 2 * AUDIO_CIRCULAR_BUFFER_SIZE);
    s_audio_running = 1;

}

void acq_audio_stop(void) {
    HAL_SAI_DMAStop(&hsai_BlockA1);
    s_audio_running = 0;
}

void acq_audio_set_log_callback(AcqAudioLogCallback cb) {
  s_acq_audio_log_callback = cb;
}

void acq_audio_task(void) {
  if (s_audio_capture_overflow) {
    led_error();
  }

  if (sd_read_position != sd_write_position) {
    if (s_acq_audio_log_callback != NULL) {
      uint8_t *pData;
      size_t data_size;
#if RETAIN_BUFFER_SIZE_BLOCKS != 2
      pData = s_ret_buffer.half[sd_read_position];
      data_size = AUDIO_CIRCULAR_BUFFER_SIZE * RETAIN_BUFFER_SIZE_BLOCKS / 2;
#else
      pData = s_audio_circular_buffer[sd_read_position];
      data_size = AUDIO_CIRCULAR_BUFFER_SIZE;
#endif
      s_acq_audio_log_callback(pData, data_size);
    }
    sd_card_writing = 0;
    sd_read_position = sd_write_position;
  }
}
