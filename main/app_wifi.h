#ifndef _APP_WIFI_H_
#define _APP_WIFI_H_

#include <esp_err.h>

extern void init_wifi_sta(void);
extern esp_err_t connect_to_ap(char *ssid, char *password);


#endif /* _APP_WIFI_H_ */

