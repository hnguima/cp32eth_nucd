#ifndef __CLIENT_TCP__H
#define __CLIENT_TCP__H

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
#include "esp_timer.h"

#include "socket_tcp.h"
#include "uart.h"

socket_ctx_t client_tcp;

typedef struct
{
    uint32_t ip;
    uint32_t port;

} client_tcp_data_t;

esp_err_t client_tcp_init(client_tcp_data_t *config);

#endif //<-- __CLIENT_TCP__H -->