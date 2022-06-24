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

#define SYSTEM_ERROR_CHECK(func, err, tag, message) \
    err = func;                                     \
    if (err)                                        \
    {                                               \
        ESP_LOGE(tag,                               \
                 "Funcão %s linha %u --> %s (%s)",  \
                 __FUNCTION__,                      \
                 __LINE__,                          \
                 message,                           \
                 esp_err_to_name(err));             \
    }

void system_reboot(uint8_t timeSeconds);

#endif /* __SYSTEM___H */