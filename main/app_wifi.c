#include <string.h>

// FreeRTOS 関連
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

// ESP 関連
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

// LWIP 関連
#include "lwip/err.h"
#include "lwip/sys.h"

#include "app_ledc.h"
#include "driver/ledc.h"

// ログ用タグ
static const char *TAG = "app_wifi";
static SemaphoreHandle_t sh_wait_for_ip = NULL;

// Wifi 関連イベントハンドラ関数
static void on_wifi_event(void *arg, esp_event_base_t event_base,
		int32_t event_id, void *event_data)
{
	switch(event_id){
		case WIFI_EVENT_WIFI_READY:
			ESP_LOGI(TAG, "Wi-Fi ready.");
			break;

		case WIFI_EVENT_STA_START:
			set_ledc_duty_and_time(0, 2000, LEDC_SNTP_DONE);
			set_ledc_duty_and_time(127, 200, LEDC_SNTP_PROC);
			ESP_LOGI(TAG, "Wi-Fi Station started.");
			break;

		case WIFI_EVENT_STA_STOP:
			ESP_LOGI(TAG, "Wi-Fi Station stoped.");
			break;

		case WIFI_EVENT_STA_CONNECTED:
			set_ledc_duty_and_time(0, 2000, LEDC_SNTP_DONE);
			set_ledc_duty_and_time(127, 500, LEDC_SNTP_PROC);
			ESP_LOGI(TAG, "Wi-Fi Station connected to AP.");
			break;

		case WIFI_EVENT_STA_DISCONNECTED:
			set_ledc_duty_and_time(0, 2000, LEDC_SNTP_DONE);
			set_ledc_duty_and_time(127, 500, LEDC_SNTP_PROC);
			ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
			ESP_ERROR_CHECK(esp_wifi_connect());
			break;

		default:
			ESP_LOGI(TAG, "Wi-Fi any event.");
			break;
	}
}

// IP 関連イベントハンドラ関数
static void on_ip_event(void *arg, esp_event_base_t event_base,
		int32_t event_id, void *event_data)
{
	switch(event_id){
		case IP_EVENT_STA_GOT_IP:
			set_ledc_duty_and_time(0, 2000, LEDC_SNTP_DONE);
			set_ledc_duty_and_time(127, 1000, LEDC_SNTP_PROC);
			xSemaphoreGive(sh_wait_for_ip);
			ESP_LOGI(TAG, "Wi-Fi Station got ip address.");
			break;

		case IP_EVENT_STA_LOST_IP:
			set_ledc_duty_and_time(0, 2000, LEDC_SNTP_DONE);
			set_ledc_duty_and_time(127, 500, LEDC_SNTP_PROC);
			ESP_LOGI(TAG, "Wi-Fi Station lost ip address.");
			break;

		default:
			ESP_LOGI(TAG, "IP any event.");
			break;
	}
}

// WiFi STA 初期化関数
void init_wifi_sta(void)
{
	//Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	esp_err_t res;

	// TCP/IP スタックを初期化
	res = esp_netif_init();
	ESP_ERROR_CHECK(res);

	// イベントループを作成
	res = esp_event_loop_create_default();
	ESP_ERROR_CHECK(res);

	esp_netif_create_default_wifi_sta();

	// デフォルト値で Wifi を初期化
	wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
	res = esp_wifi_init(&wifi_init_cfg);
	ESP_ERROR_CHECK(res);

	// WIFI 関連イベントに対応するイベントハンドラを登録
	res = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &on_wifi_event, NULL);
	ESP_ERROR_CHECK(res);

	// IP 関連イベントに対応するイベントハンドラを登録
	res = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &on_ip_event, NULL);
	ESP_ERROR_CHECK(res);
}

// WiFi AP に接続する関数
#include <strings.h>
esp_err_t connect_to_ap(char *ssid, char *password)
{
	sh_wait_for_ip = xSemaphoreCreateBinary();
	if(sh_wait_for_ip==NULL)
	{
		return ESP_ERR_NO_MEM;
	}

	wifi_config_t wifi_config = {};

	// SSID を設定
	strncpy((char *)wifi_config.sta.ssid, ssid, 32);

	// Password を設定
	strncpy((char *)wifi_config.sta.password, password, 64);

	// 認証モードを設定
	wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

	// Wifi STA を開始
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	// Wifi AP に接続
	esp_wifi_connect();

	// IP Address 取得を待機
	xSemaphoreTake(sh_wait_for_ip, portMAX_DELAY);

	return ESP_OK;
}

