#include "sender.h"
#include "wifi_connect.h"

#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

/* Hardcoded wifi credentials */
static const char ssid[] = "Gateway-SSID";
static const char passphrase[] = "TestPass!23";

static esp_err_t nvs_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    return ret;
}

void app_main(void)
{
    /* Initialize nvs partition on the system */
    ESP_ERROR_CHECK(nvs_init());
    /* Initialize TCP/IP stack */
    ESP_ERROR_CHECK(esp_netif_init());
    /* Create the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Start wifi and make connection (with hardcoded credential) */
    ESP_ERROR_CHECK(wifi_connect(ssid, passphrase));

    /* Start the sender task */
    sender_start();
}
