#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <FS.h>
//#include <ESP8266WiFiAP.h>
//#include <Wire.h>
#include  "Pin_NodeMCU.h"
#include "estructuras.h"

//#include <helper_3dmath.h>
#include <MPU6050.h>

/******************************************************************
 * PUERTO SERIE a 115200
 * y también contesta a
 * http:(IP_Addr):80
 * pendiente de hacer la parte donde se reciben los datos por HTTP
 ******************************************************************/ 

char* ssid;
char* password;

char ssid1[]     = "Churruscat";
char password1[] = "biba1Sailing";
//char ssid1[] ="Coche-M";
//char password1[] = "amodelanoche";
IPAddress miIP(172,20,10,7); // fijo la direccion del coche y el router
IPAddress miGateway(172,20,10,1);

short int i;
// definimos los PINs del motor
#define POWER_A  D1        // 5
#define POWER_B  D2        // 4 
#define DIRECCION_A   D3   // 0 
#define DIRECCION_B   D4   // 2 
#define SDA D5   // Conexiones del acelerometro MPU6050
#define SCL D6

// Defino  el webserver, que recibe las ordenes desde el mando.
ESP8266WebServer servidor(80);   

/****************************************************
 * g_mandos envia por POST el valor
 *  http://172.20.10.7/orden/valor:v,p
 *  donde v=posicion del volante
 *        p=posicion del acelerador
 * 
 */
// Formulario a enviar al navegador Web
char mandoshtml[]="/mandos.html";
File  fMandos;

//y la parte de datos

int potencia=0;

WiFiClient cliente;
MPU6050 mpu6050;
#define potenciaMax  1023
#define potenciaMin  200
#define kGiro        200  // constante para suavizar el giro. Cuanto mas pequenya, mas suaviza
short int gx, gy, gz,
           ax, ay, az;
 /* para 250grad/seg de sensibilidad,            
  *  luego divido por  para suavizar 
  */
float  giroconv=  (250.0/32768.0)/30; 
                    
int n,
    cambioVelocidad,
    giroReal;
    
velocidad vMandoAntes={0,0,0,1},
          vMandoAhora={0,0,0,1},
          vMandoCorregido={0,0,0,1},
          vDeseada ={0,0,0,0} ;

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
  *************           Inicio el giroscopio  *****************
 ******************************************************************/

  Wire.begin(SDA, SCL);           //Inicio I2C (SDA D5, SCL D6)  
  mpu6050.initialize();    //Iniciando mpu6050
  if (mpu6050.testConnection()) Serial.println("mpu6050 iniciado correctamente");
  else Serial.println("ERROR, ERROR ERROR  Error al iniciar el mpu6050");
               
 /****************************************************************
  * ahora me conecto a la WiFi, Esta el codigo con Acces point (comentado)
  * y con el ESP8266 haciendo de access point, 
 ******************************************************************/
  WiFi.disconnect(); // Desconecto para impiar la configuracion y que conecte mas rapido
  ssid=ssid1;
  password=password1;
  Serial.print("Connecting to ");Serial.println(ssid);
  WiFi.config(miIP, miGateway, IPAddress(255,255,255,0));
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(i++);
    Serial.print(".");
  }
  Serial.println(ssid);  Serial.print("*******Conectado; ADDR= ");
  Serial.println(WiFi.localIP());
  /* Pongo que cuando me lleguen mensajes a mi direccion
     directamente, ejecute la funcion "una_orden"*/
    servidor.on("/orden", una_orden);
    servidor.on("/", cargaPag); 
    SPIFFS.begin();       // monto el sistema de ficheros, donde esta la pag
    
    servidor.begin();  // Arranco el webserver
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
  delay(100);
  servidor.handleClient();  //obtiene los datos de velocidad y giro del mando
  ajustaVelocidad();    // ajusta las velocidades para que giroActual sea giroDeseado
}
/******************************************************************
 *                         Funcion
 *                         Funcion                         
 *                         Funcion
*****************************************************************/ 
void cargaPag () {
 
  if (SPIFFS.exists(mandoshtml)) {  
    File fMandos = SPIFFS.open(mandoshtml, "r");                 
    size_t total = servidor.streamFile(fMandos, "text/html"); 
    fMandos.close(); 
  } else { 
    Serial.print("No encuentro el fichero   ");Serial.println(mandoshtml);                                    
  }
}
 
void una_orden() { 

  // el argumento debe ser "orden", en funcion de lo que llegue, asigno los valores de vMandoAhora
  if (servidor.arg("orden")) {
    /*obtengo los valores de potencia y giro del mando. Para corregir la potencia del motor (Pr)
      que va de 200 a 1000 (o de -200 a -1000), de la del mando (Pm) que va de -50 a 50
      la convierto con la funcion map primero lo hago en valor asoluto y si hace falta cambio el signo
     */    
    potencia=servidor.arg("potencia").toInt();
    if (potencia!=0) {
      vMandoAhora.derecha=map(abs(potencia),1,50,200,1000);
    } else vMandoAhora.derecha=0;
    if (potencia<0) vMandoAhora.derecha= -vMandoAhora.derecha;
    vMandoAhora.giro=servidor.arg("volante").toInt();  

    // corrijo los valores de giro en función de la velocidad
   if (vMandoAhora.derecha!=0) 
      vMandoCorregido.giro=vMandoAhora.giro*kGiro/vMandoAhora.derecha;
   else 
      vMandoCorregido.giro=vMandoAhora.giro;
   vMandoCorregido.derecha=vMandoAhora.derecha+vMandoCorregido.giro;
   if (vMandoCorregido.derecha>potenciaMax)
       vMandoCorregido.derecha=potenciaMax;       
   if (vMandoCorregido.derecha < potenciaMin)
        vMandoCorregido.derecha=0;    
        
    vMandoCorregido.izquierda=vMandoCorregido.derecha-vMandoCorregido.giro;
    if (vMandoCorregido.izquierda < potenciaMin)
        vMandoCorregido.izquierda=0;    
    // descanso y paso control a otros
  }

  vMandoAntes=vMandoAhora;  
  // y envia respuesta al navegador
  servidor.send(200, "text/html", "");
  //ajusta el giro con el acelerometro para ir recto y envia orden a los motores
  ajustaVelocidad();
  yield();
}

/******************************************************************
 *    Calcula las velocidades deseadas de cada rueda en funcion de
 *    la Velocidad deseada de cada rueda, el giro deseado y el giro actual                     
 *                         
*****************************************************************/ 

void ajustaVelocidad() {
/***** corregir con las variable vDeseada.giro **********/
  mpu6050.getRotation(&gx, &gy, &gz); // obten las velocidades angulares actuales
  giroReal = int(gz * giroconv);    //  y las paso a entero en grados por segundo
  if (vMandoAhora.derecha!=0) {
    vDeseada.derecha =   vMandoAhora.derecha + vMandoCorregido.giro - giroReal;
    vDeseada.izquierda = vMandoAhora.derecha - vMandoCorregido.giro + giroReal;
    if ((vDeseada.derecha)>0) digitalWrite(DIRECCION_A , HIGH);
    else digitalWrite(DIRECCION_A , LOW);
    if ((vDeseada.izquierda>0)) digitalWrite(DIRECCION_B , HIGH);
    else digitalWrite(DIRECCION_B , LOW);
  } else {
    vDeseada.derecha=0;
    vDeseada.izquierda=0;    
  }
  analogWrite(POWER_A , vDeseada.derecha);
  analogWrite(POWER_B , vDeseada.izquierda);
}  

