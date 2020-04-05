
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFiAP.h>
#include <Wire.h>

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
//#include  "Pin_NodeMCU.h"
/*
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN             D5        // PIN 5 GPIO 14 Pin which is connected to the DHT sensor.
#define DHTTYPE           DHT11     // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT_Unified dht(DHTPIN, DHTTYPE);
*/
/******************************************************************
 * Este programa lee los daots de un sensor de temperatura, muestra la medida  por el
 * PUERTO SERIE a 115200
 * y también contesta a
 * http:(IP_Addr):80
 * El valor velocidad se varia llamando a /set//velocidad
 * y el valor giro se varia llamando a /set/giro
 * para preguntar por los calores, llamar
 ******************************************************************/ 

uint32_t delayMS;

char* ssid;
char* password;
char ssid1[]     = "WLANCasa";
char password1[] = "can(M0rras).c0n5";
char ssid2[] ="MOVISTAR_E1F2";
char password2[] = "Vivan1los2Morras";
short int i;
short int velocidad=0, giro=0;

#define LISTEN_PORT           80
#define GIRO       0
#define IZQUIERDA  1
#define DERECHA    2


WiFiServer server(LISTEN_PORT);
MPU6050 mpu6050;

// Valores RAW (sin procesar) del acelerometro y giroscopio en los ejes x,y,z
//short int ax, ay, az;
short int gx, gy, gz;
short int vDa, vIa ;
float ax_m_s2,  ay_m_s2,  az_m_s2 ;
float gx_deg_s,  gy_deg_s,  gz_deg_s ;
float  giroconv=  (250.0/32768.0);  // para 250grad/seg de sensibilidad
float  acelconv= (9.81/16384.0);    // para 2g de sensibilidad

int vDeseada[3] ={0,0,0},
     vActual[3] ={0,0,0} ;


/******************************************************************
 *    Calcula las velocidades de cada rueda en funcion de
 *             Velocidad deseada   Velocidad actual                      
 *     Derecha       vDd                 vDa
 *     Izquierda     vId                 vIa
 *     Giro de volante    : gV 
 *     Giro actual (omega): gA
 *                         
*****************************************************************/ 

void setVelocidad(void) {
  int k=10;
  
  vDeseada[DERECHA]= int(vActual[DERECHA]+vDeseada[GIRO]*k/vActual[DERECHA]-vActual[GIRO]);
  vDeseada[IZQUIERDA]= int(vActual[IZQUIERDA]-vDeseada[GIRO]*k/vActual[IZQUIERDA]+vActual[GIRO]);
  Serial.print("velocidad Iz \t");Serial.print(vDeseada[IZQUIERDA]);
  Serial.print("\t Dcha \t");Serial.print(vDeseada[DERECHA]);
  Serial.print("\t Giro \t");Serial.print(vDeseada[GIRO]);
  Serial.print("\t Giro real \t");Serial.println(vActual[GIRO]); 
}  


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

  Wire.begin(SDA, SCL);           //Inicio I2C (SDA D1=5, SCL D2=4)  
  mpu6050.initialize();    //Iniciando mpu6050
  if (mpu6050.testConnection()) Serial.println("mpu6050 iniciado correctamente");
  else Serial.println("ERROR, ERROR ERROR  Error al iniciar el mpu6050");

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
 
   WiFiClient cliente = server.available();
  if (!cliente) {      // Espero a que se conecte un cliente   
    return;
  }
  while(!cliente.available()) {  
    delay(10);
  }
   Serial.println("a procesar el cliente ");
  procesa(cliente);    // y proceso el mensaje
  setVelocidad();
  delay(50);
}
/******************************************************************
 *                         Funcion
 *                         Funcion                         
 *                         Funcion
*****************************************************************/ 
void procesa(WiFiClient cliente) {
 String quePido = cliente.readStringUntil('/');

   // del mando llegan dos valores: Giro y velocidad, los dos entre -255 y +255
   cliente.print("quePido    ");cliente.println(quePido);
//   quePido = cliente.readStringUntil('/'); // esto es para quirame el GET
   quePido = cliente.readString(); // esto es para quirame el GET
   cliente.print("quePido    ");cliente.println(quePido);
   
  // En funcion de que sea "set" o "query" llamo a quien corresponda
 
}

/****************************************************************** 
 *  Admito dos  valores:
 *  /set/motor/(numero) => donde -256 > numero < +256
 *                         mueve los motores hacia delante (+) o ahcia atraa (-)
 *                         el numero es el factor de velocidad. si  es cero, para los motores
 *  /set/giro/(numero) =>  donde -256 > numero < +256                      
 *                         gira a la derecha(+) o izquierda(-) 
******************************************/

void mandatoSetMotor(WiFiClient cliente) {
   // Leemos la velocidad deseada
  velocidad = cliente.parseInt();
  }

  
  void mandatoQuery(WiFiClient cliente) {
   
  // Esto es lo que hay
  cliente.println("\n\n************************************\n   Estos son los valores");
  cliente.print("Velocidad   ");  cliente.println(vDeseada[DERECHA]);
  cliente.print("Giro        ");  cliente.println(vDeseada[GIRO]);
}

// ***** defuelve la velocidad angular (gz)          
float omega()  {

  mpu6050.getRotation(&gx, &gy, &gz); // obten las velocidades angulares
  float gz_deg_s = gz * giroconv;    //  grados por segundo
    Serial.println(gz_deg_s);
    return  gz_deg_s;
}

