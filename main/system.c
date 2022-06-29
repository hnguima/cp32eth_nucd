#include "system.h"

static const char *TAG = "system_app";

uint32_t reboot_time = 0;

void system_reboot_task(void *param)
{

	ESP_LOGI(TAG, "Reboot system in %d ms...", reboot_time);

	vTaskDelay(reboot_time / portTICK_PERIOD_MS);

	ESP_LOGI(TAG, "Rebooting...");
	esp_restart();
	// vTaskDelete(NULL);
}

/*
 *	Reinicia a aplicação.
 *  Tempo minimo 500 ms.
 */
void system_reboot(uint16_t time_ms)
{
	reboot_time = time_ms;

	if (reboot_time < 500)
	{
		reboot_time = 500;
	}

	xTaskCreate(system_reboot_task, "system_reboot_task", 2048, NULL, 2, NULL);
}