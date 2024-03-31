#ifndef _APP_LIGHT_CTRL_H_
#define _APP_LIGHT_CTRL_H_

#include <time.h>
#include <sys/time.h>

typedef struct {
	time_t right_on;
	time_t left_on;
	time_t right_off;
	time_t left_off;
	time_t midnight;
} CtrlTime;

typedef struct {
	time_t sunrise;
	time_t sunset;
} Ephemeris;

typedef unsigned char LightState;
#define LIGHT_STATE_BOTH_OFF 0x00
#define LIGHT_STATE_RIGHT_ON 0x01
#define LIGHT_STATE_LEFT_ON  0x02
#define LIGHT_STATE_BOTH_ON  0x03

#define RL_R	0
#define RL_L	1

#define RL_RST	0
#define RL_SET	1

extern void init_light_ctrl_task();
extern void light_ctrl_start_task();
extern void light_ctrl_stop_task();
extern void light_ctrl_relay_ctrl(int nRelay, int nState);
extern void light_ctrl_set_light_state(LightState);
extern LightState light_ctrl_get_light_state();
extern CtrlTime light_ctrl_get_ctrl_time();

#endif
