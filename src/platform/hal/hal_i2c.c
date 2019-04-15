#include <platform.h>
#include <ti\devices\msp432p4xx\driverlib\i2c.h>
#include <ti\devices\msp432p4xx\driverlib\debug.h>
#include <ti\devices\msp432p4xx\driverlib\interrupt.h>
#include <ti\devices\msp432p4xx\driverlib\gpio.h>
#include <ti\devices\msp432p4xx\driverlib\cs.h>

#include <ti\devices\msp432p4xx\driverlib\gpio.h>

#define SIMULATE_M         1

#define SCL_CFG     GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN7)
#define SCL_HIGH    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN7)
#define SCL_LOW     GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN7)

#define SDA_INPUT   GPIO_setAsInputPin(GPIO_PORT_P1, GPIO_PIN6)
#define SDA_IPV     GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN6)

#define SDA_OUTPUT  GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN6)
#define SDA_HIGH    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN6)
#define SDA_LOW     GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN6)

#if SIMULATE_M
#else

const eUSCI_I2C_MasterConfig i2cConfig =
{
	EUSCI_B_I2C_CLOCKSOURCE_SMCLK, 		// SMCLK Clock Source
	24000000,		 		            // SMCLK = 400kHz
	EUSCI_B_I2C_SET_DATA_RATE_400KBPS, 	// Desired I2C Clock of 400khz
	0, 					                // No byte counter threshold
	EUSCI_B_I2C_NO_AUTO_STOP 	        // No Autostop
};

#endif

//*****************************************************
/*I2C总线实始化，将P0.4和P0.5配置为输出口，并且拉高*/
/*
        I2C总线的开始时序：
        1、将数据线SDA和时钟线SCL全部拉高
*/
void i2c_init(void)
{    
    SCL_CFG;
    SCL_HIGH;
    SDA_OUTPUT;
    SDA_HIGH;
}

/*I2C总线的开始信号*/
/*
        I2C总线的开始时序：
        1、将SDA数据线拉高
        2、将SCL时钟线拉高,延时一段时间
        3、在SCL为高电平时，拉低SDA，延时一段时间
        总结：就是在SCL为高电平时，在SDA上给一个下降沿，表示开始信号。
*/
void i2c_start(void)
{
        SDA_HIGH;
		SCL_HIGH;
		delay_us(5);
        SDA_LOW;   //START
        delay_us(5);
}

/*I2C总线的停止信号*/
/*
        I2C总线的停止时序:
        1、将SDA数据线拉低
        2、将SCL数据线拉高，延时一段时间
        3、在SCL为高电平时，将SDA数据线拉高，延时一段时间
        总结：在SCL为高电平时，在SDA上给一下上升沿，表示停止信号。
*/
void i2c_stop(void)
{
		SDA_LOW;
		SCL_HIGH;
		delay_us(5);//t(SUSTO)	min 0.6us
        SDA_HIGH;              //STOP
		delay_us(5);//t(BUF)  min1.3us 
}

/*I2C总线的应答信号*/
/*
        I2C总线应答时序：
        1、首先是需要将模拟数据线的SDA口配置为输入口。
        2、将时钟线SCL拉高，延时一段时间
        3、等待从设备应答，从设备应答时，SDA上会产生一个高电平，主设备的GPIO口检测到高电平，则表示应答成功。
        4、如果检测到应答信号，则拉SCL时钟线拉低。延时一段时间。
        5、将模拟SDA口的GPIO口配置为输出。
*/
uint8_t i2c_check_ack(void)
{
      uint8_t temp_ack =0;
      SDA_INPUT;
	  SCL_HIGH;
	  delay_us(5);
      if(SDA_IPV)
  	  {
  	  	temp_ack = 1;
  	  }
	  else
	  {
	  	temp_ack = 0;
	  }
      //delay_us(5);
      SCL_LOW;
      SDA_OUTPUT;
    return temp_ack;
}
/*I2C总线写数据函数，将要写的数据传递给形参I2CBuf*/
/*
        I2C写数据时序：
        注意：由于I2C属于串行总线，所以数据都是一位一位的传输
        1、将SCL时钟线拉低，准备传输数据，
        2、将I2CBuf的最高位传输到SDA数据线上,延时一段时间
        3、拉高SCL时钟线，延时一段时间
        小结：当SCL有一个上升沿时，传输数据。
        以上三步表示传输完一位数据。然后将I2CBuf左移一位，再重复以上三步。
        最后，将SCL位低，延时一段时间，再将SDA位高，延时一段时间。此操作是为了释放时钟线SCL和数据线SDA.
*/
void i2c_send(uint8_t buf)
{
    uint8_t i=0; 
    for(i=0; i<8; i++) 
    {
    	SCL_LOW;
        if(buf & 0x80)   
            SDA_HIGH;
        else
            SDA_LOW;
		//delay_us(5);
        SCL_HIGH;
		delay_us(5);
        buf = buf<<1;
    }
      SCL_LOW;
}

/*I2C总线读数据函数，该函数具有返回值I2CBuf*/
/*
        I2C读数据函数的时序：
        注意：首先需要将模拟SDA数据线的GPIO口配置为输入
        1、将时钟线SCL拉高，延时一段时间
        2、读取数据线SDA上的电平，然后写入I2CBuf的最低位。
        3、拉低SCL时钟线，延时一段时间
        小结：当SCL有一个下降沿时，读取数据。
        以上三步表示读取一位数据，读一个字节，需要重复以上操作步骤８次。
*/
uint8_t i2c_read(void)
{
  uint8_t data = 0; 
  SDA_INPUT;
  for(uint8_t i = 0;i<8;i++)
  {
      data = data<<1;
	  SCL_HIGH;
	  delay_us(5);
      if(SDA_IPV)
        data |= 0x1;
       SCL_LOW;
	  delay_us(5);
  }
  SDA_OUTPUT;
  return    data;    
}
/*
注：在从设备地址没有做处理的情况下：
        I2C读寄存器函数的时序：
        1、主设备发出开始信号
        2、写入从设备地址，从设备的地址左移一位 + 0，此时的0表示要从寄存器中写数据
        3、主设备接收从设备发来的应答信号
        4、写入寄存器地址
        5、主设备接收应答信号
        6、主设备再次发出开始信号
        7、主设备向从设备写入（从设备的地址左移一位+１），此时加的１表示要从寄存器中读数据
        8、从设备给出应答信号
        9、从从设备的寄存器中读出数据传给变量
        10、主设备发出停止信号
        11、返回读出的数据
*/
uint8_t i2c_readreg(uint8_t salveadd,uint8_t regadd)
{
  uint8_t data;
  
  i2c_start();
  i2c_send((salveadd<<1));
  i2c_check_ack();
  i2c_send(regadd);
  i2c_check_ack();
  
  i2c_start();
  i2c_send(((salveadd<<1)|0x01));
  i2c_check_ack();
  data = i2c_read();
  i2c_check_ack();
  i2c_stop();
  return data;
}
/*
        I2C写寄存器函数的时序：
        1、主设备先发出一个开始信号
        2、然后写入从设备地址,从设备的地址左移一位 + 0。每一个从设备的地址是唯一的。
        3、主设备接收应答信号
        4、主设备向从设备写入寄存器的地址
        5、主设备接收应答信号
        6、主设备向从设备写入数据
        7、主设备接收应答信号
        8、主设备发出停止信号
*/
void i2c_writereg(uint8_t salveadd,uint8_t regadd,uint8_t regvalue)
{
  i2c_start();
  i2c_send((salveadd<<1));
  i2c_check_ack();
  i2c_send(regadd);
  i2c_check_ack();
  i2c_send(regvalue);
  i2c_check_ack();
  i2c_stop();
}

uint8_t read_i2c(uint8_t slave_addr, uint8_t register_addr)
{
#if SIMULATE_M
    uint8_t data = i2c_readreg(slave_addr,register_addr);
    return data;
#else
	/* Specify slave address */
	I2C_setSlaveAddress(EUSCI_B0_BASE, slave_addr);

	I2C_setMode (EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
    I2C_masterSendSingleByte(EUSCI_B0_BASE,register_addr);

	/* Wait until ready */
	//I2C_masterSendStart(EUSCI_B0_BASE);

	/* Send start bit and register */
	//I2C_masterSendMultiByteNext(EUSCI_B0_BASE,register_addr);
    //I2C_masterSendSingleByte(EUSCI_B0_BASE,register_addr);

    
	/* Wait for tx to complete */
	//while(!(I2C_getInterruptStatus(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0) &
	//	EUSCI_B_I2C_TRANSMIT_INTERRUPT0));

	/* Turn off TX and generate RE-Start */
	I2C_masterReceiveStart(EUSCI_B0_BASE);

	/* Wait for start bit to complete */
	while(I2C_masterIsStartSent(EUSCI_B0_BASE));

	uint8_t data = I2C_masterReceiveMultiByteFinish(EUSCI_B0_BASE);
    
	/* Read one or more bytes */
	I2C_clearInterruptFlag(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);

	return data;
#endif
}

void read_multibyte_i2c(uint8_t slave_addr, uint8_t register_addr, uint8_t *data, uint8_t num_bytes)
{
	int i;
	for(i = 0; i < num_bytes; i++)
    {
#if SIMULATE_M
      data[i] = i2c_readreg(slave_addr,register_addr++);
#else    
	  data[i] = read_i2c(slave_addr, register_addr++);
#endif
	}
}

void write_i2c(uint8_t slave_addr, uint8_t register_addr, uint8_t register_data)
{
#if SIMULATE_M
    i2c_writereg(slave_addr,register_addr,register_data);
#else
	/* Specify slave address */
	I2C_setSlaveAddress(EUSCI_B0_BASE, slave_addr);

	I2C_setMode (EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

	/* Wait until ready */
	I2C_masterSendStart(EUSCI_B0_BASE);

	/* Send start bit and register */
	I2C_masterSendMultiByteNext(EUSCI_B0_BASE, register_addr);

	/* Send byte of data to write to register */
	I2C_masterSendMultiByteFinish(EUSCI_B0_BASE,register_data);

	I2C_clearInterruptFlag(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);
#endif
}

void init_i2c(void)
{
#if SIMULATE_M
  i2c_init();
#else
	GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN7);
	GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN7);

	/* Setting P1.6 & P1.7 as I2C */
	GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1,
				GPIO_PIN6 + GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);

	/* Disabling I2C module to clear all I2C registers before initializing */
	I2C_disableModule(EUSCI_B0_BASE);

	/* Initializing I2c Master to SMCLK at 400kbs with no autostop */
	I2C_initMaster(EUSCI_B0_BASE, &i2cConfig);

	/* Set master to transmit mode */
	I2C_setMode (EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

	/* Enable I2C Module to start operations */
	I2C_enableModule(EUSCI_B0_BASE);
#endif
}


