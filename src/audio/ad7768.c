/***************************************************************************//**
 *   @file      audio/ad7768.c
 *   @brief     Implementation of AD7768-4 Driver.
 *   @project   Project CETI
 *   @copyright Harvard University Wood Lab
 *   @authors   Michael Salino-Hugg, [TODO: Add other contributors here]
 *****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "ad7768.h"
#include <spi.h>

// #define AD7768_DEBUG

#ifdef AD7768_DEBUG
#include "log/log_syslog.h"
#endif
// #include "util.h"
//#include "audio.h"

const uint8_t standard_pin_ctrl_mode_sel[3][4] = {
//		MCLK/1,	MCLK/2,	MCLK/4,	MCLK/8
	{0x0,	0x1,	0x2,	0x3},	// Eco
	{0x4,	0x5,	0x6,	0x7},	// Median
	{0x8,	0x9,	0xA,	0xB},	// Fast
};

const uint8_t one_shot_pin_ctrl_mode_sel[3][4] = {
//		MCLK/1,	MCLK/2,	MCLK/4,	MCLK/8
	{0xC,	0xFF,	0xFF,	0xFF},	// Eco
	{0xD,	0xFF,	0xFF,	0xFF},	// Median
	{0xF,	0xE,	0xFF,	0xFF},	// Fast
};

#define _RSHIFT(x, s, w) (((x) >> s) & ((1 << w) - 1))
#define _LSHIFT(x, s, w) (((x) & ((1 << w) - 1)) << s)
/*********************
 * Private Functions *
 *********************/
static inline const ad7768_Reg_ChStandby __reg_channelStandby_fromRaw(const uint8_t raw) {
	return (ad7768_Reg_ChStandby){
        .ch[0] = _RSHIFT(raw, 0, 1), 
        .ch[1] = _RSHIFT(raw, 1, 1), 
        .ch[2] = _RSHIFT(raw, 2, 1), 
        .ch[3] = _RSHIFT(raw, 3, 1),
	};
}

static inline const uint8_t __reg_channelStandby_intoRaw(const ad7768_Reg_ChStandby *reg) {
	return _LSHIFT(reg->ch[0], 0, 1)  
		| _LSHIFT(reg->ch[1], 1, 1)  
		| _LSHIFT(reg->ch[2], 2, 1)  
		| _LSHIFT(reg->ch[3], 3, 1);
}

static inline const ad7768_Reg_ChMode __reg_channelMode_fromRaw(const uint8_t raw) {
	return (ad7768_Reg_ChMode){
		.filter_type = _RSHIFT(raw, 3, 1),
		.dec_rate = _RSHIFT(raw, 0, 3)
	};
}

static inline const uint8_t __reg_channelMode_intoRaw(const ad7768_Reg_ChMode *reg) {
	return _LSHIFT(reg->filter_type, 3, 1) 
		| _LSHIFT(reg->dec_rate, 0, 3);
}

static inline const ad7768_Reg_ChModeSelect __reg_channelModeSelect_fromRaw(const uint8_t raw) {
	return (ad7768_Reg_ChModeSelect){
		.ch = {
			[0] = _RSHIFT(raw, 0, 1), 
			[1] = _RSHIFT(raw, 1, 1), 
			[2] = _RSHIFT(raw, 2, 1), 
			[3] = _RSHIFT(raw, 3, 1)
		}
	};
}

static inline const uint8_t __reg_channelModeSelect_intoRaw(const ad7768_Reg_ChModeSelect *reg) {
	return _LSHIFT(reg->ch[0], 0, 1)  
		| _LSHIFT(reg->ch[1], 1, 1)  
		| _LSHIFT(reg->ch[2], 2, 1)  
		| _LSHIFT(reg->ch[3], 3, 1)
		| _LSHIFT(reg->ch[2], 4, 1)  
		| _LSHIFT(reg->ch[3], 5, 1);
}

static inline const ad7768_Reg_PowerMode __reg_powerMode_fromRaw(const uint8_t raw) {
	return (ad7768_Reg_PowerMode){
			.sleep_mode 	= _RSHIFT(raw, 7, 1),
			.power_mode 	= _RSHIFT(raw, 4, 2),
			.lvds_enable	= _RSHIFT(raw, 3, 1),
			.mclk_div 		= _RSHIFT(raw, 0, 2),
	};
}

static inline const uint8_t __reg_powerMode_intoRaw(const ad7768_Reg_PowerMode *reg) {
	return _LSHIFT(reg->sleep_mode, 7, 1)
		| _LSHIFT(reg->power_mode, 4, 2)
		| _LSHIFT(reg->lvds_enable, 3, 1)
		| _LSHIFT(reg->mclk_div, 0, 2);
}

static inline const ad7768_Reg_GeneralCfg __reg_generalCfg_fromRaw(const uint8_t raw){
	return (ad7768_Reg_GeneralCfg){
		.retime_en 	= _RSHIFT(raw, 5, 1),
		.vcm_pd 	= _RSHIFT(raw, 4, 1),
		.vcm_vsel 	= _RSHIFT(raw, 0, 2),
	};
}

static inline const uint8_t __reg_generalCfg_intoRaw(const ad7768_Reg_GeneralCfg *reg){
	return  _LSHIFT(reg->retime_en, 5, 1)
		| _LSHIFT(reg->vcm_pd, 4, 1)
		| _LSHIFT(reg->vcm_vsel, 0, 2);
}

static inline const ad7768_Reg_DataControl __reg_dataControl_fromRaw(const uint8_t raw){
	return (ad7768_Reg_DataControl){
		.spi_sync 		= _RSHIFT(raw, 7, 1),
		.single_shot_en = _RSHIFT(raw, 4, 1),
		.spi_reset 		= _RSHIFT(raw, 0, 2),
	};
}

static inline const uint8_t __reg_dataControl_intoRaw(const ad7768_Reg_DataControl *reg){
	return  _LSHIFT(reg->spi_sync, 7, 1)
  	| _LSHIFT(reg->single_shot_en, 4, 1)
  	| _LSHIFT(reg->spi_reset, 0, 2);
}

static inline const ad7768_Reg_InterfaceCfg __reg_interfaceCfg_fromRaw(const uint8_t raw){
	return (ad7768_Reg_InterfaceCfg){
		.crc_select = _RSHIFT(raw, 2, 2),
		.dclk_div 	= _RSHIFT(raw, 0, 2),
	};
}

static inline const uint8_t __reg_interfaceCfg_intoRaw(const ad7768_Reg_InterfaceCfg *reg){
	return  _LSHIFT(reg->crc_select, 2, 2)
  	| _LSHIFT(reg->dclk_div, 0, 2);
}

static inline const ad7768_Reg_BISTControl __reg_bistControl_fromRaw(const uint8_t raw){
	return (ad7768_Reg_BISTControl){
		.ram_bist_start = _RSHIFT(raw, 0, 1)
	};
}

static inline const uint8_t __reg_bistControl_intoRaw(const ad7768_Reg_BISTControl *reg){
	return _LSHIFT(reg->ram_bist_start, 0, 1);
}

static inline const ad7768_Reg_DeviceStatus __reg_deviceStatus_fromRaw(const uint8_t raw){
	return (ad7768_Reg_DeviceStatus){
        .chip_error       = _RSHIFT(raw, 3, 1),
        .no_clock_error   = _RSHIFT(raw, 2, 1),
        .ram_bist_pass    = _RSHIFT(raw, 1, 1),
        .ram_bist_running = _RSHIFT(raw, 0, 1),
	};
}

static inline void prv_ad7768_spi_select(ad7768_dev *dev){
	HAL_GPIO_WritePin(dev->spi_cs_port, dev->spi_cs_pin, GPIO_PIN_RESET);
}

static inline void prv_ad7768_spi_deselect(ad7768_dev *dev){
	HAL_GPIO_WritePin(dev->spi_cs_port, dev->spi_cs_pin, GPIO_PIN_SET);
}

/********************
 * Public Functions *
 ********************/

/**
 * SPI read from device.
 * @param dev - The device structure.
 * @param reg_addr - The register address.
 * @param reg_data - The register data.
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_spi_read(ad7768_dev *dev, uint8_t reg_addr, uint8_t *reg_data) {
	uint8_t tx_buf[2] = {AD7768_SPI_READ(reg_addr), 0};
	uint8_t rx_buf[2];
	HAL_StatusTypeDef ret;

	prv_ad7768_spi_select(dev);
	ret = HAL_SPI_Transmit(dev->spi_handler, (uint8_t*)tx_buf, 2, ADC_TIMEOUT);
    prv_ad7768_spi_deselect(dev);
    HAL_Delay(1); //guarentee > needs 100 ns wait
    prv_ad7768_spi_select(dev);
    ret |= HAL_SPI_TransmitReceive(dev->spi_handler, (uint8_t*)&tx_buf, (uint8_t*)&rx_buf, 2, ADC_TIMEOUT);
    prv_ad7768_spi_deselect(dev);

	*reg_data = rx_buf[1];

	#ifdef AD7768_DEBUG
	CETI_LOG("READ:{ addr: %02xh, value: %02xh} via spi", reg_addr, *reg_data);
	#endif

	return ret;
}

/**
 * SPI write to device.
 * @param dev - The device structure.
 * @param reg_addr - The register address.
 * @param reg_data - The register data.
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_spi_write(ad7768_dev *dev, uint8_t reg_addr, uint8_t reg_data) {
    HAL_StatusTypeDef ret;
	uint8_t buf[2] = {AD7768_SPI_WRITE(reg_addr), reg_data};
	prv_ad7768_spi_select(dev);
	ret = HAL_SPI_Transmit(dev->spi_handler, (uint8_t *)&buf, 2, ADC_TIMEOUT);
	prv_ad7768_spi_deselect(dev);
#ifdef AD7768_DEBUG
	CETI_LOG("WROTE:{ addr: %02xh, value: %02xh} via spi", buf[0], buf[1]);
	uint8_t result;
	ad7768_spi_read(dev, reg_addr, &result);
	if (result == reg_data) {
		CETI_LOG("Write Verified");
	} else {
		CETI_ERR("Write not verified");
	}
#endif
	return ret;
}

/**
 * Set the device sleep mode.
 * @param dev - The device structure.
 * @param mode - The device sleep mode.
 * 				 Accepted values: AD7768_ACTIVE
 * 								  AD7768_SLEEP
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_set_sleep_mode(ad7768_dev *dev, ad7768_sleep_mode mode){
	HAL_StatusTypeDef ret = HAL_OK;
	uint8_t reg_data;

    ret |= ad7768_spi_read(dev, AD7768_REG_PWR_MODE, &reg_data);
    dev->power_mode = __reg_powerMode_fromRaw(reg_data);
    if(dev->power_mode.sleep_mode != mode){
	    dev->power_mode.sleep_mode = mode;
	    ret |= ad7768_spi_write(dev, AD7768_REG_PWR_MODE, __reg_powerMode_intoRaw(&dev->power_mode));
    }

	return ret;
}

/**
 * Get the device sleep mode.
 * @param dev - The device structure.
 * @param mode - The device sleep mode.
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_get_sleep_mode(ad7768_dev *dev, ad7768_sleep_mode *mode) {
	*mode = dev->power_mode.sleep_mode;

	return HAL_OK;
}


/**
 * Set the device power mode.
 * @param dev - The device structure.
 * @param mode - The device power mode.
 * 				 Accepted values: AD7768_ECO
 *								  AD7768_MEDIAN
 *								  AD7768_FAST
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_set_power_mode(ad7768_dev *dev, ad7768_power_mode mode) {
	HAL_StatusTypeDef ret = HAL_OK;

	if (dev->pin_spi_ctrl == AD7768_SPI_CTRL) {
		dev->power_mode.power_mode = mode;
		ret = ad7768_spi_write(dev, AD7768_REG_PWR_MODE, __reg_powerMode_intoRaw(&dev->power_mode));
	}
	return ret;
}

/**
 * Get the device power mode.
 * @param dev - The device structure.
 * @param mode - The device power mode.
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_get_power_mode(ad7768_dev *dev, ad7768_power_mode *mode) {
	*mode = dev->power_mode.power_mode;

	return 0;
}

/**
 * Set the MCLK divider.
 * @param dev - The device structure.
 * @param clk_div - The MCLK divider.
 * 					Accepted values: AD7768_MCLK_DIV_32
 *									 AD7768_MCLK_DIV_8
 *									 AD7768_MCLK_DIV_4
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_set_mclk_div(ad7768_dev *dev, ad7768_mclk_div clk_div) {
	HAL_StatusTypeDef ret = HAL_OK;
    uint8_t raw;

    ret |= ad7768_spi_read(dev, AD7768_REG_PWR_MODE, &raw);
    dev->power_mode = __reg_powerMode_fromRaw(raw);
    if(dev->power_mode.mclk_div != clk_div){
        dev->power_mode.mclk_div = clk_div;
        ret |= ad7768_spi_write(dev, AD7768_REG_PWR_MODE, __reg_powerMode_intoRaw(&dev->power_mode));
    }

	return ret;
}

/**
 * Get the MCLK divider.
 * @param dev - The device structure.
 * @param clk_div - The MCLK divider.
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_get_mclk_div(ad7768_dev *dev, ad7768_mclk_div *clk_div) {
	*clk_div = dev->power_mode.mclk_div;

	return HAL_OK;
}

/**
 * Set the DCLK divider.
 * @param dev - The device structure.
 * @param clk_div - The DCLK divider.
 * 					Accepted values: AD7768_DCLK_DIV_1
 *									 AD7768_DCLK_DIV_2
 *									 AD7768_DCLK_DIV_4
 *									 AD7768_DCLK_DIV_8
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_set_dclk_div(ad7768_dev *dev, ad7768_dclk_div clk_div) {
	HAL_StatusTypeDef ret = HAL_OK;
    uint8_t raw;

	if (dev->pin_spi_ctrl != AD7768_SPI_CTRL) {
        return HAL_ERROR;
    }
		
    ret |= ad7768_spi_read(dev, AD7768_REG_INTERFACE_CFG, &raw);
    dev->interface_config = __reg_interfaceCfg_fromRaw(raw);
    if(dev->interface_config.dclk_div != clk_div){
        dev->interface_config.dclk_div = clk_div;
        ret |= ad7768_spi_write(dev, AD7768_REG_INTERFACE_CFG, __reg_interfaceCfg_intoRaw(&dev->interface_config));
    }

	return ret;
}

/**
 * Get the DCLK divider.
 * @param dev - The device structure.
 * @param clk_div - The DCLK divider.
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_get_dclk_div(ad7768_dev *dev, ad7768_dclk_div *clk_div) {
	*clk_div = dev->interface_config.dclk_div;

	return 0;
}


/**
 * Set the conversion operation mode.
 * @param dev - The device structure.
 * @param conv_op - The conversion operation mode.
 * 					Accepted values: AD7768_STANDARD_CONV
 * 									 AD7768_ONE_SHOT_CONV
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_set_conv_op(ad7768_dev *dev, ad7768_conv_op conv_op){
	HAL_StatusTypeDef ret = HAL_OK;
    uint8_t raw;

	if (dev->pin_spi_ctrl != AD7768_SPI_CTRL) {
        return HAL_ERROR;
    }

    ret |= ad7768_spi_read(dev, AD7768_REG_DATA_CTRL, &raw);
    dev->data_control = __reg_dataControl_fromRaw(raw);
    if(dev->data_control.single_shot_en != conv_op){
        dev->data_control.single_shot_en = conv_op;
        ret |= ad7768_spi_write(dev, AD7768_REG_DATA_CTRL, __reg_dataControl_intoRaw(&dev->data_control));
    }
    return ret;
}

/**
 * Get the conversion operation mode.
 * @param dev - The device structure.
 * @param conv_op - The conversion operation mode.
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_get_conv_op(ad7768_dev *dev, ad7768_conv_op *conv_op) {
	*conv_op = dev->data_control.single_shot_en;

	return 0;
}

/**
 * Set the CRC selection.
 * @param dev - The device structure.
 * @param crc_sel - The CRC selection.
 * 					Accepted values: AD7768_NO_CRC
 * 									 AD7768_CRC_4
 * 									 AD7768_CRC_16
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_set_crc_sel(ad7768_dev *dev, ad7768_crc_sel crc_sel) {
	HAL_StatusTypeDef ret = HAL_OK;
    uint8_t raw;

    ret |= ad7768_spi_read(dev, AD7768_REG_INTERFACE_CFG, &raw);
    dev->interface_config = __reg_interfaceCfg_fromRaw(raw);
    
    if(dev->interface_config.crc_select != crc_sel){
        dev->interface_config.crc_select = crc_sel;
        ret |= ad7768_spi_write(dev, AD7768_REG_INTERFACE_CFG, __reg_interfaceCfg_intoRaw(&dev->interface_config));
    }

	return ret;
}

/**
 * Get the CRC selection.
 * @param dev - The device structure.
 * @param crc_sel - The CRC selection.
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_get_crc_sel(ad7768_dev *dev, ad7768_crc_sel *crc_sel) {
	*crc_sel = dev->interface_config.crc_select;

	return 0;
}

/**
 * Set the channel state.
 * @param dev - The device structure.
 * @param ch - The channel number.
 * 			   Accepted values: AD7768_CH0
 * 			   					AD7768_CH1
 * 			   					AD7768_CH2
 * 			   					AD7768_CH3
 * @param state - The channel state.
 * 				  Accepted values: AD7768_ENABLED
 * 								   AD7768_STANDBY
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_set_ch_state(ad7768_dev *dev, ad7768_ch ch, ad7768_ch_state state) {
	HAL_StatusTypeDef ret = HAL_OK;
    uint8_t raw;

    ret |= ad7768_spi_read(dev, AD7768_REG_CH_STANDBY, &raw);
    dev->channel_standby = __reg_channelStandby_fromRaw(raw);
    if(dev->channel_standby.ch[ch] != state){
        dev->channel_standby.ch[ch] = state;
        ret |= ad7768_spi_write(dev, AD7768_REG_CH_STANDBY, __reg_channelStandby_intoRaw(&dev->channel_standby));
    }

	return ret;
}

/**
 * Get the channel state.
 * @param dev - The device structure.
 * @param ch - The channel number.
 * 			   Accepted values: AD7768_CH0
 * 			   					AD7768_CH1
 * 			   					AD7768_CH2
 * 			   					AD7768_CH3
 * @param state - The channel state.
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_get_ch_state(ad7768_dev *dev, ad7768_ch ch, ad7768_ch_state *state) {
	*state = dev->channel_standby.ch[ch];

	return HAL_OK;
}

/**
 * Set the mode configuration.
 * @param dev - The device structure.
 * @param mode - The channel mode.
 * 				 Accepted values: AD7768_MODE_A
 * 								  AD7768_MODE_B
 * @param filt_type - The filter type.
 * 					  Accepted values: AD7768_FILTER_WIDEBAND
 * 					  				   AD7768_FILTER_SINC,
 * @param dec_rate - The decimation rate.
 * 					 Accepted values: AD7768_DEC_X32
 * 					 				  AD7768_DEC_X64
 * 					 				  AD7768_DEC_X128
 * 					 				  AD7768_DEC_X256
 * 					 				  AD7768_DEC_X512
 * 					 				  AD7768_DEC_X1024
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_set_mode_config(ad7768_dev *dev, ad7768_ch_mode mode, ad7768_filt_type filt_type, ad7768_dec_rate dec_rate) {
	dev->channel_mode[mode] = (ad7768_Reg_ChMode){
		.filter_type = filt_type,
		.dec_rate = dec_rate
	};

	return ad7768_spi_write(dev, (mode == AD7768_MODE_A) ? AD7768_REG_CH_MODE_A : AD7768_REG_CH_MODE_B, __reg_channelMode_intoRaw(&dev->channel_mode[mode]));
}

/**
 * Get the mode configuration.
 * @param dev - The device structure.
 * @param mode - The channel mode.
 * @param filt_type - The filter type.
 * @param dec_rate - The decimation rate.
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_get_mode_config(ad7768_dev *dev,
			       ad7768_ch_mode mode,
			       ad7768_filt_type *filt_type,
			       ad7768_dec_rate *dec_rate)
{
	*filt_type = dev->channel_mode[mode].filter_type;
	*dec_rate = dev->channel_mode[mode].dec_rate;

	return HAL_OK;
}

/**
 * Set the channel mode.
 * @param dev - The device structure.
 * @param ch - The channel number.
 * 			   Accepted values: AD7768_CH0
 * 			   					AD7768_CH1
 * 			   					AD7768_CH2
 * 			   					AD7768_CH3
 * @param mode - The channel mode.
 * 				 Accepted values: AD7768_MODE_A
 * 								  AD7768_MODE_B
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_set_ch_mode(ad7768_dev *dev,
			   ad7768_ch ch,
			   ad7768_ch_mode mode)
{
	HAL_StatusTypeDef ret = HAL_OK;
    uint8_t raw;

    ret = ad7768_spi_read(dev, AD7768_REG_CH_MODE_SEL, &raw);
    dev->channel_mode_select = __reg_channelModeSelect_fromRaw(raw);
    if(dev->channel_mode_select.ch[ch] != mode){
        dev->channel_mode_select.ch[ch] = mode;
        ret = ad7768_spi_write(dev, AD7768_REG_CH_MODE_SEL, __reg_channelModeSelect_intoRaw(&dev->channel_mode_select));
    }

	return ret;
}

/**
 * Get the channel mode.
 * @param dev - The device structure.
 * @param ch - The channel number.
 * 			   Accepted values: AD7768_CH0
 * 			   					AD7768_CH1
 * 			   					AD7768_CH2
 * 			   					AD7768_CH3
 * @param mode - The channel mode.
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_get_ch_mode(ad7768_dev *dev,
			   ad7768_ch ch,
			   ad7768_ch_mode *mode)
{
	*mode = dev->channel_mode_select.ch[ch];

	return HAL_OK;
}

/*
 * perform soft reset on the device
 * @param device - the device structure
 */
HAL_StatusTypeDef ad7768_softReset(ad7768_dev *dev)
{
    HAL_StatusTypeDef ret = HAL_OK;
    ret |= ad7768_spi_write(dev, AD7768_REG_DATA_CTRL, __reg_dataControl_intoRaw(&(ad7768_Reg_DataControl){.spi_reset = AD7768_SPI_RESET_FIRST}));
    ret |= ad7768_spi_write(dev, AD7768_REG_DATA_CTRL, __reg_dataControl_intoRaw(&(ad7768_Reg_DataControl){.spi_reset = AD7768_SPI_RESET_SECOND}));
    HAL_Delay(5);
    return ret;
}

/**
 * Initialize the device.
 * @param device - The device structure.
 * @param init_param - The structure that contains the device initial
 * 					   parameters.
 * @return 0 in case of success, negative error code otherwise.
 */
HAL_StatusTypeDef ad7768_setup(ad7768_dev *dev){
	HAL_StatusTypeDef ret = HAL_OK;
	uint8_t data = 0;

    ret = ad7768_softReset(dev);
    ret |= ad7768_spi_read(dev, AD7768_REG_DEV_STATUS, &data); // Ensure the status register reads no error

	// TODO: Change to output an error
	// Bit 3 is CHIP_ERROR, Bit 2 is NO_CLOCK_ERROR, check both to see if there is an error
	if(data != 0x00) {
		return HAL_ERROR;
	}

    ret |= ad7768_get_revision_id(dev, &data);

    if(data != 0x06){
        return HAL_ERROR;
    }

	
	ret |= ad7768_spi_write(dev, AD7768_REG_CH_STANDBY,    __reg_channelStandby_intoRaw(&dev->channel_standby));
	ret |= ad7768_spi_write(dev, AD7768_REG_CH_MODE_A,     __reg_channelMode_intoRaw(&dev->channel_mode[AD7768_MODE_A]));
	ret |= ad7768_spi_write(dev, AD7768_REG_CH_MODE_B,     __reg_channelMode_intoRaw(&dev->channel_mode[AD7768_MODE_B]));
	ret |= ad7768_spi_write(dev, AD7768_REG_CH_MODE_SEL,   __reg_channelModeSelect_intoRaw(&dev->channel_mode_select));
	ret |= ad7768_spi_write(dev, AD7768_REG_PWR_MODE,      __reg_powerMode_intoRaw(&dev->power_mode));
	ret |= ad7768_spi_write(dev, AD7768_REG_INTERFACE_CFG, __reg_interfaceCfg_intoRaw(&dev->interface_config));
	ret |= ad7768_spi_write(dev, AD7768_REG_GPIO_CTRL, 	   0x00);
	ret |= ad7768_sync(dev);

	uint8_t error_reg; 
	ret |= ad7768_spi_read(dev, 0x09, &error_reg);

	return ret;
}

HAL_StatusTypeDef ad7768_sync(ad7768_dev *dev){

	HAL_StatusTypeDef ret = HAL_OK;

    ret |= ad7768_spi_write(dev, AD7768_REG_DATA_CTRL, __reg_dataControl_intoRaw(&(ad7768_Reg_DataControl){.spi_sync = 0}));
    ret |= ad7768_spi_write(dev, AD7768_REG_DATA_CTRL, __reg_dataControl_intoRaw(&(ad7768_Reg_DataControl){.spi_sync = 1}));

	return ret;
}

HAL_StatusTypeDef ad7768_get_revision_id(ad7768_dev *dev, uint8_t *reg_data){
    return ad7768_spi_read(dev, AD7768_REG_REV_ID, reg_data);
}
