#ifndef _CONTROL_IO_H
#define _CONTROL_IO_H

#define CTL_LORA_PWR		(1)			//(1u<<0)
#define CTL_GPS_PWR			(2)			//(1u<<1)

#define ACS_LORA_PWR_PIN		40u
#define SEN_GPS_PWR_PIN			41u
#define SEN_GPS_PULSE_PIN		34u //GPS 脉冲管脚

#define ACS_LORA_AUX_PIN		24u //LORA AUX管脚
#define ACS_LORA_M1_PIN			25u //LORA M1管脚
#define ACS_LORA_M0_PIN			26u //LORA M0管脚

#define DEV_AUX_PIN_VALUE   GPIO_getInputPinValue((ACS_LORA_AUX_PIN/10), (1u<<(ACS_LORA_AUX_PIN%10)))

#define DEV_STATE_LIGHT_RED_PIN			52u //工作状态灯 红管脚
#define DEV_STATE_LIGHT_GREEN_PIN		51u //工作状态灯 绿管脚
#define DEV_STATE_LIGHT_YELLOW_PIN		50u //工作状态灯 黄管脚

#define DEV_STATE_LIGHT_RED_PIN_VALUE GPIO_getInputPinValue((DEV_STATE_LIGHT_RED_PIN/10), (1u<<(DEV_STATE_LIGHT_RED_PIN%10))
#define DEV_STATE_LIGHT_GREEN_PIN_VALU GPIO_getInputPinValue((DEV_STATE_LIGHT_GREEN_PIN/10), (1u<<(DEV_STATE_LIGHT_GREEN_PIN%10)))
#define DEV_STATE_LIGHT_YELLOW_PIN_VALUE GPIO_getInputPinValue((DEV_STATE_LIGHT_YELLOW_PIN/10), (1u<<(DEV_STATE_LIGHT_YELLOW_PIN%10)))

#define DEV_MOVE_LIGHT_RED_PIN			47u //移动状态灯 红管脚
#define DEV_MOVE_LIGHT_GREEN_PIN		46u //移动状态灯 绿管脚
#define DEV_MOVE_LIGHT_YELLOW_PIN		45u //移动状态灯 黄管脚

#define DEV_MOVE_LIGHT_RED_PIN_VALUE     GPIO_getInputPinValue((DEV_MOVE_LIGHT_RED_PIN/10), (1u<<(DEV_MOVE_LIGHT_RED_PIN%10)))
#define DEV_MOVE_LIGHT_GREEN_PIN_VALUE   GPIO_getInputPinValue((DEV_MOVE_LIGHT_GREEN_PIN/10), (1u<<(DEV_MOVE_LIGHT_GREEN_PIN%10)))
#define DEV_MOVE_LIGHT_YELLOW_PIN_VALUE  GPIO_getInputPinValue((DEV_MOVE_LIGHT_YELLOW_PIN/10), (1u<<(DEV_MOVE_LIGHT_YELLOW_PIN%10)))

#define DEV_BUZZER_PIN					43u //移动蜂鸣器管脚
#define DEV_BUZZER_PIN_VALUE   GPIO_getInputPinValue((DEV_BUZZER_PIN/10), (1u<<(DEV_BUZZER_PIN%10)))

#define DEV_SW1_PIN						44u //移动按键管脚
#define DEV_KEY_PIN_VALUE    GPIO_getInputPinValue((DEV_SW1_PIN/10), (1u<<(DEV_SW1_PIN%10)))


void ControlIO_Init(void);

bool_t ControlIO_DevPower(uint8_t type, bool_t state, bool_t save);

void ControlIO_Aux_Callback(fpv_t callback);

void ControlIO_GPS_Pulse_Callback(fpv_t func);

bool_t ControlIO_LoraMode(uint8_t type);

void ControlIO_Key_Callback(fpv_t callback);

void ControlIO_StatusLight(uint8_t type);

void ControlIO_MoveLight_State(void);
void ControlIO_MoveLight(void);

void ControlIO_Buzzer_State(void);
void ControlIO_Buzzer(void);
#endif
