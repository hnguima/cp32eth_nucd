#ifndef __UART___H
#define __UART___H

#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "soc/uart_reg.h"

#include "esp_netif_ip_addr.h"

#include "system.h"
#include "config.h"

// Definição dos pinos de comunicação do canal Modbus RTU 1
#define UART_NUM 1
#define UART_PIN_TXD (GPIO_NUM_16)
#define UART_PIN_RXD (GPIO_NUM_4)
#define UART_PIN_RTS (UART_PIN_NO_CHANGE)
#define UART_PIN_CTS (UART_PIN_NO_CHANGE)
// #define UART_PIN_RTS (GPIO_NUM_5)
// #define UART_PIN_CTS (GPIO_NUM_14)

#define RX_BUF_SIZE 512
#define TX_BUF_SIZE 512

esp_err_t uart_init(cp32eth_data_t *data);

#endif //<-- __UART___H -->