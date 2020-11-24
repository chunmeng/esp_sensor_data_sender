#include "sender.h"

#include "esp_timer.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"

#include <string.h>
#include <mbedtls/base64.h>

#define MAX_HTTP_OUTPUT_BUFFER 2048
#define MAX_DATA_BUFFER        1024

static const char *TAG = "sender"; /* debug tag */
/* Assume a cloud http REST API server */ 
static const char *CLOUD_URL = "https://cloud.rigel.com/sensor";

/* A flag to indicate if the system has internet connection */
static bool is_connected = false;

/* Utilities to get and set the flag on event */
static bool has_connection(void)
{
    return is_connected;
}

static void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    is_connected = false;
}

static void connect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    is_connected = true;
}

/**
 * @brief a mock function to read serial from environment storage
 */
static const char *read_env_serial(void)
{
    return "Serial#123";
}

/**
 * @brief a mock function to read raw data from imaging sensor via
 *        io interface, assuming i2c
 * @param databuf   output data buffer
 * @param buflen    size of the data buffer
 *
 * @return Number of read bytes.
 */
static size_t sensor_i2c_read(char *databuf, size_t buflen)
{
    /** TODO: perform data read operation */
    size_t size = 100;
    return size;
}

/**
 * NOTE:
 * This sample uses a synchronous HTTP api call to post the data to the server as a simplified demo flow.
 * In production, it's rarely a good idea to send binary data like this.
 *
 * How the client interact with the server really depends on end to end system design, including cloud
 * setup and capabilities.
 * - Typically the device should connects and authenticate to the data/API server
 * - Then device establishes a long living session connection (e.g. websocket) for data streaming
 * - A data message schema need to be defined for the backend to process the received data.
 *   - Here a basic json message is assumed. This is a text-based message and unsuitable if the data
 *     involved is a large binary blob.
 */
#define DATA_FORMAT_STR "{\"device_id\":\"%s\", \
                          \"ts\":%llu, \
                          \"type\":\"%s\", \
                          \"data\":%s}"

/**
 * @brief a mock function to get the data to send in json format
 * @param databuf   output data buffer
 * @param buflen    size of the data buffer
 *
 * @return OK if data is obtained, else FAILED
 */
esp_err_t sensor_get_json_data(char *databuf, size_t buflen)
{
    int64_t timestamp = esp_timer_get_time();
    /**
     * NOTE:
     * Here the data is assumed to be read from the sensor and encode to base64 output at one go.
     * In practice, these 2 operations should be decoupled in 2 stages.
     */
    char base64buf[MAX_DATA_BUFFER] = {0};
    size_t written = 0;
    char rawdata[MAX_DATA_BUFFER] = {0};
    size_t rawlen = sensor_i2c_read(rawdata, sizeof(rawdata));
    if (rawlen <= 0) {
        ESP_LOGE(TAG, "Unable to read data");
        return ESP_FAIL;
    }
    /* Encode the data blob to a base64 string */
    int ret = mbedtls_base64_encode((unsigned char *) base64buf, sizeof(base64buf),
                                &written, (unsigned char *) rawdata, rawlen);
    if (ret != 0) {
        ESP_LOGE(TAG, "Error in mbedtls encode! ret = -0x%x", -ret);
        return ESP_FAIL;
    }
    /* Construct the json message */
    snprintf(databuf, buflen, DATA_FORMAT_STR, read_env_serial(), timestamp, "image", base64buf);
    return ESP_OK;
}

/* Dummy HTTP events handler */
esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        default:
            ESP_LOGD(TAG, "Other HTTP Event: %d", evt->event_id);
            break;
    }
    return ESP_OK;
}

/**
 * @brief timeout handler
 *
 * At a periodic interval, this callback is invoked to make HTTP API to send the data to the server.
 * There's no buffering/queue mechanism whatsoever - so when there's no connectivity, the data is
 * lost.
 */
static void periodic_timer_callback(void* arg)
{
    if (!has_connection()) {
        ESP_LOGE(TAG, "Internet down... try again later");
        return;
    }

    ESP_LOGI(TAG, "Send data at %lld us...", esp_timer_get_time());
    char data[MAX_HTTP_OUTPUT_BUFFER] = {0};
    /* Get the sensor data in json format */
    if (sensor_get_json_data(data, sizeof(data)) != ESP_OK) {
        ESP_LOGE(TAG, "Error getting sensor data");
        return;
    }
    /* Construct the http client */
    esp_http_client_config_t config = {
        .url = CLOUD_URL,
        .event_handler = http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, data, strlen(data));
    /* Perform API call */
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}

/**
 * @brief start the sender task
 *
 * This sample use a simple periodic timer task, and monitor the connectivity state
 * to determine if data can be sent.
 */
void sender_start(void)
{
    /* Start a 1s periodic timer task */
    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &periodic_timer_callback,
            /* name is optional, but may help identify the timer when debugging */
            .name = "sender_timer"
    };

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    /* Start timer */
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000000));

    /* Register handlers for connectivity event and start/pause send */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, NULL));
}
