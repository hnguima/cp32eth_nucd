#include <protobuf-c/protobuf-c.h>
#include "http_server.h"

static esp_err_t content_handler(httpd_req_t *req);
static esp_err_t config_handler(httpd_req_t *req);
static esp_err_t load_handler(httpd_req_t *req);
static esp_err_t reset_handler(httpd_req_t *req);
static esp_err_t OTA_status_handler(httpd_req_t *req);
static esp_err_t OTA_upload_handler(httpd_req_t *req);

#define HTTP_URI(__uri, __method, __handler, __user_ctx) \
    {                                                    \
        .uri = __uri,                                    \
        .method = __method,                              \
        .handler = __handler,                            \
        .user_ctx = __user_ctx,                          \
    }

httpd_uri_t l_http_data[] = {
    HTTP_URI("/", HTTP_GET, content_handler, NULL),
    HTTP_URI("/config", HTTP_GET, content_handler, NULL),
    HTTP_URI("/log", HTTP_GET, content_handler, NULL),
    HTTP_URI("/img/*", HTTP_GET, content_handler, NULL),
    HTTP_URI("/js/*", HTTP_GET, content_handler, NULL),
    HTTP_URI("/style/*", HTTP_GET, content_handler, NULL),
    HTTP_URI("/save_config", HTTP_PUT, config_handler, NULL),
    HTTP_URI("/load_config", HTTP_GET, load_handler, NULL),
    HTTP_URI("/load_all", HTTP_GET, load_handler, NULL),
    HTTP_URI("/upload", HTTP_POST, OTA_upload_handler, NULL),
    HTTP_URI("/status", HTTP_POST, OTA_status_handler, NULL),
    HTTP_URI("/reset", HTTP_PUT, reset_handler, NULL),
};

#define HTTP_DATA_MAX_INDEX (sizeof(l_http_data) / sizeof(l_http_data[0]))

httpd_handle_t http_instance = NULL;

static const char *TAG = "http_server";

// httpd_handle_t OTA_server = NULL;
int8_t flash_status = 0;

uint8_t is_logged = 0;
bool config_changed = true;
bool is_root = false;

login_data_t *user_info = NULL;
login_data_t *admin_info = NULL;

/* Send OTA status */
esp_err_t OTA_status_handler(httpd_req_t *req)
{
    char ledJSON[100];

    ESP_LOGI("OTA", "Status Requested");

    sprintf(ledJSON, "{\"status\":%d,\"compile_time\":\"%s\",\"compile_date\":\"%s\"}", flash_status, __TIME__, __DATE__);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, ledJSON, strlen(ledJSON));

    // This gets set when upload is complete
    if (flash_status == 1)
    {
        // O sistema não pode ser reiniciado diretamente
        // Pois a interface precisa receber a resposta desta solicitação
        system_reboot(2);
    }

    return ESP_OK;
}

/* Receive .Bin file */
esp_err_t OTA_upload_handler(httpd_req_t *req)
{
    esp_ota_handle_t ota_handle;

    char ota_buff[1024];
    int content_length = req->content_len;
    int content_received = 0;
    int recv_len;
    bool is_req_body_started = false;
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);

    // Unsucessful Flashing
    flash_status = -1;

    do
    {
        /* Read the data for the request */
        if ((recv_len = httpd_req_recv(req, ota_buff, MIN(content_length, sizeof(ota_buff)))) < 0)
        {
            if (recv_len == HTTPD_SOCK_ERR_TIMEOUT)
            {
                ESP_LOGI("OTA", "Socket Timeout");
                /* Retry receiving if timeout occurred */
                continue;
            }
            ESP_LOGI("OTA", "OTA Other Error %d", recv_len);
            return ESP_FAIL;
        }

        // Is this the first data we are receiving
        // If so, it will have the information in the header we need.
        if (!is_req_body_started)
        {
            is_req_body_started = true;

            // Lets find out where the actual data staers after the header info
            char *body_start_p = strstr(ota_buff, "\r\n\r\n") + 4;
            int body_part_len = recv_len - (body_start_p - ota_buff);

            int i = *body_start_p;
            // ESP_LOGI("OTA", "OTA RX: %d of %d\r", content_received, content_length);

            esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
            if (err != ESP_OK)
            {
                ESP_LOGI("OTA", "Error With OTA Begin, Cancelling OTA. (%s)", esp_err_to_name(err));
                return ESP_FAIL;
            }
            else
            {
                ESP_LOGI("OTA", "Writing to partition subtype %d at offset 0x%x", update_partition->subtype, update_partition->address);
            }

            // Lets write this first part of data out
            esp_ota_write(ota_handle, body_start_p, body_part_len);
        }
        else
        {
            // Write OTA data
            esp_ota_write(ota_handle, ota_buff, recv_len);
            // ESP_LOGI("OTA", "OTA RX: %d of %d\r", content_received, content_length);
            // content_received += recv_len;
        }

        content_received += recv_len;

    } while (recv_len > 0 && content_received < content_length);

    ESP_LOGI("OTA", "OTA Data RX: %d of %d\r", content_received, content_length);

    // End response
    httpd_resp_send_chunk(req, NULL, 0);

    esp_app_desc_t *app_description = esp_ota_get_app_description();
    esp_app_desc_t update_description;
    if (esp_ota_get_partition_description(update_partition, &update_description) != ESP_OK)
    {
        return ESP_FAIL;
    }

    if (strcmp(app_description->project_name, update_description.project_name) == 0)
    {

        if (esp_ota_end(ota_handle) == ESP_OK)
        {
            // Lets update the partition
            if (esp_ota_set_boot_partition(update_partition) == ESP_OK)
            {
                const esp_partition_t *boot_partition = esp_ota_get_boot_partition();

                // Webpage vai requerir o status quando completo o update
                // Esta variavel é para ela saber que o update foi bem sucedido
                flash_status = 1;

                // Atualiza o numero de updates

                cp32eth->info->fw_updates++;

                ESP_LOGI("OTA", "Next boot partition subtype %d at offset 0x%x", boot_partition->subtype, boot_partition->address);
                ESP_LOGI("OTA", "Please Restart System...");
            }
            else
            {
                ESP_LOGI("OTA", "\r\n\r\n !!! Flashed Error !!!");
            }
        }
        else
        {
            ESP_LOGI("OTA", "!!! OTA End Error !!!");
        }
    }
    else
    {

        ESP_LOGI("OTA", "Firmware invalido!!!");
    }

    return ESP_OK;
}

static char *http_auth_basic(const char *username, const char *password)
{
    int out;
    char user_info[32];
    char *digest = NULL;
    size_t n = 0;

    sprintf(user_info, "%s:%s", username, password);

    esp_crypto_base64_encode(NULL, 0, &n, (const unsigned char *)user_info, strlen(user_info));

    /* 6: The length of the "Basic " string
     * n: Number of bytes for a base64 encode format
     * 1: Number of bytes for a reserved which be used to fill zero
     */
    digest = malloc(sizeof(char) * (6 + n + 1));
    if (digest)
    {
        strcpy(digest, "Basic ");
        esp_crypto_base64_encode((unsigned char *)digest + 6, n, (size_t *)&out, (const unsigned char *)user_info, strlen(user_info));
    }
    return digest;
}

static esp_err_t content_handler(httpd_req_t *req)
{

    // ESP_LOGI(TAG, "called %s", req->uri);

    esp_err_t err = ESP_OK;
    char *buf = NULL;
    size_t buf_len = 0;

    buf_len = httpd_req_get_hdr_value_len(req, "Authorization") + 1;
    if (!(buf_len > 1))
    {
        ESP_LOGE(TAG, "No auth header received");
        httpd_resp_set_status(req, HTTPD_401);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_set_hdr(req, "Connection", "keep-alive");
        httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"User interface\"");
        httpd_resp_send(req, NULL, 0);

        return ESP_ERR_HTTPD_INVALID_REQ;
    }

    buf = malloc(buf_len);
    if (!buf)
    {
        ESP_LOGE(TAG, "No enough memory for basic authorization");
        free(buf);
        return ESP_ERR_NO_MEM;
    }

    if (httpd_req_get_hdr_value_str(req, "Authorization", buf, buf_len) != ESP_OK)
    {
        ESP_LOGE(TAG, "No auth value received");
        free(buf);
        return ESP_FAIL;
    }

    char *auth_credentials_user = http_auth_basic(user_info->username, user_info->password);
    char *auth_credentials_admin = http_auth_basic(admin_info->username, admin_info->password);
    if (!auth_credentials_user || !auth_credentials_admin)
    {
        ESP_LOGE(TAG, "No enough memory for basic authorization credentials");
        free(buf);
        return ESP_ERR_NO_MEM;
    }

    if (strncmp(auth_credentials_user, buf, buf_len) == 0 || strncmp(auth_credentials_admin, buf, buf_len) == 0)
    {
        if (strncmp(auth_credentials_admin, buf, buf_len) == 0)
        {
            is_root = true;
        }

        char filepath[32] = "/server";

        if (strcmp(req->uri, "/") == 0)
        {
            strcat(filepath, "/index.html");
        }
        else if (strcmp(req->uri, "/config") == 0)
        {
            strcat(filepath, "/config.html");
        }
        else if (strcmp(req->uri, "/log") == 0)
        {
            strcat(filepath, "/log.html");
        }
        else
        {
            strcat(filepath, req->uri);
        }

        fs_mount();

        uint32_t size = 0;
        char *filebuffer = fs_read_file(filepath, &size);

        if (filebuffer)
        {
            if (strstr(filepath, ".html"))
            {
                httpd_resp_set_type(req, "text/html");
            }
            if (strstr(filepath, ".css"))
            {
                httpd_resp_set_type(req, "text/css");
            }
            else if (strstr(filepath, ".js"))
            {
                httpd_resp_set_type(req, "text/javascript");
            }
            else if (strstr(filepath, ".svg"))
            {
                httpd_resp_set_type(req, "image/svg+xml");
            }

            httpd_resp_set_status(req, HTTPD_200);
            httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
            httpd_resp_send(req, filebuffer, size);

            free(filebuffer);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to open file for reading %s", filepath);
            httpd_resp_set_status(req, HTTPD_404);
            httpd_resp_send(req, NULL, 0);
            err = ESP_FAIL;
        }
    }
    else
    {
        ESP_LOGE(TAG, "Not authenticated");
        httpd_resp_set_status(req, HTTPD_401);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_set_hdr(req, "Connection", "keep-alive");
        httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"User interface\"");
        httpd_resp_send(req, NULL, 0);

        err = ESP_FAIL;
    }

    free(auth_credentials_user);
    free(auth_credentials_admin);
    free(buf);

    return err;
}

static esp_err_t html_update_handler(httpd_req_t *req)
{
    // ESP_LOGI(TAG, "%s called", req->uri);

    esp_err_t err = ESP_OK;
    char *req_content = malloc(req->content_len);
    httpd_req_recv(req, req_content, req->content_len);

    if (is_root)
    {

        char slice_str[5];
        httpd_req_get_hdr_value_str(req, "Slice-index", slice_str, 5);
        uint8_t slice = atoi(slice_str);

        char file_path[32] = "/server";
        strcat(file_path, req->uri);

        FILE *f;

        if (slice == 0)
        {
            f = fopen(file_path, "w");
        }
        else
        {
            f = fopen(file_path, "a");
        }

        if (f == NULL)
        {
            ESP_LOGE(TAG, "Failed to open file for writing");
            err = ESP_ERR_NOT_FOUND;
        }
        else
        {
            fwrite((void *)req_content, 1, req->content_len, f);
            fclose(f);

            const char resp[] = "Success";
            httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
        }
    }
    else
    {
        ESP_LOGE(TAG, "Not authenticated");
        httpd_resp_set_status(req, HTTPD_401);
        httpd_resp_send(req, NULL, 0);

        err = ESP_FAIL;
    }

    free(req_content);

    return ESP_OK;
}

static esp_err_t config_handler(httpd_req_t *req)
{

    char *req_content = calloc(req->content_len + 1, sizeof(char));

    uint16_t bytes_read = 0;

    while (bytes_read < req->content_len)
    {
        uint16_t ret = httpd_req_recv(req, req_content + bytes_read, req->content_len);
        if (ret <= 0)
        {
            ESP_LOGE(TAG, "Error receiving data");
            free(req_content);
            return ESP_FAIL;
        }
        bytes_read += ret;
    }

    char restart[10];
    httpd_req_get_hdr_value_str(req, "Restart", restart, 10);

    cJSON *body = cJSON_Parse(req_content);
    free(req_content);

    if (!body)
    {
        ESP_LOGE(TAG, "Failed to parse JSON");
        httpd_resp_set_status(req, HTTPD_400);
        httpd_resp_send(req, NULL, 0);
        return ESP_FAIL;
    }

    // saving old config
    cJSON *old_config_json = config_parse(cp32eth, CONFIG_ALL);
    char *old_config_str = cJSON_Print(old_config_json);
    cJSON_Delete(old_config_json);

    fs_write_file("/data/old_config", old_config_str);
    free(old_config_str);

    config_load_json(cp32eth, body);
    cJSON_Delete(body);

    const char resp[] = "Success";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    if (strstr(restart, "true") != NULL)
    {
        config_save(cp32eth);
        system_reboot(5);
    }

    httpd_resp_set_status(req, HTTPD_200);
    httpd_resp_send(req, NULL, 0);

    return ESP_OK;
}

static esp_err_t load_handler(httpd_req_t *req)
{
    // ESP_LOGI(TAG, "%s called", req->uri);

    char *body_str = NULL;

    char config_type[10];
    httpd_req_get_hdr_value_str(req, "Config-type", config_type, 10);

    if (strcmp(config_type, "all") == 0)
    {
        cJSON *data_json = config_parse(cp32eth, CONFIG_ALL);
        body_str = cJSON_Print(data_json);
        cJSON_Delete(data_json);
    }
    else if (strcmp(config_type, "old") == 0)
    {
        body_str = fs_read_file("/data/old_config", NULL);
    }
    else if (strcmp(config_type, "factory") == 0)
    {
        body_str = fs_read_file("/data/default", NULL);
    }

    if (!is_root)
    {
        cJSON *body_json = cJSON_Parse(body_str);
        free(body_str);

        cJSON_DeleteItemFromObject(body_json, "admin");

        body_str = cJSON_Print(body_json);
        cJSON_Delete(body_json);
    }

    httpd_resp_set_type(req, "application/json");

    if (!body_str)
    {
        body_str = malloc(sizeof(char));
    }
    httpd_resp_send(req, body_str, HTTPD_RESP_USE_STRLEN);

    free(body_str);

    return ESP_OK;
}

static esp_err_t reset_handler(httpd_req_t *req)
{
    // ESP_LOGI(TAG, "%s called", req->uri);

    char reset_type[10];
    httpd_req_get_hdr_value_str(req, "ResetType", reset_type, 10);

    char access[5] = "user";
    if (is_root)
        strcpy(access, "root");

    if (strcmp(reset_type, "factory") == 0)
    {
        // fix implementar função para reset de fábrica

        // fs_revert_to_default(access);
    }

    system_reboot(5);

    return ESP_OK;
}

httpd_handle_t http_server_init(cp32eth_data_t *config)
{
    cp32eth = config;

    esp_err_t err = ESP_OK;

    httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
    http_config.uri_match_fn = httpd_uri_match_wildcard;
    http_config.max_uri_handlers = HTTP_DATA_MAX_INDEX + 1;
    http_config.backlog_conn = 10;
    http_config.max_open_sockets = 3;
    http_config.lru_purge_enable = true;
    http_config.stack_size = 8192;

    user_info = config->login;
    admin_info = config->root;

    if (user_info == NULL || admin_info == NULL)
    {
        ESP_LOGE(TAG, "Falha ao obter as informações de login");
        return NULL;
    }

    if (httpd_start(&http_instance, &http_config) == ESP_OK)
    {

        for (uint8_t i = 0; i < HTTP_DATA_MAX_INDEX; i++)
        {
            httpd_uri_t content_uri = l_http_data[i];
            SYSTEM_ERROR_CHECK(httpd_register_uri_handler(http_instance, &content_uri), err, TAG, "Erro ao registrar uri");
        }

        ESP_LOGI(TAG, "Server Started");
    }
    else
    {
        ESP_LOGI(TAG, "Error starting http server...");
        return NULL;
    }

    return http_instance;
}

void http_server_stop(void)
{
    esp_err_t err = ESP_OK;

    if (http_instance != NULL)
    {
        SYSTEM_ERROR_CHECK(httpd_stop(http_instance), err, TAG, "Erro ao parar o servidor");
        ESP_LOGI(TAG, "Server Stopped");
    }

    is_logged = 0;
}
