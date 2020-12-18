//archivos Espressif API
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Archivos propios
#include "configurar_i2c.h"// configuracion del i2c
#include "pir.h"//archivo de cabecera para el PIR
#include "mqtt.h"
/*dado que en principal.c no se llama a ninguna funcion de ds3231.c
 *  no es necesario agregar el archivo de cabecera aqui
 */
//Macros y definiciones

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "WiFiconfig.h"
#include "mqtt.h"
#define Pila 1024 // reservar memoria de pila de un k byte,
//referencia de asignacion de memoria de pila para las tareas de sistema operativo
#define Pila 1024
#define punteros 1
#define bytes 6//variables globales
//******************
//seccion principal
xQueueHandle Cola_PIR;
void app_main(void){
    ESP_LOGW("Inicio", "Iniciando Aplicacion...");
    ESP_LOGW("Inicio", "Memoria disponible: %d bytes", esp_get_free_heap_size());
    ESP_LOGW("Inicio", "Version ESP-IDF: %s", esp_get_idf_version());
 Cola_PIR = xQueueCreate(punteros,bytes);
 nvs_flash_init();
 iniciar_wifi();
	iniciar_i2c();//inicializacion/ configuracion del puerto i2c
		xTaskCreatePinnedToCore(TareaPir, "Sensor_PIR", Pila*2, NULL, 2, NULL, 0);
		// tarea en el nucleo cero para funcionamiento del PIR
		//xTaskCreatePinnedToCore(Pulsador, "Pulsador", Pila*2, NULL, 2, NULL, 1);
		// tarea para Pulsador
		 xTaskCreatePinnedToCore(TareaMQTT,"MQTT",Pila*7,NULL,2,NULL,1);
}
