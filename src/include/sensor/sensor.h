#ifndef __SENSOR_H
#define __SENSOR_H

#define SENSOR_TIMEOUT 1000000

#define SENSOR_EVENT_FXOS			(1u<<1)

extern osel_task_t *sensor_task_h;
extern osel_event_t *sensor_event_h;


void sensor_init(void);

void sensor_handler(uint16_t event_type);


#endif
