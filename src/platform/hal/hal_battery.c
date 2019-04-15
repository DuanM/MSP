#include <platform.h>

void hal_battery_init(void)
{
    
}

uint16_t hal_get_vol_value(void)
{
	uint16_t vol_level=0;    
    uint8_t  energy=0;    
	vol_level = hal_adc14_manual_get();
     
//	curADCResult = (5*(fp32_t)adc_value)/((fp32_t)16384)+0.16;
    
//    curADCResult = 2*(((fp32_t)adc_value)/((fp32_t)4096)*4+0.85);
    
//	vol_level = (uint16_t)(curADCResult * 100);// + 0.5)*10);
    
#if 1
    //if (vol_level>=400)energy = 0x0A;
    //每0.2V 80个梯度
    if (vol_level>=3340)energy = 0x09;//8.2以上   
    if (vol_level>=3260&&vol_level<3340)energy = 0x08;//8.0
    if (vol_level>=3180&&vol_level<3260)energy = 0x07;//7.8
    if (vol_level>=3095&&vol_level<3180)energy = 0x06;//7.6
    if (vol_level>=3015&&vol_level<3095)energy = 0x05;//7.4
    if (vol_level>=2930&&vol_level<3015)energy = 0x04;//7.2
    if (vol_level>=2850&&vol_level<2930)energy = 0x03;//7.0
    if (vol_level>=2780&&vol_level<2850)energy = 0x02;//6.8
    if (vol_level>=2710&&vol_level<2780)energy = 0x01;//6.6.5
    if (vol_level<2710)energy = 0x00; //6.6.5以下
#endif
    
    return energy;
}
































