//-----------------------------------------------------------------------------
// Project: CETI Tag Electronics
// Copyright: Harvard University Wood Lab
// Contributors: Michael Salino-Hugg, [TODO: Add other contributors here]
//-----------------------------------------------------------------------------

/* Global & HAL includes */
#include <main.h>
#include <gpio.h>
#include <gpdma.h>
#include <icache.h>
#include <i2c.h>
#include <rtc.h>
#include <sai.h>
#include <sdmmc.h>
#include <spi.h>
#include <usb_otg.h>
#include <app_filex.h>

/* Library Includes */
#include "tusb.h"

/* Local Includes */
#include "acq/acq_ecg.h"
#include "acq/acq_pressure.h"
#include "log/log_audio.h"
#include "led.h"
#include "mission.h"
#include "usb/usb.h"
#include "util/timing.h"
#include "version.h"
#include "device/max17320.h"
//#include "cmd/cmd_bms.h"

#include "log/log_syslog.h"

void SystemClock_Config(void);

FX_MEDIA sdio_disk = {}; //struct must be initialized or invalid pointers exist
ALIGN_32BYTES (uint32_t fx_sd_media_memory[FX_STM32_SD_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)]);



/**
  * @brief Power Configuration
  * @retval None
  */
static void SystemPower_Config(void)
{
  HAL_PWREx_EnableVddIO2();

  /*
   * Disable the internal Pull-Up in Dead Battery pins of UCPD peripheral
   */
  HAL_PWREx_DisableUCPDDeadBattery();

  /*
   * Switch to SMPS regulator instead of LDO
   */
  if (HAL_PWREx_ConfigSupply(PWR_SMPS_SUPPLY) != HAL_OK)
  {
    Error_Handler();
  }
/* USER CODE BEGIN PWR */
/* USER CODE END PWR */
}

int cmd_bms_verify(void);
int cmd_bms_program_nonvolatile_memory(void);

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin) {
		case KELLER_DRDY_EXTI9_Pin: // 9
			acq_pressure_EXTI_cb();
			break;
		default:
			__NOP();
			break;
	}
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin) {
		case ECG_ADC_NDRDY_GPIO_Input_Pin: // 2
			acq_ecg_EXTI_Callback();
			break;
		case IMU_NINT_GPIO_Input_Pin: //10
			// ToDo: implement IMU data ready interrupt
			break;
		default:
			__NOP();
			break;
	}
}

int main(void) { 
    HAL_Init();
    SystemClock_Config();
    SystemPower_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_GPDMA1_Init();
    MX_ICACHE_Init();

    MX_RTC_Init();
    
    MX_I2C1_Init(); // pressure sensor
    MX_I2C3_Init(); // BMS / LEDs
    
    led_init();
    led_idle();
        
    /* open SD card for system logging */
    MX_SDMMC1_SD_Init();
    MX_FileX_Init();
        int filex_status = fx_media_open(&sdio_disk, "", fx_stm32_sd_driver, (VOID *)FX_NULL, (VOID *) fx_sd_media_memory, sizeof(fx_sd_media_memory));
        if(filex_status == FX_SUCCESS) {
        syslog_init();
        CETI_LOG("Program started!");
    }
    
    
    /* Detect if the external interface is present to enable USB for offload/debug/DFU */
    if (usb_iface_present()) {
        CETI_LOG("Key detected. Starting USB Device Interface");
      
        /* initialize USB hardware */
        // ToDo: reconfigure gpio for USB here leave USB gpio as high impedence analog in normal config
        __HAL_RCC_SYSCFG_CLK_ENABLE();
        MX_USB_OTG_HS_PCD_Init(); // setup HS USB Hardware
        
        
        // IOSV bit MUST be set to access GPIO port G[2:15] */
        HAL_PWREx_EnableVddIO2();
        
        /* USB clock enable */
        __HAL_RCC_USB_OTG_HS_CLK_ENABLE();
        __HAL_RCC_USBPHYC_CLK_ENABLE();
        
        /* Enable USB power on Pwrctrl CR2 register */
        HAL_PWREx_EnableVddUSB();
        HAL_PWREx_EnableUSBHSTranceiverSupply();
        
        /*Configuring the SYSCFG registers OTG_HS PHY*/
        HAL_SYSCFG_EnableOTGPHY(SYSCFG_OTG_HS_PHY_ENABLE);
        
        // Disable VBUS sense (B device)
        USB_OTG_HS->GCCFG &= ~USB_OTG_GCCFG_VBDEN;
        
        // B-peripheral session valid override enable
        USB_OTG_HS->GCCFG |= USB_OTG_GCCFG_VBVALEXTOEN;
        USB_OTG_HS->GCCFG |= USB_OTG_GCCFG_VBVALOVAL;
        HAL_Delay(100);
        usb_init(); // initialize tiny usb library
        
        /* main loop while in USB */
        while (usb_iface_present()) {
            // ToDo: update leds for usb
            tud_task(); // usb device task
            if (tud_mounted()){
              CETI_LOG("usb IS mounted");
            }
            // ToDo: react to incoming CDC requests
            // ToDo: sleep?
        }
        
        /* reboot system to return to capture state once key is removed */
        NVIC_SystemReset();
    }
      
    /* perform runtime system hardware test to detect available systems */
    CETI_LOG("Initializing BMS");


    CETI_LOG("Enabling VHF Pinger");


      
    /* enable Audio -5V */
    CETI_LOG("Initializing Audio");
    log_audio_enable();
    led_heartbeat();
    
    CETI_LOG("Initializing Pressure");
    // GPS pulls entire bus low if not powered
    HAL_GPIO_WritePin(GPS_PWR_EN_GPIO_Output_GPIO_Port, GPS_PWR_EN_GPIO_Output_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPS_NRST_GPIO_Output_GPIO_Port, GPS_NRST_GPIO_Output_Pin, GPIO_PIN_SET);
    acq_pressure_enable();

    CETI_LOG("Initializing IMU");

    CETI_LOG("Initializing ECG");
    MX_I2C2_Init(); // ECG ADC
    acq_ecg_enable();

    CETI_LOG("Initializing GPS");

    CETI_LOG("Initializing ARGOS");

    CETI_LOG("Enabling Antenna Flasher");



    mission_set_state(MISSION_STATE_SURFACE);
    while(1){
        mission_task();
    	/* List of possible tasks */
        // log_audio_task();
    	// Flush Audio Buffer
    	// Empty
    	// Flush Battery
    	// Flush Pressure Buffer
        // Flush IMU Buffer
    	// Flush GPS Buffer
    	// Flush ECG Buffer
    	// Message Position via APRS
    	// Update State Machine
    }

    // we should never get here 
    return -1;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
//  blink_interval_ms = BLINK_MOUNTED;
	CETI_LOG("USB mounted");
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
	CETI_LOG("USB unmounted");
//  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
//  (void) remote_wakeup_en;
//  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
//  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}
