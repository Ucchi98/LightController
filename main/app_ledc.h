#ifndef _APP_LEDC_H_
#define _APP_LEDC_H_

#include "driver/ledc.h"

extern void init_ledc(void);
extern void set_ledc_duty_and_time(int duty, int time, int channel);

typedef enum {
	LEDC_SNTP_PROC,
	LEDC_SNTP_DONE,
	LEDC_LIGHT_R,
	LEDC_LIGHT_L,
	LEDC_MAX
} LEDC_LED;

#endif
