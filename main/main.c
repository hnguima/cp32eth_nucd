#include "main.h"
#include <sys/time.h>
#include "esp_sntp.h"

static const char *TAG = "main";

cp32eth_data_t *cp32eth;

void app_main(void)
{
	esp_err_t err;

	heap_caps_check_integrity_all(1);
	// Inicializa o watchdog para tasks, tempo 60 segundos, com chamada de pânico
	SYSTEM_ERROR_CHECK(esp_task_wdt_init(60, true), err, TAG, "Erro ao inicializar o task watchdog");

	// Cria o loop padrão de eventos que roda em segundo plano
	SYSTEM_ERROR_CHECK(esp_event_loop_create_default(), err, TAG, "Erro ao inicializar o loop de eventos");
	
	SYSTEM_ERROR_CHECK(fs_mount(), err, TAG, "Erro ao inicializar o filesystem");
	// SYSTEM_ERROR_CHECK(fs_check_default(), err, TAG, "Erro ao gerar a configuração default do fylesystem");
	// SYSTEM_ERROR_CHECK(fs_load(&urt32_data), err, TAG, "Erro ao carregar o filesystem");

	// log_init();
	ESP_ERROR_CHECK(nvs_flash_init());
	// Inicializa gpio
	gpio_config_t io_conf = {
		.pin_bit_mask = GPIO_OUTPUT_PIN_SEL,
		.mode = GPIO_MODE_OUTPUT,
		.intr_type = GPIO_INTR_DISABLE,
	};
	gpio_config(&io_conf);

	gpio_set_level(PHY_ENABLE, 1);

	// Inicializa armazenamento não-volatil
	SYSTEM_ERROR_CHECK(nvs_flash_init(), err, TAG, "Erro ao inicializar o armazenamento nao volatil");
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		// NVS partition was truncated and needs to be erased
		SYSTEM_ERROR_CHECK(nvs_flash_erase(), err, TAG, "Erro ao apagar o armazenamento nao volatil");
		SYSTEM_ERROR_CHECK(nvs_flash_init(), err, TAG, "Erro ao inicializar o armazenamento nao volatil");
	}

	// Carrega as configurações da flash
	cp32eth = calloc(1, sizeof(cp32eth_data_t));
	SYSTEM_ERROR_CHECK(config_load(cp32eth), err, TAG, "Erro ao carregar o filesystem");
	
	// Atualiza o numero de resets
	cp32eth->info->resets++;

	// Inicializa tcp e uart queues
	cp32eth->socket->queue = xQueueCreate(MAX_QUEUE_LENGTH, sizeof(queue_data_t *));
	if (cp32eth->socket->queue == 0)
	{
		ESP_LOGE(TAG, "Erro ao criar a fila de tcp");
	}
	cp32eth->uart_queue = xQueueCreate(MAX_QUEUE_LENGTH, sizeof(queue_data_t *));
	if (cp32eth->uart_queue == 0)
	{
		ESP_LOGE(TAG, "Erro ao criar a fila de uart");
	}

	// Checa se a configuração é de fabrica
	// todo acionar o led de stt caso a config seja de fabrica

	// Inicializa a porta uart
	SYSTEM_ERROR_CHECK(uart_init(cp32eth), err, TAG, "Erro ao inicializar a porta uart");

	// Inicializa ethernet
	SYSTEM_RETRY(ethernet_init(cp32eth->ip), err, TAG, 500, 10);
	
	// Inicializa server http
	httpd_handle_t server = http_server_init(cp32eth);

	ws_log_init(server);

	// Inicializa o cliente tcp
	SYSTEM_ERROR_CHECK(client_tcp_init(cp32eth->socket), err, TAG, "Erro ao inicializar o cliente");

	// Adiciona a task atual ao watchdog
	SYSTEM_ERROR_CHECK(esp_task_wdt_add(NULL), err, TAG, "Erro ao adicionar a task atual ao watchdog");

	uint8_t i = 0;

	while (1)
	{

#ifdef DEBUG_MEMORY
		ESP_LOGI(TAG, "mem : %d", heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
#endif

		// printf("esp timer: %lld\n", esp_timer_get_time() / 1000);

		// Alimenta os watchdogs
		SYSTEM_ERROR_CHECK(esp_task_wdt_reset(), err, TAG, "Erro ao alimentar o watchdog da task");
		rtc_wdt_feed();

		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}