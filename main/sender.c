#include "sender.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "esp_log.h"

/* Note:

*/

static const char *TAG = "sender"; /* debug tag */

static void timer_callback(TimerHandle_t timer)
{
    volatile int *count;
    count = (volatile int *)pvTimerGetTimerID( timer );
    (*count)++;
    ESP_LOGI(TAG, "Do work...");
}

void sender_start(void)
{
    volatile int count = 0;
    /* Start a 1s periodic timer task */
    xTimerCreate("sender_task", 1000 / portTICK_PERIOD_MS, pdTRUE,
                                        (void *)&count, timer_callback);
}
