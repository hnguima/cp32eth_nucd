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

    return err;
}
