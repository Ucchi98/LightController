#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <esp_system.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_http_server.h>

#include "app_light_ctrl.h"

static const char *TAG = "app_web_serv";

esp_err_t get_favicon_handler(httpd_req_t *hr)
{
	extern const unsigned char ico_favicon_start[] asm("_binary_favicon_ico_start");
	extern const unsigned char ico_favicon_end[]   asm("_binary_favicon_ico_end");
	const size_t ico_favicon_size = ico_favicon_end - ico_favicon_start;

	httpd_resp_set_type(hr, "image/x-icon");
	httpd_resp_send(hr, (const char *)ico_favicon_start, ico_favicon_size);

	return ESP_OK;
}

esp_err_t get_auto_handler(httpd_req_t *hr)
{
	light_ctrl_start_task();

	extern const unsigned char html_auto_start[] asm("_binary_auto_html_start");
	extern const unsigned char html_auto_end[]   asm("_binary_auto_html_end");
	const size_t html_auto_size = html_auto_end - html_auto_start;

	httpd_resp_set_type(hr, "text/html");
	httpd_resp_send(hr, (const char *)html_auto_start, html_auto_size);

	return ESP_OK;
}

esp_err_t get_man_handler(httpd_req_t *hr)
{
	light_ctrl_stop_task();

	extern const unsigned char html_man_start[] asm("_binary_man_html_start");
	extern const unsigned char html_man_end[]   asm("_binary_man_html_end");
	const size_t html_man_size = html_man_end - html_man_start;

	httpd_resp_set_type(hr, "text/html");
	httpd_resp_send(hr, (const char *)html_man_start, html_man_size);

	return ESP_OK;
}

esp_err_t post_man_handler(httpd_req_t *hr)
{
	light_ctrl_stop_task();
	LightState ls = light_ctrl_get_light_state();

	if(hr->content_len!=0){
		char *buf = malloc(hr->content_len + 1);
		if(buf==NULL){
			ESP_LOGI(TAG, "post_man_handler: No Memory.");
			return ESP_FAIL;
		}

		int ret = httpd_req_recv(hr, buf, hr->content_len);
		if(ret<=0){
			if(ret==HTTPD_SOCK_ERR_TIMEOUT){
				httpd_resp_send_408(hr);
			}
			free(buf);
			return ESP_FAIL;
		}
		buf[ret] = '\0';
		ESP_LOGI(TAG, "content = %s", buf);

		char param[8];
		if(httpd_query_key_value(buf, "left", param, sizeof(param))==ESP_OK) {
			ESP_LOGI(TAG, "left = %s", param);
			if(strcmp(param, "off")==0){
				ls &= ~LIGHT_STATE_LEFT_ON;
			} else {
				if(strcmp(param, "on")==0){
					ls |= LIGHT_STATE_LEFT_ON;
				} 
			}
		} else {
			if(httpd_query_key_value(buf, "right", param, sizeof(param))==ESP_OK) {
				ESP_LOGI(TAG, "right = %s", param);
				if(strcmp(param, "off")==0){
					ls &= ~LIGHT_STATE_RIGHT_ON;
				} else  {
					if(strcmp(param, "on")==0){
						ls |= LIGHT_STATE_RIGHT_ON;
					} 
				}
			}
		}
		light_ctrl_set_light_state(ls);
		free(buf);
	}

	char outbuf[64];
	sprintf(outbuf,
			"{\"left\":\"%s\",\"right\":\"%s\"}",
			(ls & LIGHT_STATE_LEFT_ON  ? "off" : "on"),
			(ls & LIGHT_STATE_RIGHT_ON ? "off" : "on"));

	httpd_resp_set_type(hr, "application/json");
	httpd_resp_send(hr, outbuf, HTTPD_RESP_USE_STRLEN);

	return ESP_OK;
}

#define make_ts(_name_, _tm_) _make_ts(_name_, _tm_)
#define _make_ts(_name_, _tm_) \
                char _name_[32];\
                { struct tm *tm = localtime(_tm_);\
                  strftime(_name_, 32, "%y-%m-%d %H:%M:%S", tm); }

esp_err_t get_timetable_handler(httpd_req_t *hr)
{
	const char *st_string[] = {
		"Both Off", "Right On", "Left On", "Both On"
	};

	CtrlTime ct = light_ctrl_get_ctrl_time();

	time_t t = time(NULL);
	make_ts(curr, &t);
	make_ts(ron,  &ct.right_on);
	make_ts(lon,  &ct.left_on);
	make_ts(roff, &ct.right_off);
	make_ts(loff, &ct.left_off);
	make_ts(doy,  &ct.midnight);

	LightState ls = light_ctrl_get_light_state();

	char outbuf[256];
	const char *timetable = "{\"cs\":\"%s\",\"ct\":\"%s\",\"rn\":\"%s\",\"ln\":\"%s\",\"rf\":\"%s\",\"lf\":\"%s\",\"mn\":\"%s\"}";
	sprintf(outbuf, timetable, st_string[ls], curr, ron, lon, roff, loff, doy);

	httpd_resp_set_type(hr, "application/json");
	httpd_resp_send(hr, outbuf, HTTPD_RESP_USE_STRLEN);

	return ESP_OK;
}

httpd_uri_t uri_get_favicon = {
	.uri      = "/favicon.ico",
	.method   = HTTP_GET,
	.handler  = get_favicon_handler,
	.user_ctx = NULL
};

httpd_uri_t uri_get_root = {
	.uri      = "/",
	.method   = HTTP_GET,
	.handler  = get_auto_handler,
	.user_ctx = NULL
};

httpd_uri_t uri_get_auto = {
	.uri      = "/auto",
	.method   = HTTP_GET,
	.handler  = get_auto_handler,
	.user_ctx = NULL
};

httpd_uri_t uri_get_man = {
	.uri      = "/man",
	.method   = HTTP_GET,
	.handler  = get_man_handler,
	.user_ctx = NULL
};

httpd_uri_t uri_post_man = {
	.uri      = "/man",
	.method   = HTTP_POST,
	.handler  = post_man_handler,
	.user_ctx = NULL
};

httpd_uri_t uri_get_timetable = {
	.uri      = "/timetable",
	.method   = HTTP_GET,
	.handler  = get_timetable_handler,
	.user_ctx = NULL
};

httpd_handle_t init_web_server()
{
	httpd_config_t hc = HTTPD_DEFAULT_CONFIG();

	httpd_handle_t hh = NULL;

	if(httpd_start(&hh, &hc)==ESP_OK){
		httpd_register_uri_handler(hh, &uri_get_favicon);
		httpd_register_uri_handler(hh, &uri_get_root);
		httpd_register_uri_handler(hh, &uri_get_auto);
		httpd_register_uri_handler(hh, &uri_get_man);
		httpd_register_uri_handler(hh, &uri_post_man);
		httpd_register_uri_handler(hh, &uri_get_timetable);
	}

	return hh;
}

