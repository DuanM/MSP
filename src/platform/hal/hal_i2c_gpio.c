#include <platform.h>
#include <ti\devices\msp432p4xx\driverlib\gpio.h>

void hal_i2c_gpio_init(void)
{
    //配置震动增益改变I2C总线
    PZ_SCL_CFG;
    PZ_SCL_HIGH;
    PZ_SDA_OUTPUT;
    PZ_SDA_HIGH;
    PZ_SHDN_CFG;
    PZ_SHDN_HIGH;
    
    //配置声阵列增益改变I2C总线
    PS_SCL_CFG;
    PS_SCL_HIGH;
    PS_SDA_OUTPUT;
    PS_SDA_HIGH;
    PS_RESET_CFG;
    PS_RESET_HIGH;    
}


void hal_i2c_start(bool flag)
{
    if(flag){
        PZ_SCL_HIGH;
        hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME); 
        PZ_SDA_HIGH;
        hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME); 
        PZ_SDA_LOW;
        hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME);
        PZ_SCL_LOW;
    }
    else{
        PS_SCL_HIGH;
        hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME);
        PS_SDA_HIGH;
        hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME);
        PS_SDA_LOW;
        hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME);
        PS_SCL_LOW;
    }
    
    hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME);
}


void hal_i2c_stop(bool flag)
{
    if(flag){
        PZ_SDA_LOW;
        hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME);
        PZ_SCL_LOW;
        hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME);
        PZ_SCL_HIGH;
        hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME); 
        PZ_SDA_HIGH;
    }
    else{
        PS_SDA_LOW;
        hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME);
        PS_SCL_LOW;
        hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME);
        PS_SCL_HIGH;
        hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME); 
        PS_SDA_HIGH;
    }
    
    hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME);
}


bool hal_i2c_check_ack(bool flag)
{
    bool temp_ack;
    
    if(flag){
        PZ_SDA_HIGH;
        hal_i2c_delay_170ns(9);//delay_us(DELAY_TIME);
        PZ_SDA_INPUT;
        hal_i2c_delay_170ns(9);
        PZ_SCL_HIGH;
        //PZ_SDA_INPUT;
        hal_i2c_delay_170ns(9);//delay_us(DELAY_TIME);
        
        if(PZ_SDA_IPV)
          temp_ack = false;
        else
          temp_ack = true;
        
        PZ_SCL_LOW;
        hal_i2c_delay_170ns(9);//delay_us(DELAY_TIME);
        PZ_SDA_OUTPUT;
    }
    else{
        PS_SDA_HIGH;
        hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME);
        PS_SDA_INPUT;
        hal_i2c_delay_170ns(30);
        PS_SCL_HIGH;
        //PS_SDA_INPUT;
        hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME);
        
        if(PS_SDA_IPV)
          temp_ack = false;
        else
          temp_ack = true;
        
        PS_SCL_LOW;
        hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME);
        PS_SDA_OUTPUT;
    }
    
    hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME);
    return temp_ack;
}


void hal_i2c_send_byte(uint8_t buf, bool flag)
{
    uint8_t i; //,j
    
    i = 0;
    if(flag){
        while(1) //for(i=0; i<8; i++) //
        {
            if(buf & 0x80)
                PZ_SDA_HIGH;
            else
                PZ_SDA_LOW;
            
            hal_i2c_delay_170ns(4);//delay_us(DELAY_TIME);
            PZ_SCL_HIGH;
            i++;
            if(i>7){
              hal_i2c_delay_170ns(4);//delay_us(DELAY_TIME);
              PZ_SCL_LOW;
              break;
            }
            buf = buf<<1;
            hal_i2c_delay_170ns(4);//delay_us(DELAY_TIME);
            PZ_SCL_LOW;
        }
    }
    else{
        for(i=0; i<8; i++)
        {            
            if(buf & 0x80)
                PS_SDA_HIGH;
            else
                PS_SDA_LOW;
            
            hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME);
            PS_SCL_HIGH;
            hal_i2c_delay_170ns(30);//delay_us(DELAY_TIME);
            PS_SCL_LOW;
            
            buf = buf<<1;
            hal_i2c_delay_170ns(6);//delay_us(DELAY_TIME);
        }
    }
}


void hal_i2c_adjust_gain_level(uint8_t address, uint8_t instruction, uint8_t value, bool flag)
{
    bool rxd_ack;
    
    hal_i2c_start(flag);
    
    rxd_ack = false;
    while(!rxd_ack){
      hal_i2c_send_byte(address, flag);
      rxd_ack = hal_i2c_check_ack(flag);
    }
    
    rxd_ack = false;
    while(!rxd_ack){
      hal_i2c_send_byte(instruction, flag);
      rxd_ack = hal_i2c_check_ack(flag);
    }
    
    rxd_ack = false;
    while(!rxd_ack){
      hal_i2c_send_byte(value, flag);
      rxd_ack = hal_i2c_check_ack(flag);
    }
    
    hal_i2c_stop(flag);
}


void hal_i2c_delay_170ns(uint8_t times)
{
    uint8_t i;
    
    for(i=0; i<times; i++);
}



