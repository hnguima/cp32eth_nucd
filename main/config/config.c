#include "config.h"

static const char *TAG = "config";

static void config_save_task(void *param);
esp_err_t config_save(cp32eth_data_t *cp32eth);

static void config_save_task(void *param)
{

    cp32eth_data_t *cp32eth = (cp32eth_data_t *)param;

    while (1)
    {
        config_save(cp32eth);

        vTaskDelay(CONFIG_SAVE_DELAY / portTICK_PERIOD_MS);
    }
}

esp_err_t config_load(cp32eth_data_t *cp32eth)
{
    // Initializing configs
    esp_err_t err = ESP_OK;

    if (!cp32eth)
    {
        ESP_LOGE(TAG, "Estrutura cp32eth não foi alocada");
        return ESP_FAIL;
    }

    fs_mount();

    // Read config file
    char *config_str = fs_read_file(CONFIG_FILE, NULL);
    cJSON *config_json = cJSON_Parse(config_str);
    free(config_str);

    if (config_json == NULL)
    {
        ESP_LOGW(TAG, "arqivo de configuração não encontrado, carregando default");

        config_str = fs_read_file(CONFIG_DEFAULT_FILE, NULL);
        config_json = cJSON_Parse(config_str);
        free(config_str);

        if (config_json == NULL)
        {
            ESP_LOGE(TAG, "arquivo de configuração default não encontrado");
            return ESP_FAIL;
        }
    }

    config_load_json(cp32eth, config_json);

    cJSON_Delete(config_json);

    // create saving task
    xTaskCreate(config_save_task, "config_save_task", 4096, cp32eth, 5, NULL);

    return err;
}

esp_err_t config_save(cp32eth_data_t *cp32eth)
{
    if (!cp32eth)
    {
        ESP_LOGE(TAG, "Estrutura cp32eth não foi alocada");
        return ESP_FAIL;
    }

    cJSON *config_json = config_parse(cp32eth, CONFIG_ALL);
    if (!config_json)
    {
        ESP_LOGE(TAG, "Erro ao parsear configuração");
        return ESP_FAIL;
    }

    char *config_old_str = fs_read_file(CONFIG_FILE, NULL);
    cJSON *config_old_json = cJSON_Parse(config_old_str);
    free(config_old_str);

    if (config_old_json != NULL && cJSON_Compare(config_json, config_old_json, false))
    {
        ESP_LOGW(TAG, "Configuração não foi alterada, não salvar");
        cJSON_Delete(config_json);
        cJSON_Delete(config_old_json);
        return ESP_OK;
    }

    char *config_str = cJSON_Print(config_json);
    if (config_str == NULL)
    {
        ESP_LOGE(TAG, "Erro ao imprimir configuração");
        return ESP_FAIL;
    }

    fs_write_file(CONFIG_FILE, config_str);
    free(config_str);

    cJSON_Delete(config_json);
    cJSON_Delete(config_old_json);

    return ESP_OK;
}

esp_err_t config_load_json(cp32eth_data_t *cp32eth, cJSON *config_json)
{

    if (!config_json || !cp32eth)
    {
        return ESP_FAIL;
    }

    char *aux_str;
    esp_err_t err = ESP_OK;

    // Load info data
    if (cp32eth->info == NULL)
        cp32eth->info = malloc(sizeof(info_data_t));

    cJSON *info_json = cJSON_GetObjectItem(config_json, "info");
    if (info_json)
    {
        aux_str = JSON_GET_VALUE(info_json, "description", JSON_TYPE_STRING);
        cp32eth->info->description = calloc(strlen(aux_str) + 1, sizeof(char));
        strcpy(cp32eth->info->description, aux_str);

        aux_str = JSON_GET_VALUE(info_json, "fw_version", JSON_TYPE_STRING);
        cp32eth->info->fw_version = calloc(strlen(aux_str) + 1, sizeof(char));
        strcpy(cp32eth->info->fw_version, aux_str);

        aux_str = JSON_GET_VALUE(info_json, "hw_version", JSON_TYPE_STRING);
        cp32eth->info->hw_version = calloc(strlen(aux_str) + 1, sizeof(char));
        strcpy(cp32eth->info->hw_version, aux_str);

        aux_str = JSON_GET_VALUE(info_json, "mac_addr", JSON_TYPE_STRING);
        cp32eth->info->mac_addr = calloc(strlen(aux_str) + 1, sizeof(char));
        strcpy(cp32eth->info->mac_addr, aux_str);

        cp32eth->info->resets = JSON_GET_VALUE(info_json, "resets", JSON_TYPE_INT);
        cp32eth->info->config_updates = JSON_GET_VALUE(info_json, "config_updates", JSON_TYPE_INT);
        cp32eth->info->if_updates = JSON_GET_VALUE(info_json, "if_updates", JSON_TYPE_INT);
        cp32eth->info->fw_updates = JSON_GET_VALUE(info_json, "fw_updates", JSON_TYPE_INT);

        cp32eth->info->up_time = JSON_GET_VALUE(info_json, "up_time", JSON_TYPE_INT);
        cp32eth->info->init_time = JSON_GET_VALUE(info_json, "init_time", JSON_TYPE_INT);
        cp32eth->info->install_time = JSON_GET_VALUE(info_json, "install_time", JSON_TYPE_INT);

        cp32eth->info->available_mem = JSON_GET_VALUE(info_json, "available_mem", JSON_TYPE_INT);
    }
    else
    {
        // ESP_LOGE(TAG, "Erro ao carregar info");
        err = ESP_FAIL;
    }

    // Load login data
    if (cp32eth->login == NULL)
        cp32eth->login = malloc(sizeof(login_data_t));

    cJSON *login_json = cJSON_GetObjectItem(config_json, "login");
    if (login_json)
    {
        aux_str = JSON_GET_VALUE(login_json, "username", JSON_TYPE_STRING);
        cp32eth->login->username = calloc(strlen(aux_str) + 1, sizeof(char));
        strcpy(cp32eth->login->username, aux_str);

        aux_str = JSON_GET_VALUE(login_json, "password", JSON_TYPE_STRING);
        cp32eth->login->password = calloc(strlen(aux_str) + 1, sizeof(char));
        strcpy(cp32eth->login->password, aux_str);
    }
    else
    {
        // ESP_LOGE(TAG, "Erro ao carregar login");
        err = ESP_FAIL;
    }

    // Load root data
    if (cp32eth->root == NULL)
        cp32eth->root = malloc(sizeof(login_data_t));

    cJSON *root_json = cJSON_GetObjectItem(config_json, "admin");
    if (root_json)
    {
        aux_str = JSON_GET_VALUE(root_json, "username", JSON_TYPE_STRING);
        cp32eth->root->username = calloc(strlen(aux_str) + 1, sizeof(char));
        strcpy(cp32eth->root->username, aux_str);

        aux_str = JSON_GET_VALUE(root_json, "password", JSON_TYPE_STRING);
        cp32eth->root->password = calloc(strlen(aux_str) + 1, sizeof(char));
        strcpy(cp32eth->root->password, aux_str);
    }
    else
    {
        // ESP_LOGE(TAG, "Erro ao carregar root");
        err = ESP_FAIL;
    }

    // Load ip data
    if (!cp32eth->ip)
        cp32eth->ip = malloc(sizeof(ip_data_t));

    cJSON *ip_json = cJSON_GetObjectItem(config_json, "ip");
    if (ip_json)
    {
        ESP_LOGI(TAG, "Found IP");
        cp32eth->ip->ip = JSON_GET_VALUE(ip_json, "ip", JSON_TYPE_INT);
        cp32eth->ip->mask = JSON_GET_VALUE(ip_json, "mask", JSON_TYPE_INT);
        cp32eth->ip->gateway = JSON_GET_VALUE(ip_json, "gw", JSON_TYPE_INT);
    }
    else
    {
        // ESP_LOGE(TAG, "Erro ao carregar ip");
        err = ESP_FAIL;
    }

    // Load wifi data
    if (!cp32eth->wifi)
        cp32eth->wifi = malloc(sizeof(wifi_data_t));

    cJSON *wifi_json = cJSON_GetObjectItem(config_json, "wifi");
    if (wifi_json)
    {
        cp32eth->wifi->enabled = JSON_GET_VALUE(wifi_json, "enabled", JSON_TYPE_INT);

        aux_str = JSON_GET_VALUE(wifi_json, "ssid", JSON_TYPE_STRING);
        cp32eth->wifi->ssid = calloc(strlen(aux_str) + 1, sizeof(char));
        strcpy(cp32eth->wifi->ssid, aux_str);

        aux_str = JSON_GET_VALUE(wifi_json, "password", JSON_TYPE_STRING);
        cp32eth->wifi->password = calloc(strlen(aux_str) + 1, sizeof(char));
        strcpy(cp32eth->wifi->password, aux_str);
    }
    else
    {
        // ESP_LOGE(TAG, "Erro ao carregar wifi");
        err = ESP_FAIL;
    }

    // Load socket data
    if (!cp32eth->socket)
        cp32eth->socket = malloc(sizeof(client_tcp_data_t));

    cJSON *socket_json = cJSON_GetObjectItem(config_json, "socket");
    if (socket_json)
    {
        ESP_LOGI(TAG, "Found socket");
        cp32eth->socket->ip = JSON_GET_VALUE(socket_json, "ip", JSON_TYPE_INT);
        cp32eth->socket->port = JSON_GET_VALUE(socket_json, "port", JSON_TYPE_INT);
    }
    else
    {
        // ESP_LOGE(TAG, "Erro ao carregar socket");
        err = ESP_FAIL;
    }

    return err;
}

cJSON *config_parse(cp32eth_data_t *cp32eth, config_parse_e option)
{
    cJSON *out = cJSON_CreateObject();

    if (option == CONFIG_ALL)
    {
        // Create info object
        cJSON *info = cJSON_CreateObject();
        cJSON_AddStringToObject(info, "description", cp32eth->info->description);
        cJSON_AddStringToObject(info, "fw_version", cp32eth->info->fw_version);
        cJSON_AddStringToObject(info, "hw_version", cp32eth->info->hw_version);
        cJSON_AddStringToObject(info, "mac_addr", cp32eth->info->mac_addr);
        cJSON_AddNumberToObject(info, "resets", cp32eth->info->resets);
        cJSON_AddNumberToObject(info, "config_updates", cp32eth->info->config_updates);
        cJSON_AddNumberToObject(info, "if_updates", cp32eth->info->if_updates);
        cJSON_AddNumberToObject(info, "fw_updates", cp32eth->info->fw_updates);
        cJSON_AddNumberToObject(info, "up_time", cp32eth->info->up_time);
        cJSON_AddNumberToObject(info, "init_time", cp32eth->info->init_time);
        cJSON_AddNumberToObject(info, "install_time", cp32eth->info->install_time);
        cJSON_AddNumberToObject(info, "available_mem", cp32eth->info->available_mem);

        cJSON_AddItemToObject(out, "info", info);

        // Create login object
        cJSON *login = cJSON_CreateObject();
        cJSON_AddStringToObject(login, "username", cp32eth->login->username);
        cJSON_AddStringToObject(login, "password", cp32eth->login->password);

        cJSON_AddItemToObject(out, "login", login);

        // Create root object
        cJSON *root = cJSON_CreateObject();

        cJSON_AddStringToObject(root, "username", cp32eth->root->username);
        cJSON_AddStringToObject(root, "password", cp32eth->root->password);

        cJSON_AddItemToObject(out, "admin", root);

        // Create ip object
        cJSON *ip = cJSON_CreateObject();

        cJSON_AddNumberToObject(ip, "ip", cp32eth->ip->ip);
        cJSON_AddNumberToObject(ip, "mask", cp32eth->ip->mask);
        cJSON_AddNumberToObject(ip, "gw", cp32eth->ip->gateway);

        cJSON_AddItemToObject(out, "ip", ip);

        // Create wifi object
        cJSON *wifi = cJSON_CreateObject();

        cJSON_AddNumberToObject(wifi, "enabled", cp32eth->wifi->enabled);
        cJSON_AddStringToObject(wifi, "ssid", cp32eth->wifi->ssid);
        cJSON_AddStringToObject(wifi, "password", cp32eth->wifi->password);

        cJSON_AddItemToObject(out, "wifi", wifi);

        // Create socket object
        cJSON *socket = cJSON_CreateObject();

        cJSON_AddNumberToObject(socket, "ip", cp32eth->socket->ip);
        cJSON_AddNumberToObject(socket, "port", cp32eth->socket->port);

        cJSON_AddItemToObject(out, "socket", socket);
    }

    return out;
}
