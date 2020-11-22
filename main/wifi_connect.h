#ifndef __WIFI_CONNECT_H__
#define __WIFI_CONNECT_H__

#include "esp_err.h"
#include "esp_netif.h"
#include "esp_event.h"

esp_err_t wifi_connect(const char *ssid, const char *passwd);
esp_err_t wifi_disconnect(void);

#endif  /* __WIFI_CONNECT_H__ */