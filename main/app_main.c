#include "app_wifi.h"
#include "app_sntp.h"
#include "app_gpio.h"
#include "app_light_ctrl.h"
#include "app_ledc.h"
#include "app_web_serv.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// プログラムのエントリー関数
void app_main(void)
{
	// GPIO 初期化
	init_gpio();

	// LEDC 初期化
	init_ledc();
	
	// WiFi を STA モードで初期化
	init_wifi_sta();

	// WiFI STA を AP に接続
	connect_to_ap(CONFIG_LIGHT_CTRL_WIFI_SSID, CONFIG_LIGHT_CTRL_WIFI_PASSWORD);

	// Set Timezone to JST
	putenv("TZ=JST-9");
	tzset();

	// SNTP モジュールを初期化
	init_sntp(CONFIG_LIGHT_CTRL_SNTP_SERVER);

	// SNTP 同期待ち
	wait_for_sntp_sync();

	// Light Control を初期化
	init_light_ctrl_task();

	// Web Service を開始
	init_web_server();
}

