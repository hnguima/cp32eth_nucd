#ifndef __CONFIG__H
#define __CONFIG__H 

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
 * CJSON
 */
#include "cJSON.h"

/**
 * User Libs
 */
#include "filesystem.h"
// #include "http_server.h"


/**
 * Defines
 */
#define SYSTEM_REBOOT_EVENT BIT2

#define CONFIG_FILE "/data/config"
#define CONFIG_DEFAULT_FILE "/data/default"
#define CONFIG_SAVE_DELAY 5 * 60 * 1000 // 5 minutes

typedef struct
{
    char *description;
    char *hw_version;
    char *fw_version;
    char *mac_addr;

    
    uint16_t resets;
    uint16_t config_updates;
    uint16_t if_updates;
    uint16_t fw_updates;

    uint32_t up_time;
    uint32_t init_time;
    uint32_t install_time;

    uint32_t available_mem;

} info_data_t;

typedef struct
{
    char *username;
    char *password;

} login_data_t;

typedef struct
{
    int32_t ip;
    int32_t mask;
    int32_t gateway;

} ip_data_t;

typedef struct
{

    uint8_t enabled;

    char *ssid;
    char *password;

} wifi_data_t;

typedef struct
{
    int32_t ip;
    uint32_t port;

    QueueHandle_t queue;

} client_tcp_data_t;

typedef struct
{
    uint8_t *data;
    int16_t size;

} queue_data_t;
typedef struct
{

    info_data_t *info;
    login_data_t *login;
    login_data_t *root;

    ip_data_t *ip;
    wifi_data_t *wifi;
    client_tcp_data_t *socket;

    QueueHandle_t uart_queue;

} cp32eth_data_t;

typedef enum
{
    CONFIG_ALL = 0,
} config_parse_e;

esp_err_t config_load(cp32eth_data_t *cp32eth);
esp_err_t config_load_json(cp32eth_data_t *cp32eth, cJSON *config_json);
cJSON *config_parse(cp32eth_data_t *cp32eth, config_parse_e option);
esp_err_t config_save(cp32eth_data_t *cp32eth);

#endif //<-- __CONFIG__H -->