#ifndef __SOCKET_TCP___H
#define __SOCKET_TCP___H

/* Bibliotecas */
#include "esp_system.h"
#include "esp_log.h"
#include "esp_task_wdt.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
/* Definições */
#define CLIENT_MAX_NUMBER 1 //Numero máximo de clientes conectados na mesma porta do socket TCP
#define MAX_SOCKETS 2

#define SOCKET_TYPE_SERVER 0
#define SOCKET_TYPE_CLIENT 1

#define SOCKET_RX_BUF_MAX 1024 
#define SOCKET_TX_BUF_MAX 1024

#define CONNECTED_BIT BIT0
#define REBOOT_BIT BIT1

typedef struct
{
    in_addr_t addr;
    in_port_t port;


    int cli_sock;

    uint8_t buf_rx[SOCKET_RX_BUF_MAX];
    int16_t size_rx;
    uint8_t buf_tx[SOCKET_TX_BUF_MAX];
    int16_t size_tx;

    void (*recv_cb)(uint8_t *data_rx, int16_t size_rx);
    void (*on_connect_cb)();

    bool is_persistent;
    uint8_t retry_max;
    uint16_t retry_delay;
    uint8_t retry;

    SemaphoreHandle_t mutex;

} socket_ctx_t;

/**
 * @brief adiciona conteudo a ser enviado pelo socket, assim que a tas estiver disponpivel o dado 
 * será enviado                                  
 * 
 * @param socket ponteiro para estrutura do socket
 * @param buffer buffer a ser enviado
 * @param size tamanho do dado a ser enviado
 * @return esp_err_t retorna erro caso o socket não esteja aberto ou se o tamanho do dado for 
 *           maior que o suportado
 */
esp_err_t socket_send(socket_ctx_t *socket, uint8_t *buffer, uint16_t size);

esp_err_t socket_task_create(socket_ctx_t *new_socket, uint8_t type);

/**
 * @brief deleta a task associada ao socket, fechando a conexão
 * 
 * @param socket descritor do socket a ser fechado
 * @return esp_err_t retorna o erro que causou o fechamento da conexão
 */
esp_err_t socket_task_delete(int socket);

#endif //<-- __SOCKET_TCP___H -->