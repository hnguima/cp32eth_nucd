#ifndef __MAIN__H
#define __MAIN__H

/*
 *Includes - BLibliotecas do sistema e do usuario.
 */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/dirent.h>

// FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"

// GPIO Driver
#include "driver/gpio.h"

// Callbacks
#include "esp_event.h"

// Ethernet lib
#include "esp_eth.h"
#include "esp_netif.h"

// Wifi
#include "esp_wifi.h"

// Watchdog
#include "soc/rtc_wdt.h"
#include "esp_task_wdt.h"

// Drivers
#include "driver/gpio.h"
#include "driver/periph_ctrl.h"
#include "esp32/rom/gpio.h"

// Lwip
#include "lwip/err.h"
#include "lwip/sys.h"
#include <lwip/sockets.h>

// NVS
#include "nvs.h"
#include "nvs_flash.h"

// User libs
#include "config.h"
#include "filesystem.h"
#include "uart.h"

// networking
#include "ethernet.h"
#include "http_server.h"
#include "ws_log.h"
// #include "socket_tcp.h"
#include "client_tcp.h"

#define PHY_ENABLE 33
#define LED_STT 32
#define LED_ONLINE 16
#define GPIO_OUTPUT_PIN_SEL ((1ULL << PHY_ENABLE) | (1ULL << LED_STT) | (1ULL << LED_ONLINE))

#define MAX_QUEUE_LENGTH 128

// #define DEBUG_MEMORY

#endif //<-- __MAIN__H -->