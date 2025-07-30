#include "stm32u5xx_hal.h"

#define KEY_PORT (0)
#define KEY_PIN (0)

static void mission_predeploy_enable_key_wakeup(void){
    // GPIO_InitTypeDef gpioInitStruct = {0};

    // gpioInitStruct.Mode = GPIO_MODE_INPUT;
    // gpioInitStruct.Pull = GPIO_PULLDOWN;
    // gpioInitStruct.Speed = GPIO_SPEED_LOW;

    // HW_GPIO_Init( KEY_PORT, KEY_PIN, &gpioInitStruct );

    // LL_EXTI_InitTypeDef EXTIinitStruct = {0};
    // // EXTIinitStruct.Line_0_31 = LL_EXTI_LINE_10;
    // EXTIinitStruct.LineCommand = ENABLE;
    // EXTIinitStruct.Mode = LL_EXTI_MODE_IT_EVENT;
    // EXTIinitStruct.Trigger = LL_EXTI_TRIGGER_RISING_FALLING;
    // LL_EXTI_Init(&EXTIinitStruct);
}

// Predeployment is the time prior to the tag being attached to a whale
// Main functions:
// 1) monitor "key" voltage to enter offload/interface modes
// 2) monitor depth sensor for detecting deployment
// 3) GPS for timesynch (?)
//void mission_predeploy_enable(void) {
//    // enable bms_log
//    // enable depth_log
//}


//void mission_predeploy_update(void) {
//    // ToDo: sleep until interrupt
//    /*
//    HAL_SuspendTick();
//	__disable_irq();
//	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
//	// code to execute prior to jumping to IRQ here
//	__enable_irq();
//	__HAL_RCC_GPIOC_CLK_ENABLE();
//    __HAL_RCC_GPIOD_CLK_ENABLE(); // sdmmc,
//	HAL_ResumeTick();
//    */
//
//    // ToDo: if (new_depth_sample)
//
//
//    // ToDo: store collected data if needed
//    int key_present = 0;
//    int should_dive = 0;
//    if (key_present) {
//        // ToDo: transistion to USB device if key if connected
//    } else if (should_dive) {
//        // ToDo: transistion to dive state if depth > ???
//    }
//
//
//}
