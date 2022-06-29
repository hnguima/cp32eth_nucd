#ifndef __SYSTEM___H
#define __SYSTEM___H

/*
 *Includes - BLibliotecas do sistema e do usuario.
 */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// FreeRTOS
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#define SYSTEM_REBOOT_EVENT BIT2

esp_err_t __err_code__;

#define SYSTEM_ERROR_CHECK(func, err, tag, message)                                  \
    err = func;                                                                      \
    if (err)                                                                         \
    {                                                                                \
        ESP_LOGE(tag, "Funcão %s linha %u --> %s", __FUNCTION__, __LINE__, message); \
    }

#define SYSTEM_RETRY(func, err, tag, interval, count)                                           \
    for (int i = 0; i < count; i++)                                                             \
    {                                                                                           \
        err = func;                                                                             \
        if (err == ESP_OK)                                                                      \
            break;                                                                              \
                                                                                                \
        if (i == count - 1)                                                                     \
            ESP_LOGE(tag, "Funcão %s linha %u --> erro", __FUNCTION__, __LINE__);               \
        else                                                                                    \
            ESP_LOGW(tag, "Funcão %s linha %u --> tentando novamente", __FUNCTION__, __LINE__); \
                                                                                                \
        vTaskDelay(interval / portTICK_PERIOD_MS);                                              \
    }

void system_reboot(uint16_t time_ms);

#endif /* __SYSTEM___H */