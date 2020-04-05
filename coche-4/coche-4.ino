#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFiAP.h>
#include <Wire.h>
#include  "Pin_NodeMCU.h"
#include "estructuras.h"

#include <helper_3dmath.h>
#include <MPU6050.h>
//#include <MPU6050_6Axis_MotionApps20.h>
//#include <MPU6050_9Axis_MotionApps41.h>

/*
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
*/

/******************************************************************
 
 * PUERTO SERIE a 115200
 * y también contesta a
 * http:(IP_Addr):80
 * pendiente de hacer la parte donde se reciben los datos por HTTP
 ******************************************************************/ 

uint32_t delayMS;

char* ssid;
char* password;
char ssid1[]     = "WLANCasa";
char password1[] = "**********";
char ssid2[] ="MOVISTAR_E1F2";
char password2[] = "*********";
short int i;
//short int velocidad=0, giro=0;

#define LISTEN_PORT           80

WiFiServer server(LISTEN_PORT);
MPU6050 mpu6050;

short int gx, gy, gz;

float  giroconv=  (250.0/32768.0);  // para 250grad/seg de sensibilidad
int k=10;                           // constante para suavizar el giro 

velocidad vMandoAntes={0,0,0},
          vMandoAhora={0,0,0},
          vDeseada ={0,0,0},
          vActual ={0,0,0} ;

/******************************************************************
 *                         SETUP 
 *                         SETUP 
 *                         SETUP 
 ******************************************************************/ 
 
void setup() {
/***********************************
 *** Empiezo por el puerto serie 
 * Espero 5 segundos para dar un poco de tiempo
 ***********************************/
  Serial.begin(115200); 
  delay(10);
  for (int i=0;i<10;i++) {
    delay(500);
    Serial.print("llevo esperando ");
    Serial.print(i*0.5);
    Serial.print( " segundos\n");
  }
  Serial.println();
/******************************************************************
  *************           Inicio el acelerometro  *****************
 ******************************************************************/
/*  ******** Quitar comentario
  Wire.begin(SDA, SCL);           //Inicio I2C (SDA D1=5, SCL D2=4)  
  mpu6050.initialize();    //Iniciando mpu6050
  if (mpu6050.testConnection()) Serial.println("mpu6050 iniciado correctamente");
  else Serial.println("ERROR, ERROR ERROR  Error al iniciar el mpu6050");
*/
 /****************************************************************
  * ahora me conecto a la WiFi, como a veces falla, pruebo con dos
 ******************************************************************/

  ssid=ssid1;
  password=password1;
  Serial.print("Connecting to ");Serial.println(ssid);
  WiFi.begin(ssid,password);
 
  while (WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(i++);
    Serial.print(".");
    if (i>20) { 
      if (ssid==ssid1){
        ssid=ssid2;
        password=password2;
      } else {
        ssid=ssid1;
        password=password1;
      }
      i=0;
      Serial.println();
      Serial.print("Me conecto a "); Serial.println(ssid);
      WiFi.begin(ssid,password);      
    }
  }
  Serial.println(ssid);  Serial.print("*******Conectado; ADDR= ");
  Serial.println(WiFi.localIP());
 

 /****************************************************************
  *Creo una instancia de servidor
 ******************************************************************/ 
  server.begin();    //arranco el servidor
}
/******************************************************************
 *                         LOOP
 *                         LOOP
 *                         LOOP 
 ******************************************************************/ 
void loop() {
 
  delay(10);
  WiFiClient cliente = server.available();
  if (!cliente) {      // Espero a que se conecte un cliente   
    return;
  }
  while(!cliente.available()) {  
    delay(10);
  }
  Serial.println("a procesar el cliente ");
  procesa(&cliente);    // y proceso el mensaje
  setVelocidad();
}
/******************************************************************
 *                         Funcion
 *                         Funcion                         
 *                         Funcion
*****************************************************************/ 
void procesa(WiFiClient *cliente) {
 //cliente.setTimeout(5000);
 String quePido = cliente->readStringUntil('/');

   // del mando llegan dos valores: Giro y velocidad, los dos entre -255 y +255
   cliente->print("quePido1    ");cliente->println(quePido);
//   quePido = cliente.readStringUntil('/'); // esto es para quirame el GET
   quePido = cliente->readString(); // esto es para quirame el GET
   cliente->print("quePido2    ");cliente->println(quePido);
   /*vMandoAhora.derecha= asaber
   vMandoAhora.izquierda=asaber
   vMandoAhora.giro=elgiro
   */
}

/****************************************************************** 
 *  Control de los motores de las ruedas. Admito dos  valores:
 *  /set/motor/(numero) => donde -256 > numero < +256
 *                         mueve los motores hacia delante (+) o ahcia atras (-)
 *                         el numero es el factor de velocidad. si  es cero, para los motores
 *  /set/giro/(numero) =>  donde -256 > numero < +256                      
 *                         gira a la derecha(+) o izquierda(-) 
 ***************************************************************/

/******************************************************************
 *    Calcula las velocidades deseadas de cada rueda en funcion de
 *    la Velocidad deseada de cada rueda, el giro deseado y el giro actual                     
 *                         
*****************************************************************/ 

void setVelocidad() {
/***** corregir con las variable vMando *++++++++++*/
  mpu6050.getRotation(&gx, &gy, &gz); // obten las velocidades angulares actuales
  vActual.giro = int(gz * giroconv);    //  y las paso a entero en grados por segundo
  Serial.println(vActual.giro);
  vDeseada.derecha= int(vActual.derecha+vDeseada.giro*k/vActual.derecha-vActual.giro);
  vDeseada.izquierda= int(vActual.izquierda-vDeseada.giro*k/vActual.izquierda+vActual.giro);
  Serial.print("velocidad Iz \t");Serial.print(vDeseada.izquierda);
  Serial.print("\t Dcha \t");Serial.print(vDeseada.derecha);
  Serial.print("\t Giro \t");Serial.print(vDeseada.giro);
  Serial.print("\t Giro real \t");Serial.println(vActual.giro); 
  /* y aqui viene darle a setmotor */
  
}  

