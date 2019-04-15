#include <platform.h>
#include <fxos8700cq.h>

static fxos_handle_t fxos_handle;

bool_t FXOS_Config(void)
{
    /* Initialize the I2C access function. */
	fxos_handle.I2C_ReceiveFunc = read_multibyte_i2c;
	fxos_handle.I2C_SendFunc = write_i2c;
    
    /* Set Slave Address. */    
	fxos_handle.slaveAddress = FXOS8700CQ_SLAVE_ADDR3;
    
	return FXOS_Init(&fxos_handle);
}

bool_t FXOS_Get_Data(void)
{
	device_info_t *p_device_info = device_info_get();
	return FXOS_ReadSensorData(&fxos_handle,&p_device_info->param.fxos_data);
}


#define  EN_INTERRUPTS

/******************************************************************************
 * Code
 ******************************************************************************/
bool_t FXOS_Init(fxos_handle_t *fxos_handle)
{
	if(fxos_handle==PLAT_NULL) return PLAT_FALSE;

	uint8_t tmp[1] = {0};

	init_i2c();

	if(FXOS_ReadReg(fxos_handle, WHO_AM_I_REG, tmp, 1) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}
	
	if (tmp[0] != kFXOS_WHO_AM_I_Device_ID)
	{
		DBG_FXOS_PRINTF("fxos = %x\r\n",tmp[0]);
	    return PLAT_FALSE;
	}

	/* setup auto sleep with FFMT trigger */
	/* go to standby */
	if(FXOS_ReadReg(fxos_handle, CTRL_REG1, tmp, 1) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}

	if(FXOS_WriteReg(fxos_handle, CTRL_REG1, tmp[0] & (uint8_t)~ACTIVE_MASK) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}

	/* Read again to make sure we are in standby mode. */
	if(FXOS_ReadReg(fxos_handle, CTRL_REG1, tmp, 1) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}
	
	if ((tmp[0] & ACTIVE_MASK) == ACTIVE_MASK)
	{
	    return PLAT_FALSE;
	}

	/* Disable the FIFO */
	if(FXOS_WriteReg(fxos_handle, F_SETUP_REG, F_MODE_DISABLED) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}

#ifdef LPSLEEP_HIRES
	/* enable auto-sleep, low power in sleep, high res in wake */
	if(FXOS_WriteReg(fxos_handle, CTRL_REG2, SLPE_MASK | SMOD_LOW_POWER | MOD_HIGH_RES) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}
#else
	/* enable auto-sleep, low power in sleep, high res in wake */
	if(FXOS_WriteReg(fxos_handle, CTRL_REG2, MOD_HIGH_RES) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}

#endif
		
	/* set up Mag OSR and Hybrid mode using M_CTRL_REG1, use default for Acc */
	//if(FXOS_WriteReg(fxos_handle, M_CTRL_REG1, (M_RST_MASK | M_OSR_MASK | M_HMS_MASK)) != PLAT_TRUE)
	if(FXOS_WriteReg(fxos_handle, M_CTRL_REG1, (FXOS8700CQ_M_CTRL_REG1_M_ACAL | FXOS8700CQ_M_CTRL_REG1_MO_OS3(7) | FXOS8700CQ_M_CTRL_REG1_M_HMS2(3))) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}

	/* Enable hyrid mode auto increment using M_CTRL_REG2 */
	if(FXOS_WriteReg(fxos_handle, M_CTRL_REG2, (M_HYB_AUTOINC_MASK)) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}

#ifdef EN_FFMT
	/* enable FFMT for motion detect for X and Y axes, latch enable */
	if(FXOS_WriteReg(fxos_handle, FF_MT_CFG_REG, XEFE_MASK | YEFE_MASK | ELE_MASK | OAE_MASK) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}
#endif

#ifdef SET_THRESHOLD
	/* set threshold to about 0.25g */
	if(FXOS_WriteReg(fxos_handle, FT_MT_THS_REG, 0x04) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}
#endif

#ifdef SET_DEBOUNCE
	/* set debounce to zero */
	if(FXOS_WriteReg(fxos_handle, FF_MT_COUNT_REG, 0x00) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}
#endif

#ifdef EN_AUTO_SLEEP
	/* set auto-sleep wait period to 5s (=5/0.64=~8) */
	if(FXOS_WriteReg(fxos_handle, ASLP_COUNT_REG, 8) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}
#endif
	/* default set to 4g mode */
	if(FXOS_WriteReg(fxos_handle, XYZ_DATA_CFG_REG, FULL_SCALE_4G) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}
	
#ifdef EN_INTERRUPTS
	/* enable data-ready, auto-sleep and motion detection interrupts INT_CFG_FF_MT_MASK*/
	/* FXOS1_WriteRegister(CTRL_REG4, INT_EN_DRDY_MASK | INT_EN_ASLP_MASK | INT_EN_FF_MT_MASK); */
	if(FXOS_WriteReg(fxos_handle, CTRL_REG4, INT_EN_FF_MT_MASK) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}
	/* route data-ready interrupts to INT1, others INT2 (default) INT_CFG_DRDY_MASK*/
	if(FXOS_WriteReg(fxos_handle, CTRL_REG5,0) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}
	/* enable ffmt as a wake-up source */
	if(FXOS_WriteReg(fxos_handle, CTRL_REG3, WAKE_FF_MT_MASK) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}
	/* finally activate accel_device with ASLP ODR=0.8Hz, ODR=100Hz, FSR=2g */
	if(FXOS_WriteReg(fxos_handle, CTRL_REG1, HYB_ASLP_RATE_0_8HZ | HYB_DATA_RATE_100HZ | ACTIVE_MASK) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}
#else
	/* Setup the ODR for 50 Hz and activate the accelerometer */
	if(FXOS_WriteReg(fxos_handle, CTRL_REG1, (HYB_DATA_RATE_200HZ | ACTIVE_MASK)) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}
#endif

	/* Read Control register again to ensure we are in active mode */
	if(FXOS_ReadReg(fxos_handle, CTRL_REG1, tmp, 1) != PLAT_TRUE)
	{
	    return PLAT_FALSE;
	}

	if ((tmp[0] & ACTIVE_MASK) != ACTIVE_MASK)
	{
	    return PLAT_FALSE;
	}

	return PLAT_TRUE;
}

bool_t FXOS_ReadSensorData(fxos_handle_t *fxos_handle, dev_fxos_t *FxosData)
{
	bool_t status=PLAT_TRUE;
	uint8_t tmp_buff[FXOS8700CQ_READ_LEN] = {0};
	//uint8_t i = 0;
	
	//加速值
	//if (!FXOS_ReadReg(fxos_handle, OUT_X_MSB_REG, tmp_buff, 6) == PLAT_TRUE)
	//{
	//    status = PLAT_FALSE;
	//}
	
	//FxosData->AccelData.x=(tmp_buff[0]<<8|tmp_buff[1]);
	//FxosData->AccelData.y=(tmp_buff[2]<<8|tmp_buff[3]);
	//FxosData->AccelData.z=(tmp_buff[4]<<8|tmp_buff[5]);
	
	// Have to apply corrections to make the int16_t correct
	//if(FxosData->AccelData.x > UINT14_MAX/2) FxosData->AccelData.x -= UINT14_MAX;
	//if(FxosData->AccelData.y > UINT14_MAX/2)  FxosData->AccelData.y -= UINT14_MAX;
	//if(FxosData->AccelData.z > UINT14_MAX/2)  FxosData->AccelData.z -= UINT14_MAX;
	
	//磁力值
	//if (!FXOS_ReadReg(fxos_handle, M_OUT_X_MSB_REG, tmp_buff, 6) == PLAT_TRUE)
	//{
	//    status = PLAT_FALSE;
	//}
	
	//FxosData->MagnData.x=(tmp_buff[0]<<8|tmp_buff[1]);
	//FxosData->MagnData.y=(tmp_buff[2]<<8|tmp_buff[3]);
	//FxosData->MagnData.z=(tmp_buff[4]<<8|tmp_buff[5]);
	
	// Have to apply corrections to make the int16_t correct
	//if(FxosData->MagnData.x > UINT14_MAX/2) FxosData->MagnData.x -= UINT14_MAX;
	//if(FxosData->MagnData.y > UINT14_MAX/2)  FxosData->MagnData.y -= UINT14_MAX;
	//if(FxosData->MagnData.z > UINT14_MAX/2)  FxosData->MagnData.z -= UINT14_MAX;
	
	if (!FXOS_ReadReg(fxos_handle, M_OUT_X_MSB_REG, tmp_buff, FXOS8700CQ_READ_LEN) == PLAT_TRUE)
	{
		status = PLAT_FALSE;
	}
	
	// copy the 14 bit accelerometer byte data into 16 bit words
	FxosData->MagnData.x = (tmp_buff[0] << 8) | tmp_buff[1];
	FxosData->MagnData.y = (tmp_buff[2] << 8) | tmp_buff[3];
	FxosData->MagnData.z = (tmp_buff[4] << 8) | tmp_buff[5];
	
	// copy the magnetometer byte data into 16 bit words
	FxosData->AccelData.x = (tmp_buff[6] << 8) | tmp_buff[7];
	FxosData->AccelData.y = (tmp_buff[8] << 8) | tmp_buff[9];
	FxosData->AccelData.z = (tmp_buff[10] << 8) | tmp_buff[11];
	
	// Have to apply corrections to make the int16_t correct
	if(FxosData->AccelData.x > UINT14_MAX/2) FxosData->AccelData.x -= UINT14_MAX;
	if(FxosData->AccelData.y > UINT14_MAX/2)  FxosData->AccelData.y -= UINT14_MAX;
	if(FxosData->AccelData.z > UINT14_MAX/2)  FxosData->AccelData.z -= UINT14_MAX;
	
	DBG_FXOS_PRINTF("AccelData:x=%d,y=%d,z=%d\r\n",FxosData->AccelData.x,FxosData->AccelData.y,FxosData->AccelData.z);
	DBG_FXOS_PRINTF("MagnData:x=%d,y=%d,z=%d\r\n\r\n",FxosData->MagnData.x,FxosData->MagnData.y,FxosData->MagnData.z);
	
	return status;
}

bool_t FXOS_ReadReg(fxos_handle_t *handle, uint8_t reg, uint8_t *val, uint8_t bytesNumber)
{
    if(handle==PLAT_NULL) return PLAT_FALSE;
    if(val==PLAT_NULL) return PLAT_FALSE;

	if (!handle->I2C_ReceiveFunc)
	{
		return PLAT_FALSE;
	}
	
	handle->I2C_ReceiveFunc(handle->slaveAddress, reg, val, bytesNumber);
	
	return  PLAT_TRUE;
}

bool_t FXOS_WriteReg(fxos_handle_t *handle, uint8_t reg, uint8_t val)
{
	if(handle==PLAT_NULL) return PLAT_FALSE;

	if (!handle->I2C_SendFunc)
	{
	    return PLAT_FALSE;
	}
	
	handle->I2C_SendFunc(handle->slaveAddress, reg, val);
	
	return  PLAT_TRUE;
}






















