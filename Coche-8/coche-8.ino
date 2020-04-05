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
char password1[] = "can(M0rras).c0n5";
char ssid2[] ="Coche-M";
char password2[] = "amodelanoche";
short int i;

#define POWER_A  D1        // 5
#define POWER_B  D2        // 4 
#define DIRECCION_A   D3   // 0 
#define DIRECCION_B   D4   // 2 

// Defino  el webserver
ESP8266WebServer server(80);
// Formulario a enviar al navegador Web
const char *form = "<center><form action='/'>"
"<style> #btn-1 { width: 100px; height: 30px;} </style>"
"<style> #btn-2 { width: 100px; height: 50px;} </style>" 
"<button name='orden' id='btn-2' 'type='submit' value='4'>Adelante</button><br><br><br>"
"<button name='orden' id='btn-1' type='submit' value='1'>Izquierda</button>&nbsp;"
"<button name='orden' id='btn-2' type='submit' value='5'>PARAR</button>"
"<button name='orden' id='btn-1' type='submit' value='2'>Derecha</button><br><br><br>"
"<button name='orden' id='btn-2' type='submit' value='3'>Atras</button><br><br><br><br><br>"
"<button name='orden' id='btn-2' type='submit' value='7'>Menos</button>"
"'<========================>'"
"<button name='orden' id='btn-2' type='submit' value='6'>Mas</button><br>"
"</form></center>";
 
int potencia=500;
WiFiClient cliente;
MPU6050 mpu6050;
#define pasoPotencia  100  // incrementos y decremento de potencia
#define potenciaMax  1023
int casiPotenciaMax=potenciaMax-pasoPotencia;
#define pasoGiro      100  // incrementos y decrementos de giro
#define giroMax      1000
#define kGiro         60  // constante para suavizar el giro
short int gx, gy, gz; 
float  giroconv=  (250.0/32768.0);  // para 250grad/seg de sensibilidad
int n,
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
  * ahora me conecto a la WiFi, Esta el codigo con Acces point (comentado)
  * y con el ESP8266 haciendo de access point, 
 ******************************************************************/
  WiFi.disconnect(); // Desconecto para impiar la configuracion y que conecte mas rapido
  ssid=ssid1;
  password=password1;
  Serial.print("Connecting to ");Serial.println(ssid);
  WiFi.begin(ssid,password);
 
  while (WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(i++);
    Serial.print(".");
  }
 /*****   Arranco el Access Point  ****/     
/*  
  ssid=ssid2;
  password=password2;
  WiFi.softAPdisconnect(); 
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password); 
  // dirección ip, gateway, netmask
  WiFi.config(IPAddress(192,168,1,158), IPAddress(192,168,1,1), IPAddress(255,255,255,0));
 
  // connect
  while (WiFi.status() != WL_CONNECTED)    {
        delay(200);
    }
 */
  /* aqui deberia encender un LED */
  Serial.println(ssid);  Serial.print("*******Conectado; ADDR= ");
  Serial.println(WiFi.localIP());

  /* Pongo que cuando me lleguen mensajes a mi direccion
     directamente, ejecute la funcion "una_orden"*/
    server.on("/", una_orden);
 
    // Arranco el webserver
    server.begin();
    // y defino los PINs
    pinMode(POWER_A , OUTPUT);     
    pinMode(POWER_B , OUTPUT);     
    pinMode(DIRECCION_A , OUTPUT); 
    pinMode(DIRECCION_B , OUTPUT); 

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
 }
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
 // setVelocidad();       //actua sobre los motores en funcion del mando
 // ajustaVelocidad();    // ajusta las velocidades para que giroActual sea giroDeseado
}
/******************************************************************
 *                         Funcion
 *                         Funcion                         
 *                         Funcion
*****************************************************************/ 
void stop(void) {
  analogWrite(POWER_A , 0);
  analogWrite(POWER_B , 0);
}
 
void forward(void) {
  analogWrite(POWER_A , potencia);
  analogWrite(POWER_B , potencia);
  digitalWrite(DIRECCION_A , HIGH);
  digitalWrite(DIRECCION_B , HIGH);
}
 
void backward(void) {
  analogWrite(POWER_A , potencia);
  analogWrite(POWER_B , potencia);
  digitalWrite(DIRECCION_A , LOW);
  digitalWrite(DIRECCION_B , LOW);
}
 
void left(void) {
  analogWrite(POWER_A , potencia);
  analogWrite(POWER_B , potencia);
  digitalWrite(DIRECCION_A , LOW);
  digitalWrite(DIRECCION_B , HIGH);
}
 
void right(void) {
  analogWrite(POWER_A , potencia);
  analogWrite(POWER_B , potencia);
  digitalWrite(DIRECCION_A , HIGH);
  digitalWrite(DIRECCION_B , LOW);
}
 
void una_orden() { 
  // el argumento debe ser "orden", en funcion de lo que llegue, asigno los valores de vMandoAhora
  if (server.arg("orden")) {
    int laOrden = server.arg("orden").toInt(); 
    switch (laOrden) {
        case 1:   // gira mas a la izquierda
          left();
          break;
        case 2:   // gira mas a la derecha
          right();
          break;
        case 3:   // hacia atras recto si ibas hacia delante, stop  
          vDeseada.giro=0;               
          if (vMandoAntes.derecha >0) vMandoAhora.derecha=0 ; 
          break;
        case 4:   // hacia delante recto si ibas hacia atras, stop
          vDeseada.giro=0; 
          if (vMandoAntes.derecha <0) vMandoAhora.derecha=0; 
          break;
        case 5:    // para
          vMandoAhora.derecha=0;
          vMandoAhora.giro=0;
          break;
        case 6:   // aumentar la velocidad
          if (vMandoAntes.derecha>casiPotenciaMax)
             cambioVelocidad=casiPotenciaMax-vMandoAntes.derecha ;
          else cambioVelocidad=pasoPotencia;
          vMandoAhora.derecha+=cambioVelocidad;
          break;
        case 7:    // bajar la velocidad
          if (vMandoAntes.derecha<-casiPotenciaMax) 
             cambioVelocidad=casiPotenciaMax-vMandoAntes.derecha ;
          else cambioVelocidad=pasoPotencia;
          vMandoAhora.derecha-=cambioVelocidad;
          break;
    }
    // descanso y paso control a otros
  }
  // y envia respuesta al navegador
  vMandoAntes=vMandoAhora;
  yield();
  server.send(200, "text/html", form);
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
/***** corregir con las variable vMando **********/
 // mpu6050.getRotation(&gx, &gy, &gz); // obten las velocidades angulares actuales
//  vActual.giro = int(gz * giroconv);    //  y las paso a entero en grados por segundo
//  Serial.println(vActual.giro);
  vDeseada.derecha = vActual.derecha+vDeseada.giro*kGiro/vActual.derecha-vActual.giro;
  vDeseada.izquierda = vActual.izquierda-vDeseada.giro*kGiro/vActual.izquierda+vActual.giro;
  Serial.print("velocidad Iz \t");Serial.print(vDeseada.izquierda);
  Serial.print("\t Dcha \t");Serial.print(vDeseada.derecha);
  Serial.print("\t Giro \t");Serial.print(vDeseada.giro);
  Serial.print("\t Giro real \t");Serial.println(vActual.giro); 
  /* y aqui viene darle a setmotor */
  
}  

