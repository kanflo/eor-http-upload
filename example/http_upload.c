/* HTTP upload example
 *
 * This sample code is in the public domain.
 */

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <espressif/esp_common.h>
#include <string.h>
#include <stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <http_upload.h>

#include <ssid_config.h>

// Helpers
#define delay_ms(t) vTaskDelay((t) / portTICK_RATE_MS)
#define systime_ms() (xTaskGetTickCount()*portTICK_RATE_MS)


#define WEB_SERVER "172.16.3.107" // Replace
#define WEB_PORT 80
#define WEB_URL "uload.php"

void http_upload_task(void *pvParameters)
{
    char *chunk1 = "Hello ";
    char *chunk2 = "World!";
    uint32_t upload_size = strlen(chunk1) + strlen(chunk2);
    uint32_t http_status;

    delay_ms(6000);
    printf("--------\n");
    printf("Uploading to %s:%d/%s\n", WEB_SERVER, WEB_PORT, WEB_URL);

    do {
        while (!upload_connect(WEB_SERVER, WEB_PORT)) {
            delay_ms(1000);
        }
        printf("Connected\n");
        if (!upload_begin(WEB_URL, "file.bin", upload_size)) {
            printf("Upload begin failed\n");
            break;
        }

        // Once connected we can upload any number of chunks we like
        // The uploaded size MUST match that of upload_size
        if (!upload_data((void*) chunk1, strlen(chunk1))) {
            printf("Upload of chunk1 failed\n");
            break;
        }
        if (!upload_data((void*) chunk2, strlen(chunk2))) {
            printf("Upload of chunk2 failed\n");
            break;
        }
        http_status = upload_finish();
        if (!http_status) {
            printf("Failed to read HTTP status code\n");
        } else {
            printf("Upload complete, server returned %u\n", http_status);
        }
        upload_close();
    } while(0);
    
    while(1) {
        delay_ms(1000);
    }
}


void user_init(void)
{
    sdk_uart_div_modify(0, UART_CLK_FREQ / 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };

    /* required to call wifi_set_opmode before station_set_config */
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

    xTaskCreate(&http_upload_task, (signed char *) "upload_task", 256, NULL, 2, NULL);
}
