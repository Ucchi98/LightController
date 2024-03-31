#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_sntp.h"
#include "esp_log.h"

#include "app_ledc.h"

// ログ用タグを定義
static const char* TAG = "app_sntp";

void sntp_time_sync_notification_cb(struct timeval *tv)
{
	char buf[26];

	// Set status LED to time synchronization complete
	set_ledc_duty_and_time(0, 2000, LEDC_SNTP_PROC);
	set_ledc_duty_and_time(119, 2000, LEDC_SNTP_DONE);

	// Get Current Time
	time_t t = time(NULL);
	ctime_r(&t,  buf); buf[24] = '\0';
	ESP_LOGI(TAG, "time sync completed at %s", buf);    
}

// SNTP モジュールの初期化関数
void init_sntp(char *sntp_server)
{
	if(sntp_server==NULL) return;

	// SNTP の動作モードを設定
	esp_sntp_setoperatingmode(SNTP_SYNC_MODE_IMMED);

	// SNTP サーバを設定
	esp_sntp_setservername(0, sntp_server);

	// Callback 設定
	esp_sntp_set_time_sync_notification_cb(sntp_time_sync_notification_cb);

	// Initialize SNTP module
	esp_sntp_init();
}

// 時刻が同期するのを待機する関数    
void wait_for_sntp_sync(void)
{
	while(sntp_get_sync_status()!=SNTP_SYNC_STATUS_COMPLETED){
		//ESP_LOGI(TAG, "wait for time sync");

		// wait for 1sec
		vTaskDelay(1000 / portTICK_PERIOD_MS);    
	}
}

