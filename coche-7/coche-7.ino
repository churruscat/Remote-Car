#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
//#include <ESP8266WiFiAP.h>
//#include <Wire.h>
#include  "Pin_NodeMCU.h"
#include "estructuras.h"

//#include <helper_3dmath.h>
#include <MPU6050.h>
//#include <MPU6050_6Axis_MotionApps20.h>
//#include <MPU6050_9Axis_MotionApps41.h>

/******************************************************************
 
 * PUERTO SERIE a 115200
 * y también contesta a
 * http:(IP_Addr):80
 * pendiente de hacer la parte donde se reciben los datos por HTTP
 ******************************************************************/ 

char* ssid;
char* password;
char ssid1[]     = "WLANCasa";
char password1[] = “contrasenya”;
short int i=0;

#define LISTEN_PORT           80
ESP8266WebServer server(LISTEN_PORT);
WiFiClient cliente;
MPU6050 mpu6050;

short int gx, gy, gz;
float  giroconv=  (250.0/32768.0);  // para 250grad/seg de sensibilidad
int k=10,                           // constante para suavizar el giro 
    n,
    cambioVelocidad;
String volanteYvel;
char  cadenaDatos[8];
 
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

  for (int i=0;i<5;i++) {
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
  WiFi.disconnect(); // Desconecto para impiar la configuracion y que conecte mas rapido
  WiFi.softAPdisconnect(); 
  ssid=ssid1;
  password=password1;
  Serial.print("Me voy a conectar a");Serial.println(ssid);
  WiFi.begin(ssid,password);
  // WiFi.mode(WIFI_AP_STA);
  //WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(i++);
    Serial.print(".");
      Serial.println();
      Serial.print("Conectado a "); Serial.println(ssid);
      WiFi.begin(ssid,password);      
    }
  Serial.println(ssid);  Serial.print("*******Conectado; ADDR= ");
  Serial.println(WiFi.localIP());

 if(WiFi.waitForConnectResult() == WL_CONNECTED){
    server.on("/", HTTP_POST, [](){
      if (server.hasArg("plain")) {
        volanteYvel=server.arg("plain");
        for ( i = 0; i < volanteYvel.length(); i++) {
          if (volanteYvel.substring(i, i+1) == ":") {
            vMandoAhora.giro = volanteYvel.substring(0, i).toInt();
            vMandoAhora.derecha = volanteYvel.substring(i+1).toInt();
            break;
          }
        }
        Serial.print("Volante= "); Serial.print(vMandoAhora.giro); Serial.print("\t"); 
        Serial.print("Velocidad= "); Serial.println(vMandoAhora.derecha);
        cambioVelocidad=vMandoAhora.derecha-vMandoAntes.derecha;  
        vDeseada.derecha+=cambioVelocidad;
        vDeseada.izquierda+=cambioVelocidad;
        vDeseada.giro=vMandoAhora.giro;
        vMandoAntes=vMandoAhora;
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", "OK");
      };
    });
    yield();
 };
 server.begin();    //arranco el servidor
 } /*********** fin de SETUP ******************/
/******************************************************************
 *                         LOOP
 *                         LOOP
 *                         LOOP 
 ******************************************************************/ 
void loop() {
 
  delay(100);
  server.handleClient();  //obtiene los datos de velocidad y giro del mando
 // procesa(&cliente);    // y proceso el mensaje
 // setVelocidad();       //ajusta los valores al motor
}
/******************************************************************
 *                         Funcion
 *                         Funcion                         
 *                         Funcion
*****************************************************************/ 

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
 // mpu6050.getRotation(&gx, &gy, &gz); // obten las velocidades angulares actuales
//  vActual.giro = int(gz * giroconv);    //  y las paso a entero en grados por segundo
//  Serial.println(vActual.giro);
  vDeseada.derecha= int(vActual.derecha+vDeseada.giro*k/vActual.derecha-vActual.giro);
  vDeseada.izquierda= int(vActual.izquierda-vDeseada.giro*k/vActual.izquierda+vActual.giro);
  Serial.print("velocidad Iz \t");Serial.print(vDeseada.izquierda);
  Serial.print("\t Dcha \t");Serial.print(vDeseada.derecha);
  Serial.print("\t Giro \t");Serial.print(vDeseada.giro);
  Serial.print("\t Giro real \t");Serial.println(vActual.giro); 
  /* y aqui viene darle a setmotor */
  
}  

