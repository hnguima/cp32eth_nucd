//  Copyright 2017 by Malte Janduda
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include "ws_log.h"

#include "esp_system.h"
#include "esp_log.h"

#include <string.h>

#define MAX_WS_LEN 2048

static httpd_handle_t http_instance = NULL;

char buf[MAX_WS_LEN];

/*
 * Structure holding server handle
 * and internal socket fd in order
 * to use out of request send
 */

#define MAX_WS_CLIENTS 2

static const char *TAG = "ws_log";

int ws_client[MAX_WS_CLIENTS];
uint8_t ws_client_num = 0;
char *ws_data = NULL;

/*
 * async send function, which we put into the httpd work queue
 */
static void ws_async_send(void *arg)
{
    int *ws_client = (int *)arg;

    char *ws_payload = malloc(strlen(ws_data) + 1);
    strcpy(ws_payload, ws_data);

    httpd_ws_frame_t ws_pkt = {
        .type = HTTPD_WS_TYPE_TEXT,
        .payload = (uint8_t *)ws_payload,
        .len = strlen(ws_payload),
    };

    httpd_ws_send_frame_async(http_instance, *ws_client, &ws_pkt);

    free(ws_payload);
}

esp_err_t ws_async_message(char *data)
{
    esp_err_t err = ESP_OK;
    // copy data into ws_data
    if (ws_data != NULL)
        free(ws_data);

    ws_data = calloc(sizeof(char), strlen(data) + 1);
    strcpy(ws_data, data);

    uint8_t ws_test = 0;
    // insert into work queue for each client
    for (int i = 0; i < MAX_WS_CLIENTS; i++)
    {
        if (httpd_ws_get_fd_info(http_instance, ws_client[i]) == HTTPD_WS_CLIENT_WEBSOCKET)
        {
            err = httpd_queue_work(http_instance, ws_async_send, (void *)&ws_client[i]);
            ws_test++;
        }
    }

    if (ws_test == 0)
    {
        ws_client_num = 0;
    }

    return err;
}

/*
 * This handler echos back the received ws data
 * and triggers an async send if certain message received
 */
static esp_err_t ws_handler(httpd_req_t *req)
{
    uint8_t buf[128] = {0};
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = buf;
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 128);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
        return ret;
    }

    // ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);

    // ESP_LOGI(TAG, "Got packet from %d, clients %d", httpd_req_to_sockfd(req), ws_client_num);

    uint8_t ws_client_exists = 0;
    for (uint8_t i = 0; i < ws_client_num; i++)
    {
        if (ws_client[i] == httpd_req_to_sockfd(req))
        {
            // ESP_LOGW(TAG, "client %d already exists", ws_client[i]);
            ws_client_exists = 1;
            break;
        }
    }

    if (!ws_client_exists && ws_client_num < MAX_WS_CLIENTS)
    {
        ws_client[ws_client_num] = httpd_req_to_sockfd(req);
        ws_client_num++;
    }

    return ESP_OK;
}

static const httpd_uri_t ws = {
    .uri = "/ws",
    .method = HTTP_GET,
    .handler = ws_handler,
    .user_ctx = NULL,
    .is_websocket = true,
};

int ws_log_vprintf(const char *str, va_list l)
{
    int err = 0;
    int len;
    char task_name[16];
    char *cur_task = pcTaskGetTaskName(xTaskGetCurrentTaskHandle());
    strncpy(task_name, cur_task, 16);
    task_name[15] = 0;

    if (strncmp(task_name, "tiT", 16) != 0)
    {
        memset(buf, 0, MAX_WS_LEN);
        vsprintf(buf, str, l);

        ws_async_message(buf);
    }
    return vprintf(str, l);
}

//has to be called before after httpd_start()
int ws_log_init(httpd_handle_t handle)
{

    http_instance = handle;
    httpd_register_uri_handler(http_instance, &ws);

    esp_log_set_vprintf(ws_log_vprintf);

    return 0;
}
