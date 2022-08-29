#include "socket_tcp.h"

void socket_server_task(void *vParam);
void socket_client_task(void *vParam);

static const char *TAG = "socket_tcp";

socket_ctx_t *sockets[MAX_SOCKETS];
uint8_t open_sockets = 0;

// done testar o funcionamento da socket task create (aparentemente ok)
esp_err_t socket_task_create(socket_ctx_t *new_socket, uint8_t type)
{

	if (!(open_sockets < MAX_SOCKETS))
	{
		ESP_LOGE(TAG, "Numero de servers abertos ja e o maximo permitido: %d", MAX_SOCKETS);
		return ESP_ERR_NOT_SUPPORTED;
	}

	int8_t socket_index = -1;

	for (uint8_t i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i] != NULL)
		{
			if (open_sockets == 0)
			{
				sockets[i] = NULL;
				continue;
			}

			if (new_socket->addr == sockets[i]->addr && new_socket->port == sockets[i]->port)
			{
				ESP_LOGE(TAG, "Já existe um servidor aberto com este IP e porta");
				return ESP_ERR_NOT_SUPPORTED;
			}

			if (socket_index == -1)
			{
				socket_index = i;
			}
		}
	}

	sockets[socket_index] = new_socket;

	char name[16];
	ESP_LOGI(TAG, "Socket aberto %d", open_sockets);

	sockets[socket_index]->mutex = xSemaphoreCreateMutex();

	if (sockets[socket_index]->mutex == NULL)
	{
		ESP_LOGE(TAG, "nao foi possivel criar o mutex");
		return ESP_FAIL;
	}

	sprintf(name, "socket_%2d", socket_index);

	if (type == 0)
	{
		sockets[socket_index]->size_tx = 0;
		sockets[socket_index]->size_rx = 0;
		xTaskCreate(socket_server_task, name, 8192, (void *)sockets[socket_index], 5, NULL);
	}
	else
	{
		xTaskCreate(socket_client_task, name, 8192, (void *)sockets[socket_index], 5, NULL);
	}

	open_sockets++;

	return ESP_OK;
}

esp_err_t socket_task_delete(int socket)
{
	// Falha na abertura do socket Server, ou por falta de memória ou IP errado do servidor
	ESP_LOGI(TAG, "socket_task foi encerrada...");

	esp_err_t err = close(socket);

	// error no fechamento do socket? (-1)
	ESP_LOGI(TAG, "Erro que ocasionou o encerramento: %d", err);

	vTaskDelete(NULL);

	open_sockets--;

	return err;
}

esp_err_t socket_send(socket_ctx_t *socket, uint8_t *buffer, uint16_t size)
{

	esp_err_t err = ESP_OK;

	vTaskDelay(100 / portTICK_PERIOD_MS);

	if (xSemaphoreTake(socket->mutex, (TickType_t)100) == pdTRUE)
	{
		if (socket == NULL)
		{
			err = ESP_ERR_NOT_FOUND;
		}
		else if (socket->size_tx != 0)
		{
			err = ESP_ERR_INVALID_STATE;
		}
		else if (socket->size_tx != 0)
		{
			err = ESP_ERR_INVALID_STATE;
		}
		else if (size > SOCKET_TX_BUF_MAX)
		{
			err = ESP_ERR_INVALID_ARG;
		}
		else
		{
			memcpy(socket->buf_tx, buffer, size);
			socket->size_tx = size;
		}

		if (socket->size_tx > 0)
		{
			err = send(socket->cli_sock, socket->buf_tx, socket->size_tx, 0);
			if (err < 0)
			{
				ESP_LOGE(TAG, "erro no send: errno %d", errno);
				xSemaphoreGive(socket->mutex);
			}
			else
			{
				// ESP_LOG_BUFFER_HEX(TAG, socket->buf_tx, socket->size_tx);
				socket->size_tx = 0;
			}
		}
		else
		{
			// ESP_LOGW(TAG, "Resposta TCP não enviada.");
		}
		xSemaphoreGive(socket->mutex);
	}
	else
	{
		ESP_LOGE(TAG, "mutex socket nao liberado");
	}

	return err;
}

// Task de configuração e validação das conexões com o socket TCP
// fix adaptar essa task para funcionar como a outra
void socket_server_task(void *vParam)
{
	socket_ctx_t *socket_ctx = (socket_ctx_t *)vParam;
	struct sockaddr_in client_addr;
	struct sockaddr_in server_addr;

	// Inicializa o socket TCP
	int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	ESP_LOGI(TAG, "socket server iniciado no endereco %d.%d.%d.%d:%d.", (socket_ctx->addr) & 0xFF, ((socket_ctx->addr) >> 8) & 0xFF,
			 ((socket_ctx->addr) >> 16) & 0xFF, (socket_ctx->addr) >> 24, socket_ctx->port);

	if (listen_sock < 0)
	{
		ESP_LOGE(TAG, "Erro %d ao criar o socket", listen_sock);
		socket_task_delete(listen_sock);
	}

	// Vincula o IP do servidor com a PORTA a ser aberta.
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = socket_ctx->addr;
	server_addr.sin_port = htons(socket_ctx->port);

	// Definir o numero da porta e endereço do socket
	int err = bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

	if (err < 0)
	{
		ESP_LOGE(TAG, "Erro %d na função bind.", err);
		socket_task_delete(listen_sock);
		return;
	}

	// Define numero máximo de clientes simultaneos
	ESP_LOGI(TAG, "socket escutando.");
	err = listen(listen_sock, CLIENT_MAX_NUMBER);

	if (err < 0)
	{
		ESP_LOGE(TAG, "Erro %d na função listen.", err);
		socket_task_delete(listen_sock);
		return;
	}

	socklen_t client_addr_size = sizeof(client_addr);

	while (true)
	{
		ESP_LOGI(TAG, "conexao ethernet estabelecida.");

		// Aceita a conexão e retorna o socket do cliente "clientSock" armazenará o handle da conexão socket do cliente
		int client_socket = accept(listen_sock, (struct sockaddr *)&client_addr, &client_addr_size);

		// error? (-1)
		if (client_socket < 0)
		{
			ESP_LOGE(TAG, "Erro %d na função accept.", client_socket);
			socket_task_delete(listen_sock);
			return;
		}
		else
		{
			ESP_LOGI(TAG, "Socket Handle ID: %d", client_socket);

			//  socket->wd_mdb_counter = 0; //Zera o contador do watchdog da conexão Modbus TCP

			socket_ctx->cli_sock = client_socket;

			uint8_t *data_rx = malloc(1024);
			uint8_t *data_tx = NULL;

			if (data_rx == NULL)
			{
				ESP_LOGE(TAG, "malloc Error");
				close(socket_ctx->cli_sock);
				continue;
			}

			ESP_LOGI(TAG, "socket_recv %d iniciada...", socket_ctx->cli_sock);

			while (true)
			{
				// O codigo fica travado nesse ponto até que cheguem dados.
				ssize_t size_rx = recv(socket_ctx->cli_sock, data_rx, 1024, 0);

				// error? (-1)
				if (size_rx < 0)
				{
					ESP_LOGE(TAG, "Erro %d durante a recepção do pacote.", size_rx);
					break;
				}

				// Cliente encerrou a conexão com o servidor?
				if (size_rx == 0)
				{
					ESP_LOGE(TAG, "O cliente encerrou a conexão inesperadamente.");
					break;
				}

				data_rx[size_rx] = '\0';

				// Existem bytes a serem lidos do buffer TCP da LWIP
				ESP_LOGI(TAG, "\n Socket Handle ID: %d\n Bytes Recebidos(size: %d)\n Conteudo: %s", socket_ctx->cli_sock, size_rx, data_rx);

				data_tx = malloc(sizeof(char) * 30);

				socket_ctx->recv_cb(data_rx, size_rx);

				uint8_t size_tx = 0;
				if (size_tx > 0)
				{
					int err = send(socket_ctx->cli_sock, data_tx, size_tx, 0);
					ESP_LOGI(TAG, "%d bytes enviados via socket.", err);

					//  su2mdb_ctx->wd_mdb_counter = 0; //Zera o contador do watchdog da conexão Modbus TCP
				}
				else
				{
					ESP_LOGE(TAG, "Resposta TCP não enviada.");
				}

				// Bloqueia a task por alguns instantes.
				// Caso seja necessario maior throughput de dados, diminua esse intervalo.
				vTaskDelay(100 / portTICK_PERIOD_MS);
			}

			// Encerra a conexão socket, liberando o objeto em memória;
			close(socket_ctx->cli_sock);

			// Limpa a id do socket armazenada no contexto da aplicação
			socket_ctx->cli_sock = 0;

			// Libera o buffer data utilizado no armazenamento dos bytes recebidos que foi criado de maneira dinâmica;
			free(data_rx);
		}
	}
}

void socket_client_task(void *vParam)
{
	socket_ctx_t *socket_ctx = (socket_ctx_t *)vParam;
	struct sockaddr_in server_addr;

	// Vincula o IP do servidor com a PORTA a ser aberta.
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = socket_ctx->addr;
	server_addr.sin_port = htons(socket_ctx->port);

	while (true)
	{
		// Aguarda a conexão Ethernet do ESP32 com o roteador;

		ESP_LOGI(TAG, "conexao ethernet estabelecida.");

		// Inicializa o socket TCP
		socket_ctx->cli_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (socket_ctx->cli_sock < 0)
		{
			ESP_LOGE(TAG, "Erro %d ao criar o socket", socket_ctx->cli_sock);
			socket_task_delete(socket_ctx->cli_sock);
		}
		ESP_LOGI(TAG, "socket client criado endereco %d.%d.%d.%d:%d.", (socket_ctx->addr) & 0xFF, ((socket_ctx->addr) >> 8) & 0xFF,
				 ((socket_ctx->addr) >> 16) & 0xFF, (socket_ctx->addr) >> 24, socket_ctx->port);

		// Aceita a conexão e retorna o socket do cliente "clientSock" armazenará o handle da conexão socket do cliente
		int err = connect(socket_ctx->cli_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

		// fcntl(socket_ctx->cli_sock, F_SETFL, O_NONBLOCK);

		if (err < 0)
		{
			ESP_LOGE(TAG, "Erro %d na função connect.", err);
			if (socket_ctx->is_persistent || (socket_ctx->retry < socket_ctx->retry_max))
			{
				ESP_LOGW(TAG, "tentando novamente em %dms. retry n: %d, max: %d", socket_ctx->retry_delay, ++socket_ctx->retry, socket_ctx->retry_max);
				vTaskDelay(socket_ctx->retry_delay / portTICK_PERIOD_MS);
				// Encerra a conexão socket, liberando o objeto em memória;
				close(socket_ctx->cli_sock);
				continue;
			}
			else
			{
				socket_task_delete(socket_ctx->cli_sock);
				return;
			}
		}
		else
		{
			ESP_LOGI(TAG, "socket client conectado");
			socket_ctx->on_connect_cb();
			socket_ctx->retry = 0;

			//  socket->wd_mdb_counter = 0; //Zera o contador do watchdog da conexão Modbus TCP

			ESP_LOGI(TAG, "socket_recv %d iniciada...", socket_ctx->cli_sock);

			//Adiciona a task atual a lista de tasks monitoradas por watchdog
			// esp_task_wdt_add(NULL);

			while (true)
			{

				socket_ctx->size_rx = recv(socket_ctx->cli_sock, socket_ctx->buf_rx, SOCKET_RX_BUF_MAX, 0);
				// Error occurred during receiving
				if (socket_ctx->size_rx > 0)
				{
					socket_ctx->buf_rx[socket_ctx->size_rx] = '\0';

					// ESP_LOG_BUFFER_HEXDUMP(TAG, socket_ctx->buf_rx, socket_ctx->size_rx, ESP_LOG_WARN);

					socket_ctx->recv_cb(socket_ctx->buf_rx, socket_ctx->size_rx);
				}
				else if (socket_ctx->size_rx < 0)
				{
					ESP_LOGE(TAG, "erro recv(%d): socket not connected", errno);
					socket_ctx->on_disconnect_cb();

					//Deleta a task atual da lista de tasks monitoradas por watchdog
					// esp_task_wdt_delete(NULL);

					// retry?
					if (socket_ctx->is_persistent || (socket_ctx->retry < socket_ctx->retry_max))
					{
						ESP_LOGW(TAG, "tentando novamente em %dms. retry n: %d, max: %d", socket_ctx->retry_delay, ++socket_ctx->retry, socket_ctx->retry_max);
						vTaskDelay(socket_ctx->retry_delay / portTICK_PERIOD_MS);

						// Encerra a conexão socket, liberando o objeto em memória;
						close(socket_ctx->cli_sock);
						break;
					}
					else
					{
						socket_task_delete(socket_ctx->cli_sock);
						return;
					}
				}

				//Reseta o watchdog, caso não tenha travado
				// esp_task_wdt_reset();
				// Bloqueia a task por alguns instantes.
				// Caso seja necessario maior throughput de dados, diminua esse intervalo.
				vTaskDelay(10 / portTICK_PERIOD_MS);
			}
		}
	}
}