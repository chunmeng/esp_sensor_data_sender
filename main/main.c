#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "wifi_connect.h"

static const char ssid[] = "Gateway-SSID";
static const char passphrase[] = "TestPass!23";

void app_main(void)
{
    /* initialize nvs partition on the system */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    /* initialize TCP/IP stack */
    ESP_ERROR_CHECK(esp_netif_init());
    /* create the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Start wifi and make connection (with hardcoded credential) */
    ESP_ERROR_CHECK(wifi_connect(ssid, passphrase));
}
