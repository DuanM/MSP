#ifndef _CONTROL_IO_H
#define _CONTROL_IO_H

#define CTL_LORA_PWR		(1)			//(1u<<0)
#define CTL_GPS_PWR			(2)			//(1u<<1)

void ControlIO_Init(void);

bool_t ControlIO_DevPower(uint8_t type, bool_t state, bool_t save);

void ControlIO_GPS_Pulse_Callback(fpv_t func);
bool_t ControlIO_LoraMode(uint8_t type);
bool_t ControlIO_StatusLight(uint8_t type);
bool_t ControlIO_MoveLight(uint8_t type);
void ControlIO_Buzzer(uint8_t type);
#endif