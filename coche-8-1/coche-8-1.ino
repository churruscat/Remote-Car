#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <ESP8266WiFiAP.h>
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
char ssid1[]     = "RCND.PANTALAN 5";
char password1[] = "estribor";
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
"<button name='orden' id='btn-2' type='submit' value='6'>Mas</button><br><br><br><br><br>"
"</form></center>";
//y la parte de datos

const char *datosVelocidad="<center><table>"
 "<tr><td><strong>Velocidad</strong></td><td>";
const char *datosGiro="</td></tr><tr><td><strong>giro</strong></td><td>";
const char *datosGiroscopio="</td></tr><tr><td><strong>giroscopio</strong></td><td>";
const char *datosCierre="</td></tr><center></table>";
const char *datosCierre1="</td></tr>";
const char *datosCierre2="<center></table>";
char pagina[1600];  //falta contar cuantos caracteres son en total

int potencia=500;
WiFiClient cliente;
MPU6050 mpu6050;
#define pasoPotencia  100  // incrementos y decremento de potencia
#define potenciaMax  1023
int casiPotenciaMax=potenciaMax-pasoPotencia;
#define potenciaMin  350
int casiPotenciaMin=potenciaMin+pasoPotencia;
#define pasoGiro      100  // incrementos y decrementos de giro
#define giroMax      1000
int casiGiroMax=giroMax-pasoGiro;
#define kGiro         60  // constante para suavizar el giro
short int gx, gy, gz; 
float  giroconv=  (250.0/32768.0);  // para 250grad/seg de sensibilidad
int n,
    cambioVelocidad;
    
velocidad vMandoAntes={0,0,0,1},
          vMandoAhora={0,0,0,1},
          vMandoCorregido={0,0,0,0},
          vDeseada ={0,0,0,0},
          vActual ={0,0,0,0} ;

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
 } /*********** fin de SETUP ******************/
/******************************************************************
 *                         LOOP
 *                         LOOP
 *                         LOOP 
 ******************************************************************/ 
void loop() {
  delay(200);
  server.handleClient();  //obtiene los datos de velocidad y giro del mando
 // Serial.println("handleclient"); 
 // ajustaVelocidad();    // ajusta las velocidades para que giroActual sea giroDeseado
}
/******************************************************************
 *                         Funcion
 *                         Funcion                         
 *                         Funcion
*****************************************************************/ 
 
void una_orden() { 
  // el argumento debe ser "orden", en funcion de lo que llegue, asigno los valores de vMandoAhora
  if (server.arg("orden")) {
    int laOrden = server.arg("orden").toInt(); 
    switch (laOrden) {
        case 1:   // gira mas a la izquierda
          if (vMandoAntes.giro > -casiGiroMax)
             vMandoAhora.giro-=pasoGiro;             
          else vMandoAhora.giro=-giroMax;
          break;
        case 2:   // gira mas a la derecha
          if (vMandoAntes.giro < casiGiroMax)
             vMandoAhora.giro+=pasoGiro;
          else vMandoAhora.giro=giroMax;
          break;
        case 3:   // hacia atras recto. **Comentado Si ibas hacia delante, stop  
          vMandoAhora.giro=0;               
//          if (vMandoAntes.sentido >0) vMandoAhora.derecha=0 ; 
          vMandoAhora.sentido=-1;
          break;
        case 4:   // hacia delante recto. **Comentado  Si ibas hacia atras, stop
          vMandoAhora.giro=0; 
//          if (vMandoAntes.sentido <0) vMandoAhora.derecha=0; 
          vMandoAhora.sentido=1;
          break;
        case 5:    // para
          vMandoAhora.derecha=0;
          vMandoAhora.giro=0;
          break;
        case 6:   // aumentar la velocidad
          if (vMandoAntes.derecha>casiPotenciaMax)
             { cambioVelocidad=casiPotenciaMax-vMandoAntes.derecha ; }
          else if (vMandoAntes.derecha<potenciaMin)
                { cambioVelocidad=potenciaMin ;}
          else { cambioVelocidad=pasoPotencia;}
          vMandoAhora.derecha+=cambioVelocidad;
          break;
        case 7:    // bajar la velocidad
          if (vMandoAntes.derecha<casiPotenciaMin) 
             cambioVelocidad=vMandoAntes.derecha-potenciaMin ;
          else cambioVelocidad=pasoPotencia;
          vMandoAhora.derecha-=cambioVelocidad;
          break;
    }
    // corrijo los valores de giro en función de la velocidad
   if (vMandoAhora.derecha!=0) vMandoCorregido.giro=vMandoAhora.giro*kGiro/vMandoAhora.derecha;
    else vMandoCorregido.giro=vMandoAhora.giro;
    vMandoCorregido.derecha=vActual.derecha+vMandoCorregido.giro;
    if (vMandoCorregido.derecha>potenciaMax)
       vMandoCorregido.derecha=potenciaMax;       
    if (vMandoCorregido.derecha < potenciaMin)
        vMandoCorregido.derecha=0;    
        
    vMandoCorregido.izquierda=vActual.derecha-vMandoCorregido.giro;
    if (vMandoCorregido.izquierda < potenciaMin)
        vMandoCorregido.izquierda=0;    
    // descanso y paso control a otros
  }
  // y envia respuesta al navegador
  vMandoAntes=vMandoAhora;  // ??? aqui o en setVelocidad
//  ajustaVelocidad();
  yield();
  sprintf(pagina,"%s %s %d %s %d %s %d %s",
  form,datosVelocidad,vMandoAhora.derecha,
  datosGiro,vMandoAhora.giro,
  datosGiroscopio,0,
  datosCierre);
  
  server.send(200, "text/html", pagina);
}
 
/****************************************************************** 
 *  ??????????????????????????????????????????????????????'
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

void ajustaVelocidad() {
/***** corregir con las variable vMando **********/
 // mpu6050.getRotation(&gx, &gy, &gz); // obten las velocidades angulares actuales
//  vActual.giro = int(gz * giroconv);    //  y las paso a entero en grados por segundo
//  Serial.println(vActual.giro);
  if (vActual.derecha!=0) vDeseada.derecha = vActual.derecha+vDeseada.giro*kGiro/vActual.derecha-vActual.giro;
     else  vDeseada.derecha = vActual.derecha+vDeseada.giro-vActual.giro;
  if (vActual.izquierda!=0) vDeseada.izquierda = vActual.izquierda-vDeseada.giro*kGiro/vActual.izquierda+vActual.giro;
     else vDeseada.izquierda = vActual.izquierda-vDeseada.giro+vActual.giro;
  analogWrite(POWER_A , vDeseada.derecha);
  analogWrite(POWER_B , vDeseada.derecha);
  if (vDeseada.derecha>0) digitalWrite(DIRECCION_A , HIGH);
  else digitalWrite(DIRECCION_A , LOW);
  if (vDeseada.derecha>0) digitalWrite(DIRECCION_B , HIGH);
  else digitalWrite(DIRECCION_B , LOW);
  Serial.print("velocidad Iz \t");Serial.print(vDeseada.izquierda);
  Serial.print("\t Dcha \t");Serial.print(vDeseada.derecha);
  Serial.print("\t Giro \t");Serial.print(vDeseada.giro);
  Serial.print("\t Giro real \t");Serial.println(vMandoCorregido.giro);   
}  

