#include "uart.h"

static const char *TAG = "uart";

static QueueHandle_t uart_queue;

char get_version_command[] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x00, 0x00, 0x03, 0x9A, 0x0E};
char get_mac_command[] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x00, 0x00, 0x03, 0x59, 0xAF};

static void uart_event_task(void *param)
{
    cp32eth_data_t *data = (cp32eth_data_t *)param;

    uart_event_t event;
    uint8_t *dtmp = (uint8_t *)malloc(RX_BUF_SIZE);
    for (;;)
    {
        // Waiting for UART event.
        if (xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY))
        {
            bzero(dtmp, RX_BUF_SIZE);
            // ESP_LOGI(TAG, "uart[%d] event: %d", UART_NUM, event.type);
            switch (event.type)
            {
            // Event of UART receving data
            /*We'd better handler data event fast, there would be much more data events than
            other types of events. If we take too much time on data event, the queue might
            be full.*/
            case UART_DATA:
                uart_read_bytes(UART_NUM, dtmp, event.size, portMAX_DELAY);

                // ESP_LOG_BUFFER_HEXDUMP(TAG, dtmp, event.size, ESP_LOG_INFO);

                if (strstr((char *)dtmp, "ABCDEFGHIJKLMNOPQ") != NULL)
                {
                    if (dtmp[17] == 'r')
                    {
                        // limpa dados de configura��o
                        // reinicializa em 6s
                        ESP_LOGI(TAG, "Recebeu comando de limpar dados de configuracao");
                    }
                    else if (dtmp[17] == 's')
                    {
                        // obtem 128 bytes (dados de configura��o de par�metros de rede),
                        // verifica dados e reinicializa em 6s se for nova config (reinicializa rede),
                        // caso contr�rio apenas reinicializa protocolo
                        ESP_LOGI(TAG, "Recebeu comando de obter dados de configuracao");

                        uint8_t *config_str = &dtmp[18];

                        uint8_t ip_str[16], mask_str[16], gw_str[16], host_ip[16];
                        uint16_t host_port;

                        memcpy(ip_str, config_str, 16);
                        data->ip->ip = esp_ip4addr_aton((char *)ip_str);
                        memcpy(mask_str, &config_str[20], 16);
                        data->ip->mask = esp_ip4addr_aton((char *)mask_str);
                        memcpy(gw_str, &config_str[40], 16);
                        data->ip->gateway = esp_ip4addr_aton((char *)gw_str);

                        memcpy(host_ip, &config_str[60], 16);
                        data->socket->ip = esp_ip4addr_aton((char *)host_ip);
                        host_port = *(uint16_t *)&config_str[80];
                        data->socket->port = host_port;

                        // printf("ip: %s, mask: %s, gw: %s, host_ip: %s, host_port: %d\n", ip_str, mask_str, gw_str, host_ip, host_port);
                        // printf("ip: %d, mask: %d, gw: %d, host_ip: %d, host_port: %d\n", data->ip->ip, data->ip->mask, data->ip->gateway, data->socket->ip, data->socket->port);

                        config_save(data);
                        system_reboot(6000);
                    }
                    else if (dtmp[17] == 'R')
                    {
                        // obtem 128 bytes (dados de opera��o para apresenta��o em interface http)
                        // e reinicializa protocolo
                        ESP_LOGI(TAG, "Recebeu comando de obter dados de operacao");
                    }
                } // ABCDEFGHIJKLMNOPQR
                else if (strstr((char *)dtmp, get_version_command) != NULL)
                {
                    uint8_t version[3];
                    sscanf(data->info->fw_version, "v%hhu.%hhu.%hhu", &version[0], &version[1], &version[2]);

                    uint8_t version_str[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, version[0], version[1], version[2], 0x00, 0x00, 0x00};
                    uart_write_bytes(UART_NUM, version_str, 15);
                }
                else if (strstr((char *)dtmp, get_mac_command) != NULL)
                {
                    uint8_t mac[6];
                    sscanf(data->info->mac_addr, "%hhu:%hhu:%hhu:%hhu:%hhu:%hhu", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

                    uint8_t mac_str[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], 0x00, 0x00, 0x00};
                    uart_write_bytes(UART_NUM, mac_str, 15);
                }
                else
                {
                    queue_data_t *queue_data = (queue_data_t *)malloc(sizeof(queue_data_t));

                    // copy data from dtmp
                    queue_data->data = (uint8_t *)malloc(event.size);
                    memcpy(queue_data->data, dtmp, event.size);
                    queue_data->size = event.size;

                    if (xQueueSend(data->socket->queue, (void *)&queue_data, (TickType_t)portMAX_DELAY) != pdTRUE)
                    {
                        ESP_LOGE(TAG, "Failed to send to the queue");
                        free(queue_data);
                    }
                }
                // if (dtmp[0] == '\b')
                // {
                //     ESP_LOGI(TAG, "caiu aqui");
                //     uart_write_bytes(UART_NUM, "\b \b", 3);
                // }
                // else
                // {
                //     uart_write_bytes(UART_NUM, (const char *)dtmp, event.size);
                // }
                break;
            // Others
            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

esp_err_t uart_init(cp32eth_data_t *data)
{
    esp_err_t err;

    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 20,
    };

    SYSTEM_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config), err, TAG, "UART param config error");
    SYSTEM_ERROR_CHECK(uart_set_pin(UART_NUM, UART_PIN_TXD, UART_PIN_RXD, UART_PIN_RTS, UART_PIN_CTS),
                       err, TAG, "UART pin config error");
    SYSTEM_ERROR_CHECK(uart_driver_install(UART_NUM, RX_BUF_SIZE * 2, TX_BUF_SIZE * 2, 20, &uart_queue, 0),
                       err, TAG, "UART driver install error");
    SYSTEM_ERROR_CHECK(uart_set_mode(UART_NUM, UART_MODE_UART), err, TAG, "UART mode error");
    SYSTEM_ERROR_CHECK(uart_set_rx_timeout(UART_NUM, 100), err, TAG, "UART set rx timeout error"); // Tempo para considerar o pacote finalizado. Cada 1 unidade representa ~11bits da recepção.

    // Create a task to handler UART event from ISR
    xTaskCreate(uart_event_task, "uart_event_task", 4096, data, 12, NULL);

    return err;
}