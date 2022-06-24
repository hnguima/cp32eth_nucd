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
 *  Tempo minimo 2 segundos.
 */
void system_reboot(uint8_t time)
{
	reboot_time = time * 1000;

	if (reboot_time < 2000)
	{
		reboot_time = 2000;
	}

	xTaskCreate(system_reboot_task, "system_reboot_task", 2048, NULL, 2, NULL);
}