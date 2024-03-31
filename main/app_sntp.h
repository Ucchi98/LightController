#ifndef _APP_SNTP_H_
#define _APP_SNTP_H_

#include "lwip/err.h"
#include "lwip/sys.h"
//#include "lwip/apps/sntp.h"
#include "esp_sntp.h"

extern void init_sntp(char *sntp_server);
extern void wait_for_sntp_sync(void);


#endif /* _APP_SNTP_H_ */

