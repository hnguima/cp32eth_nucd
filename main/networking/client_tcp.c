#include "client_tcp.h"

// Tag para o log
static const char *TAG = "client_tcp";

void client_tcp_on_connect_cb()
{
    ESP_LOGI(TAG, "conectado ao servidor");
}

void client_tcp_recv_cb(uint8_t *data_rx, int16_t size_rx)
{
    ESP_LOGI(TAG, "Recieved %d bytes", size_rx);
    // log payload hex
    ESP_LOG_BUFFER_HEXDUMP(TAG, data_rx, size_rx, ESP_LOG_INFO);

    uart_write_bytes(UART_NUM, data_rx, size_rx);
}

static void tcp_send_task(void *param){
    client_tcp_data_t *config = (client_tcp_data_t *)param;

    uint8_t *data_tx = (uint8_t *)malloc(TX_BUF_SIZE);

    //Cehck queue for data to send
    while(1){
        if (xQueueReceive(config->tcp_quere, &data_tx, (TickType_t)portMAX_DELAY))
        {
            ESP_LOGI(TAG, "Sending %d bytes", data_tx.size);
            // log payload hex
            ESP_LOG_BUFFER_HEXDUMP(TAG, data_tx.data, data_tx.size, ESP_LOG_INFO);
            // send data
            socket_send(data_tx.data, data_tx.size);
        }
    }
}

    esp_err_t client_tcp_init(client_tcp_data_t *config)
{
    esp_err_t err = ESP_OK;

    client_tcp.addr = config->ip;
    client_tcp.port = config->port;
    client_tcp.is_persistent = true;
    client_tcp.retry_delay = 5000;
    client_tcp.recv_cb = client_tcp_recv_cb;
    client_tcp.on_connect_cb = client_tcp_on_connect_cb;

    err = socket_task_create(&client_tcp, SOCKET_TYPE_CLIENT);

    //Inicializa task que monitora e envia para o servidor
    xTaskCreate(tcp_send_task, "tcp_send_task", 2048, NULL, 5, config);

    return err;
}
