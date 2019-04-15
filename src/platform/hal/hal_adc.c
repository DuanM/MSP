
#include <platform.h>
#include <ti\devices\msp432p4xx\driverlib\gpio.h>
#include <ti\devices\msp432p4xx\driverlib\ref_a.h>
#include <ti\devices\msp432p4xx\driverlib\adc14.h>

void hal_adc14_init(void)
{	
    bool_t res;
    
    REF_A_setReferenceVoltage(REF_A_VREF2_5V);
    REF_A_disableTempSensor();
    REF_A_enableReferenceVoltage();
    ADC14_enableModule();
    ADC14_setResolution(ADC_12BIT);
    res = ADC14_initModule(ADC_CLOCKSOURCE_SMCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, ADC_NOROUTE);
	DBG_ASSERT(res == PLAT_TRUE);
    
    /* Configuring GPIOs for Analog In */
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5,GPIO_PIN5 | GPIO_PIN4 | GPIO_PIN0, GPIO_SECONDARY_MODULE_FUNCTION);
    res = ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);
	DBG_ASSERT(res == PLAT_TRUE);
    res = ADC14_setPowerMode(ADC_UNRESTRICTED_POWER_MODE);
    DBG_ASSERT(res == PLAT_TRUE);
    res = ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A0, ADC_NONDIFFERENTIAL_INPUTS);
    DBG_ASSERT(res == PLAT_TRUE);
    res = ADC14_configureConversionMemory(ADC_MEM1, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A1, ADC_NONDIFFERENTIAL_INPUTS);
    DBG_ASSERT(res == PLAT_TRUE);
	res = ADC14_configureConversionMemory(ADC_MEM5, ADC_VREFPOS_INTBUF_VREFNEG_VSS, ADC_INPUT_A5, ADC_NONDIFFERENTIAL_INPUTS);
	DBG_ASSERT(res == PLAT_TRUE);
	res = ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM5, PLAT_FALSE);
    DBG_ASSERT(res == PLAT_TRUE);
    res = ADC14_setSampleHoldTrigger(ADC_TRIGGER_ADCSC, PLAT_FALSE);
   	DBG_ASSERT(res == PLAT_TRUE);
    res = ADC14_setSampleHoldTime(ADC_PULSE_WIDTH_4, ADC_PULSE_WIDTH_4);
    ADC14_enableConversion();
    DBG_ASSERT(res == PLAT_TRUE);
    res = ADC14_toggleConversionTrigger();
	DBG_ASSERT(res == PLAT_TRUE);
	//add for battery value
	//ADC14_configureSingleSampleMode(ADC_MEM5, PLAT_TRUE);
}	

void hal_adc14_get(uint16_t *measure_value, uint8_t size)
{
	for (uint8_t i=0; i<size; i++)
	{
		measure_value[i] = ADC14_getResult(ADC_MEM0<<i);
	}
 
  	ADC14_toggleConversionTrigger();
}

uint16_t hal_adc14_manual_get(void)
{	
    ADC14_toggleConversionTrigger();
    
	return ADC14_getResult(ADC_MEM5);
}




