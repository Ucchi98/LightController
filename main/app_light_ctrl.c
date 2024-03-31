#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// FreeRTOS 関連
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_log.h"

#include "soc/gpio_reg.h"
#include "driver/gpio.h"

#include "app_light_ctrl.h"
#include "app_ledc.h"

#define PULSE_LO 0
#define PULSE_HI 1

#define GPIO_RL_R_SET GPIO_NUM_4
#define GPIO_RL_R_RST GPIO_NUM_5
#define GPIO_RL_L_SET GPIO_NUM_6
#define GPIO_RL_L_RST GPIO_NUM_7

#define STACK_SIZE (4 * 1024)

static const char *TAG = "app_light_ctrl";

const Ephemeris e_data[] = // { sunrise, sunset }
{
	{ 24600, 59880 }, { 24660, 59940 }, { 24660, 60000 }, { 24660, 60000 }, { 24660, 60060 }, { 24660, 60120 }, { 24660, 60180 }, { 24660, 60240 }, { 24660, 60300 }, { 24660, 60360 },
	{ 24660, 60360 }, { 24660, 60420 }, { 24660, 60480 }, { 24600, 60540 }, { 24600, 60600 }, { 24600, 60660 }, { 24600, 60720 }, { 24540, 60780 }, { 24540, 60840 }, { 24540, 60900 },
	{ 24480, 60960 }, { 24480, 61020 }, { 24420, 61080 }, { 24420, 61140 }, { 24360, 61200 }, { 24360, 61260 }, { 24300, 61320 }, { 24240, 61380 }, { 24240, 61440 }, { 24180, 61560 },
	{ 24120, 61620 }, { 24120, 61680 }, { 24060, 61740 }, { 24000, 61800 }, { 23940, 61860 }, { 23880, 61920 }, { 23880, 61980 }, { 23820, 62040 }, { 23760, 62100 }, { 23700, 62160 },
	{ 23640, 62220 }, { 23580, 62280 }, { 23520, 62340 }, { 23460, 62400 }, { 23400, 62460 }, { 23340, 62520 }, { 23280, 62580 }, { 23220, 62640 }, { 23160, 62700 }, { 23040, 62760 },
	{ 22980, 62820 }, { 22920, 62880 }, { 22860, 62940 }, { 22800, 63000 }, { 22680, 63060 }, { 22620, 63120 }, { 22560, 63180 }, { 22500, 63240 }, { 22380, 63240 }, { 22320, 63300 },
	{ 22260, 63360 }, { 22200, 63420 }, { 22080, 63480 }, { 22020, 63540 }, { 21960, 63600 }, { 21840, 63660 }, { 21780, 63720 }, { 21660, 63780 }, { 21600, 63780 }, { 21540, 63840 },
	{ 21420, 63900 }, { 21360, 63960 }, { 21300, 64020 }, { 21180, 64080 }, { 21120, 64140 }, { 21000, 64140 }, { 20940, 64200 }, { 20820, 64260 }, { 20760, 64320 }, { 20700, 64380 },
	{ 20580, 64440 }, { 20520, 64440 }, { 20400, 64500 }, { 20340, 64560 }, { 20220, 64620 }, { 20160, 64680 }, { 20100, 64740 }, { 19980, 64740 }, { 19920, 64800 }, { 19800, 64860 },
	{ 19740, 64920 }, { 19620, 64980 }, { 19560, 65040 }, { 19500, 65040 }, { 19380, 65100 }, { 19320, 65160 }, { 19200, 65220 }, { 19140, 65280 }, { 19080, 65340 }, { 18960, 65340 },
	{ 18900, 65400 }, { 18840, 65460 }, { 18720, 65520 }, { 18660, 65580 }, { 18600, 65580 }, { 18480, 65640 }, { 18420, 65700 }, { 18360, 65760 }, { 18240, 65820 }, { 18180, 65880 },
	{ 18120, 65880 }, { 18060, 65940 }, { 17940, 66000 }, { 17880, 66060 }, { 17820, 66120 }, { 17760, 66180 }, { 17700, 66240 }, { 17580, 66240 }, { 17520, 66300 }, { 17460, 66360 },
	{ 17400, 66420 }, { 17340, 66480 }, { 17280, 66540 }, { 17220, 66540 }, { 17160, 66600 }, { 17100, 66660 }, { 17040, 66720 }, { 16980, 66780 }, { 16920, 66840 }, { 16860, 66840 },
	{ 16800, 66900 }, { 16740, 66960 }, { 16680, 67020 }, { 16680, 67080 }, { 16620, 67080 }, { 16560, 67140 }, { 16500, 67200 }, { 16440, 67260 }, { 16440, 67320 }, { 16380, 67320 },
	{ 16320, 67380 }, { 16320, 67440 }, { 16260, 67500 }, { 16260, 67500 }, { 16200, 67560 }, { 16140, 67620 }, { 16140, 67680 }, { 16080, 67680 }, { 16080, 67740 }, { 16080, 67800 },
	{ 16020, 67800 }, { 16020, 67860 }, { 15960, 67920 }, { 15960, 67920 }, { 15960, 67980 }, { 15960, 67980 }, { 15900, 68040 }, { 15900, 68040 }, { 15900, 68100 }, { 15900, 68160 },
	{ 15900, 68160 }, { 15900, 68160 }, { 15900, 68220 }, { 15900, 68220 }, { 15900, 68280 }, { 15900, 68280 }, { 15900, 68340 }, { 15900, 68340 }, { 15900, 68340 }, { 15900, 68340 },
	{ 15900, 68400 }, { 15900, 68400 }, { 15960, 68400 }, { 15960, 68400 }, { 15960, 68460 }, { 15960, 68460 }, { 16020, 68460 }, { 16020, 68460 }, { 16020, 68460 }, { 16080, 68460 },
	{ 16080, 68460 }, { 16080, 68460 }, { 16140, 68460 }, { 16140, 68460 }, { 16200, 68460 }, { 16200, 68460 }, { 16260, 68400 }, { 16260, 68400 }, { 16320, 68400 }, { 16320, 68400 },
	{ 16380, 68340 }, { 16440, 68340 }, { 16440, 68340 }, { 16500, 68280 }, { 16500, 68280 }, { 16560, 68280 }, { 16620, 68220 }, { 16620, 68220 }, { 16680, 68160 }, { 16740, 68160 },
	{ 16740, 68100 }, { 16800, 68040 }, { 16860, 68040 }, { 16860, 67980 }, { 16920, 67920 }, { 16980, 67920 }, { 17040, 67860 }, { 17040, 67800 }, { 17100, 67740 }, { 17160, 67740 },
	{ 17220, 67680 }, { 17220, 67620 }, { 17280, 67560 }, { 17340, 67500 }, { 17400, 67440 }, { 17460, 67380 }, { 17460, 67320 }, { 17520, 67260 }, { 17580, 67200 }, { 17640, 67140 },
	{ 17640, 67080 }, { 17700, 67020 }, { 17760, 66960 }, { 17820, 66900 }, { 17880, 66840 }, { 17880, 66780 }, { 17940, 66720 }, { 18000, 66660 }, { 18060, 66540 }, { 18060, 66480 },
	{ 18120, 66420 }, { 18180, 66360 }, { 18240, 66240 }, { 18300, 66180 }, { 18300, 66120 }, { 18360, 66060 }, { 18420, 65940 }, { 18480, 65880 }, { 18480, 65820 }, { 18540, 65700 },
	{ 18600, 65640 }, { 18660, 65580 }, { 18660, 65460 }, { 18720, 65400 }, { 18780, 65280 }, { 18840, 65220 }, { 18900, 65160 }, { 18900, 65040 }, { 18960, 64980 }, { 19020, 64860 },
	{ 19080, 64800 }, { 19080, 64680 }, { 19140, 64620 }, { 19200, 64560 }, { 19260, 64440 }, { 19260, 64380 }, { 19320, 64260 }, { 19380, 64200 }, { 19440, 64080 }, { 19440, 64020 },
	{ 19500, 63900 }, { 19560, 63840 }, { 19620, 63720 }, { 19620, 63660 }, { 19680, 63540 }, { 19740, 63480 }, { 19800, 63360 }, { 19800, 63300 }, { 19860, 63240 }, { 19920, 63120 },
	{ 19980, 63060 }, { 19980, 62940 }, { 20040, 62880 }, { 20100, 62760 }, { 20160, 62700 }, { 20220, 62580 }, { 20220, 62520 }, { 20280, 62460 }, { 20340, 62340 }, { 20400, 62280 },
	{ 20460, 62160 }, { 20460, 62100 }, { 20520, 62040 }, { 20580, 61920 }, { 20640, 61860 }, { 20700, 61800 }, { 20760, 61680 }, { 20820, 61620 }, { 20820, 61560 }, { 20880, 61440 },
	{ 20940, 61380 }, { 21000, 61320 }, { 21060, 61260 }, { 21120, 61140 }, { 21180, 61080 }, { 21240, 61020 }, { 21240, 60960 }, { 21300, 60900 }, { 21360, 60780 }, { 21420, 60720 },
	{ 21480, 60660 }, { 21540, 60600 }, { 21600, 60540 }, { 21660, 60480 }, { 21720, 60420 }, { 21780, 60360 }, { 21840, 60300 }, { 21900, 60240 }, { 21960, 60180 }, { 22020, 60120 },
	{ 22080, 60060 }, { 22140, 60000 }, { 22200, 59940 }, { 22260, 59940 }, { 22320, 59880 }, { 22380, 59820 }, { 22440, 59760 }, { 22500, 59760 }, { 22560, 59700 }, { 22620, 59640 },
	{ 22680, 59640 }, { 22740, 59580 }, { 22800, 59520 }, { 22860, 59520 }, { 22920, 59460 }, { 22980, 59460 }, { 23040, 59400 }, { 23100, 59400 }, { 23100, 59400 }, { 23160, 59340 },
	{ 23220, 59340 }, { 23280, 59340 }, { 23340, 59280 }, { 23400, 59280 }, { 23460, 59280 }, { 23520, 59280 }, { 23580, 59280 }, { 23640, 59280 }, { 23700, 59280 }, { 23760, 59280 },
	{ 23760, 59280 }, { 23820, 59280 }, { 23880, 59280 }, { 23940, 59280 }, { 24000, 59280 }, { 24000, 59280 }, { 24060, 59280 }, { 24120, 59340 }, { 24180, 59340 }, { 24180, 59340 },
	{ 24240, 59340 }, { 24240, 59400 }, { 24300, 59400 }, { 24360, 59460 }, { 24360, 59460 }, { 24420, 59520 }, { 24420, 59520 }, { 24480, 59580 }, { 24480, 59580 }, { 24540, 59640 },
	{ 24540, 59640 }, { 24540, 59700 }, { 24600, 59760 }, { 24600, 59760 }, { 24600, 59820 }, { 24600, 59880 }
};

TaskHandle_t      x_task_light_ctrl = NULL;
SemaphoreHandle_t x_sem_light_ctrl = NULL;
SemaphoreHandle_t x_sem_light_state = NULL;
CtrlTime ct;
LightState ls;

CtrlTime get_ctrl_time()
{
	// Get Current Time
	time_t t = time(NULL);

	// Get Midnight
	struct tm *lt = localtime(&t);
	lt->tm_sec = 0;
	lt->tm_min = 0;
	lt->tm_hour = 0;
	time_t t_mn = mktime(lt);

	Ephemeris e = e_data[lt->tm_yday];
	time_t duration = e.sunset - e.sunrise;
	static CtrlTime ct;
	ct.right_on  = t_mn + e.sunrise - duration / 28;
	ct.left_on   = t_mn + e.sunrise + duration / 36;
	ct.right_off = t_mn + e.sunset - duration / 36;
	ct.left_off  = t_mn + e.sunset + duration / 28;
	ct.midnight  = t_mn + 24 * 60 * 60;
	
	return ct;
}

void light_ctrl_set_led_state(LightState ls)
{
	switch(ls){
		case LIGHT_STATE_BOTH_OFF:
			set_ledc_duty_and_time(0, 2000, LEDC_LIGHT_R);
			set_ledc_duty_and_time(0, 2000, LEDC_LIGHT_L);
			break;
		case LIGHT_STATE_RIGHT_ON:
			set_ledc_duty_and_time(127, 2000, LEDC_LIGHT_R);
			set_ledc_duty_and_time(0, 2000, LEDC_LIGHT_L);
			break;
		case LIGHT_STATE_BOTH_ON:
			set_ledc_duty_and_time(127, 2000, LEDC_LIGHT_R);
			set_ledc_duty_and_time(119, 2000, LEDC_LIGHT_L);
			break;
		case LIGHT_STATE_LEFT_ON:
			set_ledc_duty_and_time(0, 2000, LEDC_LIGHT_R);
			set_ledc_duty_and_time(119, 2000, LEDC_LIGHT_L);
			break;
		default:
			break;
	}
}

const int nGPIO[] = {
	1ULL << GPIO_RL_R_RST | 1ULL << GPIO_RL_L_RST, // LR OFF
	1ULL << GPIO_RL_R_SET | 1ULL << GPIO_RL_L_RST, //  R ON
	1ULL << GPIO_RL_R_RST | 1ULL << GPIO_RL_L_SET, // L  ON
	1ULL << GPIO_RL_R_SET | 1ULL << GPIO_RL_L_SET, // LR ON
};

void light_ctrl_set_light_state(LightState ls_new)
{
	// set gpio HI
	REG_WRITE(GPIO_OUT_W1TS_REG, nGPIO[ls_new]);

	// wait 50ms
	vTaskDelay(50 / portTICK_PERIOD_MS);

	// set gpio LO
	REG_WRITE(GPIO_OUT_W1TC_REG, nGPIO[ls_new]);

	light_ctrl_set_led_state(ls_new);
	ls = ls_new;
}

void print_time()
{
	char buf[26];
	ctime_r(&ct.right_on,  buf); buf[24] = '\0';
	ESP_LOGI(TAG, "Right  On: %s", buf);
	ctime_r(&ct.left_on,   buf); buf[24] = '\0';
	ESP_LOGI(TAG, " Left  On: %s", buf);
	ctime_r(&ct.right_off, buf); buf[24] = '\0';
	ESP_LOGI(TAG, "Right Off: %s", buf);
	ctime_r(&ct.left_off,  buf); buf[24] = '\0';
	ESP_LOGI(TAG, " Left Off: %s", buf);
	ctime_r(&ct.midnight,  buf); buf[24] = '\0';
	ESP_LOGI(TAG, " Midnight: %s", buf);
}

void v_light_ctrl_task(void *pvParams)
{
	char buf[26];
	time_t t;
	time_t t_delay;
	ct = get_ctrl_time();

	print_time();

	for(;;)
	{
		xSemaphoreTake(x_sem_light_state, portMAX_DELAY);

		t = time(NULL);
		ctime_r(&ct.midnight,  buf); buf[24] = '\0';
		ESP_LOGI(TAG, "  Current: %s", buf);

		if(t<ct.right_on) {
			light_ctrl_set_light_state(LIGHT_STATE_BOTH_OFF);
			t_delay = ct.right_on - t;
			ESP_LOGI(TAG, "    State: Both OFF");
		} else if(t<ct.left_on) {
			light_ctrl_set_light_state(LIGHT_STATE_RIGHT_ON);
			t_delay = ct.left_on - t;
			ESP_LOGI(TAG, "    State: Right ON");
		} else if(t<ct.right_off) {
			light_ctrl_set_light_state(LIGHT_STATE_BOTH_ON);
			t_delay = ct.right_off - t;
			ESP_LOGI(TAG, "    State: Right and Left ON");
		} else if(t<ct.left_off) {
			light_ctrl_set_light_state(LIGHT_STATE_LEFT_ON);
			t_delay = ct.left_off - t;
			ESP_LOGI(TAG, "    State: Left ON");
		} else if(t<ct.midnight) {
			light_ctrl_set_light_state(LIGHT_STATE_BOTH_OFF);
			t_delay = ct.midnight- t;
			ESP_LOGI(TAG, "    State: Both OFF");
		} else {
			light_ctrl_set_light_state(LIGHT_STATE_BOTH_OFF);
			ct = get_ctrl_time();
			t_delay = ct.right_on - t;
			ESP_LOGI(TAG, "    State: Midnight");
			print_time();
		}

		xSemaphoreGive(x_sem_light_state);

		vTaskDelay(pdMS_TO_TICKS(t_delay * 1000));
	}
}

void light_ctrl_start_task()
{
	xSemaphoreTake(x_sem_light_ctrl, portMAX_DELAY);
	if(x_task_light_ctrl==NULL)
		xTaskCreate(v_light_ctrl_task, "light_ctrl_task", STACK_SIZE, NULL, tskIDLE_PRIORITY, &x_task_light_ctrl);
	xSemaphoreGive(x_sem_light_ctrl);
}

void light_ctrl_stop_task()
{
	xSemaphoreTake(x_sem_light_ctrl, portMAX_DELAY);
	if(x_task_light_ctrl!=NULL){
		vTaskDelete(x_task_light_ctrl);
		x_task_light_ctrl = NULL;
	}
	xSemaphoreGive(x_sem_light_ctrl);
}

void init_light_ctrl_task()
{
	if(x_sem_light_ctrl!=NULL) return;
	x_sem_light_ctrl = xSemaphoreCreateMutex();
	if(x_sem_light_ctrl==NULL) return;

	if(x_sem_light_state!=NULL) return;
	x_sem_light_state = xSemaphoreCreateMutex();
	if(x_sem_light_state==NULL) return;

	light_ctrl_start_task();
}

CtrlTime light_ctrl_get_ctrl_time()
{
	return ct;
}

LightState light_ctrl_get_light_state()
{
	LightState ls_tmp;

	xSemaphoreTake(x_sem_light_state, portMAX_DELAY);
	ls_tmp = ls;
	xSemaphoreGive(x_sem_light_state);

	return ls_tmp;
}

