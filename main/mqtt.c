// Archivos necesarios---------
#include <string.h>
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "driver/gpio.h"
/*

La lista de librer�as a utilizar para este archivo fuente es,

string.h � para aprovechar funciones para el manejo de cadenas de caracteres.

esp_event.h � se utilizar� la caracter�stica de eventos para tomar acciones tras la ejecuci�n del protocolo MQTT,
especialmente cuando se logra la conexi�n, publicaci�n y suscripci�n.
 freeRTOS.h y task.h � se utilizan estas librer�as para cargar todas las funciones y definiciones necesarias para
 el sistema operativo freeRTOS.
queue.h � para utilizar la funcionalidad de cola de mensajes es necesario agregar esta librer�a.
 esp_log.h � para los mensajes de depuraci�n a trav�s de UART0 por defecto.
 mqtt_client.h � se carga esta librer�a para poder utilizar las API correspondientes al protocolo WiFi.
 Tras cargar las librer�as necesarias para este c�digo fuente, se realizan las definiciones necesarias y declaraciones
 de variables globales.

*/
// Definiciones ---------------------------------------
#define direccionMQTT  "mqtt://bunkerlab.io"
#define puertoMQTT  1883
#define usuarioMQTT "usuario_broker"
#define claveMQTT  "clave_broker"
#define LED GPIO_NUM_26
#define RELE2 GPIO_NUM_2
// Variables globales----------------------------------
static const char *TAG = "MQTT";
int Activacion=0;
extern int Emergencia;

/*

Se realizan cuatro (4) definiciones que se utilizan en el orden respectivo para la DNS del br�ker en la nube, el
n�mero de puerto, el nombre de usuario para el br�ker y la contrase�a.

 Para las variables globales solamente se definen 2, un puntero que se utilizar� en las funciones de depuraci�n
 ESP_LOGI, ESP_LOGW y ESP_LOGE. La segunda variable global se utiliza con la palabra reservada �extern� pues esta
 variable fue declarada en el archivo principal.c. luego de estas l�neas de c�digo se escriben las funciones que
 se encargar�n de ejecutar el protocolo.

 */

/*

Manejador de eventos

La primera funci�n a codificar en este archivo es la que aprovecha �switch/ case� para ejecutar los eventos
relacionados con el protocolo MQTT, los cuales se enumeran en una estructura dentro de la librer�a �mqtt_client.h�
de la siguiente forma,

MQTT_EVENT_ERROR: de existir errores de ejecuci�n en el protocolo se asigna el valor cero con este macro.
MQTT_EVENT_CONNECTED: cuando se establece con �xito la conexi�n con el br�ker MQTT se utiliza este valor para la variable event_id.
MQTT_EVENT_DISCONNECTED: event_id toma el valor 2 con este macro cuando se desconecta el cliente del br�ker.
MQTT_EVENT_SUBSCRIBED: al realizar una solicitud de suscripci�n y ser aceptado se activa este evento.
MQTT_EVENT_UNSUBSCRIBED: al realizar una solicitud de anulaci�n de suscripci�n y ejecutarse correctamente, se activa el evento n�mero 4.
MQTT_EVENT_PUBLISHED: tras publicar correctamente en el br�ker se activa el event_id  n�mero 5.
MQTT_EVENT_DATA: el evento enumerado con el 6 corresponde a la recepci�n de informaci�n en un t�pico al cual el dispositivo se encuentra suscrito.

 */

static esp_err_t eventosMQTT(esp_mqtt_event_handle_t event){
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "comando", 0);
            ESP_LOGI(TAG, "Suscripcion exitosa, id=%d", msg_id);
            msg_id = esp_mqtt_client_subscribe(client, "status", 0);
            ESP_LOGI(TAG, "Suscripcion exitosa, id=%d", msg_id);
            msg_id = esp_mqtt_client_subscribe(client, "notificacion", 0);
            ESP_LOGI(TAG, "Suscripcion exitosa, id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT desconectado");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "Suscripcion de topico, id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "Suscripcion terminada, id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "Publicacion realizada, id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGW(TAG, "Dato recibido");
            char aux[20] = {0};                              //ARRAY AUXILIAR PARA RECIBIR DATOS
            strncpy(aux,event->topic,event->topic_len);      //COPIAR EL NOMBRE DEL TOPICO EN LA VARIABLE AUX
            if(strcmp(aux,"comando")==0)                     //VERIFICO SI EL TOPICO QUE PRODUJO EL EVENTO ES EL TOPICO comando
				{
            	     bzero(aux,20);                          //SI EL TOPICO ES comando ENTRA AQUI Y LIMPIO EL ARRAY
            	     strncpy(aux,event->data,event->data_len);

            	     gpio_set_direction(LED,GPIO_MODE_OUTPUT);//COPIO EL MENSAJE EN LA VARIABLE AUX

            	     if(strcmp(aux,"on")==0){                  //VERIFICO SI EL MENSAJE ES on
            	    	 gpio_set_level(LED,1);                 //SI ES on ENCIENDO EL LED DEL GPIO 2
            	     	 Activacion=1;
            	     }
            	     else if(strcmp(aux,"off")==0){            //VERIFICO SI EL MENSAJE ES off
            	    	 gpio_set_level(LED,0);
            	    	 Activacion=0;
            	     }
               		else if(strcmp(aux,"OK")==0){            //VERIFICO SI EL MENSAJE ES Ok es decir q todo esta bien
            		    gpio_set_level(RELE2,0);
            		    Emergencia=0;
               		}
				}
            else if(strcmp(aux,"status")==0)                //SI EL TOPICO NO ES comando VERIFICO SI ES status
				{
            		printf("recibido topico status\n\r");
				}
            else if(strcmp(aux,"notificacion")==0)          //SI EL TOPICO NO ES status VERIFICO SI ES notificacion
				{
            		printf("recibido topico notificacion\n\r");
				}
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGE(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

/*

En este caso de ejemplificaci�n, en el evento de conexi�n al br�ker se toman las acciones de suscripci�n a los
t�picos de �humedad� y �temperatura� de modo que el microcontrolador podr� visualizar los mensajes que publica
en el br�ker y reportarlos a trav�s del terminal serial. En el resto de los eventos solamente se escriben
mensajes que son mostrados en la consola a trav�s del UART0 indicando la ejecuci�n actual.

Funcion manejador MQTT

Para utilizar la caracter�stica de eventos es necesario registrar la funci�n que contiene su configuraci�n,
para ello se utiliza este �manejadorMQTT�.

 */

static void ManejadorMQTT(void *handler_args,
      esp_event_base_t base,
      int32_t event_id,
      void *event_data) {
    ESP_LOGD(TAG, "Evento activo de tipo: %s, id = %d", base, event_id);
    eventosMQTT(event_data);
}

/*

Esta funci�n se declara como est�tica, sin valores de retorno y cuatro (4) par�metros de entrada, tras el
registro de esta funci�n para el manejo del MQTT una tarea del sistema operativo que se ejecuta en segundo plano
con estas API se encarga de utilizarla y as� poder especificar el valor de la variable event_id a medida que ocurren los eventos.

 */

/*

Tarea MQTT

La �ltima funci�n a codificar en este archivo fuente es la tarea freeRTOS que se encarga de utilizar el protocolo de comunicaci�n
para publicar los datos tomados de la humedad y la temperatura desde la tarea del sensor DHT11.
 */

void TareaMQTT(void* P){
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = direccionMQTT,
   .port = puertoMQTT,
   //.username = usuarioMQTT,
   //.password = claveMQTT,
    };
    esp_mqtt_client_handle_t cliente = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(cliente,
          ESP_EVENT_ANY_ID,
      ManejadorMQTT,
      cliente);
    esp_mqtt_client_start(cliente);
    char datosDHT[6];
    char E1[]={"Alerta, intruso"};
    char E2[]={"EMERGENCIA"};
    for(;;){
     if(Emergencia==1){
      esp_mqtt_client_publish(cliente,"Alarma",E1,strlen(E1),0,0);
      Emergencia=0;}
     if(Emergencia==2)
     esp_mqtt_client_publish(cliente,"Alarma",E2,strlen(E2),0,0);
     vTaskDelay(100/portTICK_PERIOD_MS);
    }
}
/*

En esta tarea freeRTOS, antes de entrar en el bucle repetitivo infinito, es necesario registrar las funciones y estructuras para el
 funcionamiento de los eventos para el protocolo, esto se inicia codificando la estructura que contiene los datos requeridos por el
 br�ker para aceptar la conexi�n, estos datos fueron declarados en las definiciones al inicio del c�digo

Especificando la direcci�n del br�ker, el n�mero de puerto, el usuario y la clave. Esta estructura contiene varias variables
adicionales que si no son expresamente modificadas, mantienen valores por defecto, como por ejemplo la identidad del dispositivo
(.client_id) entre otros.


Luego de especificar las caracter�sticas requeridas para el protocolo, se debe registrar la estructura e iniciar el cliente MQTT,
para ello se utilizan las tres funciones subsecuentes.

Luego de inicializar las API para el protocolo MQTT, el dispositivo utilizar� la conexi�n WiFi para establecer comunicaci�n con
el br�ker especificado con las caracter�sticas de conexi�n que fueron registradas

Solo resta la declaraci�n de tres variables locales que ser�n utilizadas para recibir los datos del sensor DHT11 y construir el
mensaje a transmitir en dos t�picos, uno para la humedad y el otro para la temperatura
Culminado este punto, se ejecuta el ciclo repetitivo en cuyo funcionamiento, la tarea se bloquea mientras espera la lectura del
DHT11 en la otra tarea, al recibir contenido en la cola de mensajes se procesa la informaci�n para publicarla mediante MQTT.

La funci�n �xQueueReceive� bloquea la tarea mientras espera por contenido en la cola de mensajes llamada �Cola_PIR�, al recibir los datos escritos en la cola desde �tareaDHT�, estos son copiados en la variable �datosDHT�.

Cuando se reciben los bytes con los c�digos ASCII que representan los valores de humedad y temperatura, se alteran las variables que se declararon como locales �HR_dht� para la humedad relativa y �T_dht� para la temperatura. De modo que el mensaje (payload) para el t�pico llamado �Humedad� termina siendo el contenido de la variable �HR_dht� que es una cadena de caracteres con el mensaje �Humedad R.: XX%� donde los valores que corresponden a las posiciones XX son los dos primeros bytes que se reciben en la cola de mensajes (datosDHT[0] y datosDHT[1]).

De forma similar ocurre con el contenido a publicar en el t�pico �Temperatura� donde el contenido del mensaje en la variable �T_dht� es �Temperatura: XX.XC�, donde los valores en las posiciones XX.X se copian desde el valor recibido en los tres �ltimos bytes de la cola de mensajes (datosDHT[3], datosDHT[4] y datosDHT[5]).

Despu�s de modificar el contenido de las cadenas de caracteres �HR_dht� y �T_dht� se utilizan dos funciones para publicar en sus t�picos respectivos los datos de la medici�n de humedad y temperatura.

Esta funci�n tiene seis (6) par�metros de entrada,

El cliente MQTT.
La cadena de caracteres con el nombre del t�pico.
El contenido a publicar, dentro de una variable como cadena de caracteres.
El tama�o en bytes del contenido a publicar.
La prioridad de la publicaci�n qos, cuyas opciones de m�nimo a m�ximo es 0, 1 � 2.
Especificar si se desea que el br�ker retenga la publicaci�n de modo que si no existen dispositivos suscritos al momento de la publicaci�n, el primer dispositivo que se suscriba podr� leer el �ltimo mensaje publicado, si no se desea retener el mensaje se asigna el valor �0� mientras que si se desea retener el mensaje se asigna el valor �1�.
 */
