#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "app_ledc.h"

#define LEDC_FREQ 500

#define LEDC_GPIO_LED1_R GPIO_NUM_8
#define LEDC_GPIO_LED1_B GPIO_NUM_9
#define LEDC_GPIO_LED2_R GPIO_NUM_0
#define LEDC_GPIO_LED2_B GPIO_NUM_1

enum {
	LED1_R,
	LED1_B,
	LED2_R,
	LED2_B,
};

typedef struct {
	int target_duty;
	int fade_time_ms;
	int channel;
} FadeConf;

FadeConf fc[4] = { 0 };
TaskHandle_t x_task[4] = { 0 };

void v_ledc_fade_task(void *pv_params)
{
	FadeConf *fc = (FadeConf *)pv_params;
	for(;;)
	{
		// fade in
		ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, fc->channel, fc->target_duty, fc->fade_time_ms);
		ledc_fade_start(LEDC_LOW_SPEED_MODE, fc->channel, LEDC_FADE_WAIT_DONE);

		// fade out
		ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, fc->channel, 0, fc->fade_time_ms);
		ledc_fade_start(LEDC_LOW_SPEED_MODE, fc->channel, LEDC_FADE_WAIT_DONE);
	}
}

void init_ledc()
{
	ledc_timer_config_t ledc_timer = {
		.timer_num       = LEDC_TIMER_0,
		.duty_resolution = LEDC_TIMER_14_BIT,  // resolution of PWM duty
		.freq_hz         = LEDC_FREQ,          // frequency of PWM signal
		.speed_mode      = LEDC_LOW_SPEED_MODE // timer mode
	};
	ledc_timer_config(&ledc_timer);

	ledc_channel_config_t ledc_channel[] =
	{
		{
			.speed_mode     = LEDC_LOW_SPEED_MODE,
			.channel        = LEDC_CHANNEL_0,
			.timer_sel      = LEDC_TIMER_0,
			.intr_type      = LEDC_INTR_DISABLE,
			.gpio_num       = LEDC_GPIO_LED1_R,
			.duty           = 0,
			.hpoint         = 0
		},
		{
			.speed_mode     = LEDC_LOW_SPEED_MODE,
			.channel        = LEDC_CHANNEL_1,
			.timer_sel      = LEDC_TIMER_0,
			.intr_type      = LEDC_INTR_DISABLE,
			.gpio_num       = LEDC_GPIO_LED1_B,
			.duty           = 0,
			.hpoint         = 0
		},
		{
			.speed_mode     = LEDC_LOW_SPEED_MODE,
			.channel        = LEDC_CHANNEL_2,
			.timer_sel      = LEDC_TIMER_0,
			.intr_type      = LEDC_INTR_DISABLE,
			.gpio_num       = LEDC_GPIO_LED2_R,
			.duty           = 0,
			.hpoint         = 0
		},
		{
			.speed_mode     = LEDC_LOW_SPEED_MODE,
			.channel        = LEDC_CHANNEL_3,
			.timer_sel      = LEDC_TIMER_0,
			.intr_type      = LEDC_INTR_DISABLE,
			.gpio_num       = LEDC_GPIO_LED2_B,
			.duty           = 0,
			.hpoint         = 0
		}
	};

	ledc_fade_func_install(0);

	char s_task[16];
	for(int i=0 ; i<sizeof(ledc_channel) / sizeof(ledc_channel_config_t); i++){
		sprintf(s_task, "ledc_task_%d", i);

		fc[i].target_duty = 0;
		fc[i].fade_time_ms = 1000;
		fc[i].channel = ledc_channel[i].channel;

		ledc_channel_config(&ledc_channel[i]);
		xTaskCreate(v_ledc_fade_task, s_task, 2048, &fc[i], tskIDLE_PRIORITY, &x_task[i]);
	}
}

void set_ledc_duty_and_time(int duty, int time, int channel)
{
	fc[channel].target_duty  = duty;
	fc[channel].fade_time_ms = time;
}

