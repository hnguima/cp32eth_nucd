#ifndef __ETHERNET___H
#define __ETHERNET___H 

/**
 * Lib C
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
/**
 * FreeRTOS
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

/**
 * Logs;
 */
#include "esp_system.h"
#include "esp_log.h"

/**
 * Callbacks 
 */
//#include "esp_event_loop.h"
#include "esp_event.h"

/**
 * Ethernet lib
 */
#include "esp_eth.h"
#include "esp_netif.h"

/**
 * Drivers;
 */
#include "esp32/rom/gpio.h"
#include "driver/gpio.h"
#include "driver/periph_ctrl.h"

#define CONNECTED_BIT BIT0
#define REBOOT_BIT BIT1

typedef struct
{
    int32_t ip;
    int32_t mask;
    int32_t gateway;

} ip_data_t;

void ethernet_init(ip_data_t *config);
// void ethernet_init(void);

#endif //<-- __ETHERNET___H -->