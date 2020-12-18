#include "freertos/FreeRTOS.h" //necesario sistema operativo
#include "freertos/task.h" //necesario sistema operativo
#include "esp_wifi.h" // libreria de funciones del wifi
#include "esp_event_loop.h" // wifi es manejado a traves de una tarea de sistema operativo a baja prioridad llamada eventos
#include "nvs_flash.h" //libreria de memoria flash (wifi lo utiliza)
//Definiciones de SSID y contrasena
#define nombreWiFi "MINHO"//aqui va el nombre de la red wifi a la que se conectara el dispositivo
#define claveWiFi "wickedisgood95"//aqui va la contrasena de la red wifi a la cual nos conectaremos
//****************************************************
//****************************************************
static esp_err_t ManejadorEventos(void *ctx, system_event_t *evento){// creamos tarea de baja prioridad  de eventos
	//esta tarea solo se ejecuta en este modulo, podemos declararla como statica, no se tiene q declarar en el archivo cabecera
	// entrega codigos de error en caso de ser necesario
	//al tratarse de una tarea de sistema operativo se declara con un puntero a vacio (*ctx)
	switch(evento->event_id){// se ejecutan los fragmentos de codigos q son para casos particulares
	case SYSTEM_EVENT_STA_START:// cada uno de los casos se refiere a un numero, en este caso este seria el numero 2
	//2 corresponde al evento llamado inicio de la instruccion
		esp_wifi_connect();//conectar wifi al punto de acceso
		break;//fin del caso para romper el ciclo del swich
	case SYSTEM_EVENT_STA_DISCONNECTED:// no se logro una coneccion por clave incorrecta o porq el punto de acceso no este disponible
		printf("\n\nConexion perdida con el punto de Acceso\n\n");
		vTaskDelay(1000/portTICK_PERIOD_MS);//retardo de un segundo con bloqueo de esta tarea en el sistema operativo
		printf("\n\nReconectado\n\n");
		esp_wifi_connect();// se ejecuta de nuevo la conexion para reintentar el ingreso
		break;
	default:
		break;
	}
	//una vez realizada la conexion se continua con el codigo q se encuentra en webHTTP.c
	// este seria todo el codigo para el manejador de eventos
	return ESP_OK;//codigo de salida, codigo de error de salida, indica q todo esta bien
}

void iniciar_wifi(void){// funcion q inicializa el wifi
	tcpip_adapter_init();//inicializar adaptador tcpip
		esp_event_loop_init(ManejadorEventos, NULL);//especificamos funcion en la cual registramos el manejador de eventos(el escrito arriba)
		wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();// creamos un objeto para inicializacion por defecto wifi
		esp_wifi_init (&cfg);//
		esp_wifi_set_storage(WIFI_STORAGE_RAM);// espeficicar almacenamiento de la memoria, en este caso se guardara en la memoria ram
		wifi_config_t wifi_config = {//estructura en la cual especificamos q la conexion es como una estacion
				//el dispositivo tiene dos posibildiades, se puede configurar como estacion sta o como un punto de acceso (at)
				.sta ={// la estacion se debe conectar a un punto de acceso, necesita nombre y contrasena
						.ssid = nombreWiFi,
						.password = claveWiFi,
				},// asi se escribe la estructura interna
		};
		esp_wifi_set_mode(WIFI_MODE_STA);//especificar q deseamos utilizar el modo estacion
		esp_wifi_set_config(WIFI_IF_STA, &wifi_config);// creamos funcion en la cual se registra la configuracion q especificamos en l estructura,
		// para q quede almacenado en la memoria flash el nombre y contrasena
		esp_wifi_start();// se inicia el wifi en el micro
		// una vez ejecutada esta funcion se ejecuta el case switch de arriba
}
