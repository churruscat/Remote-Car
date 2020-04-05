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
char ssid1[] = "WLANCasa";
char password1[] = "can(M0rras).c0n5";

//char ssid1[]     = "Churruscat";
//char password1[] = "biba1Sailing";
char ssid2[] ="Coche-M";
char password2[] = "amodelanoche";
IPAddress miIP(192, 168, 0, 50); // this 3 lines for a fix IP-address
IPAddress miGateway(172,20,10,7);

short int i;
// definimos los PINs del motor
#define POWER_A  D1        // 5
#define POWER_B  D2        // 4 
#define DIRECCION_A   D3   // 0 
#define DIRECCION_B   D4   // 2 
#define SDA D5   // Conexiones del acelerometro MPU6050
#define SCL D6

// Defino  el webserver
ESP8266WebServer server(80);
/*String jQuery="jquery-3.2.1.min.js";
String jQueryKnob="jquery.knob.min.js";
String g_mandos="g_mandos.js";*/
char  form[10000]="<html>"
                "<head>"
                    "<title>Manejo coche Marti</title>"                    
                    "<script src=\"jquery-3.2.1.min.js\"></script>"
                    "<script src=\"jquery.knob.min.js\"></script>"
                    "<script src=\"g_mandos.js\"></script>"
                    "<style>body{padding:0;margin: 0px 0px;"
                          "font-family: \"Helvetica Neue\", Helvetica, Arial, sans-serif;"
                          "font-weight: 300; text-rendering: optimizelegibility;}"
                        "p{font-size: 30px; line-height: 30px}"
                        "div.controles{text-align: center; width: 50%; float: left}"
                        "div.controles > p{font-size: 20px}"
                    "</style>"
                "</head>"
                "<body>"
                    "<div  style=\"text-align: center; width:100%;font-size:200%;\">"
                        "<h1>Coche equipo 3C</h1>"
                    "</div>"
                    "<div style=\"clear:both\"></div>"
            "<!-- ahora el acelerador -->"
                    "<div class=\"controles\">"
                        "<p> Acelerador</p>"
                        "<input class=\"acelerador\" data-angleOffset=-90 data-angleArc=180 data-min=-50"
                            "data-max=50 data-fgColor=\"#FF0000\" data-thickness=\"0.6\" data-width=\"100%\""
                            "data-cursor=true data-rotation=\"clockwise\" value=\"0\">"
                  "</div>"
            "<!-- y el volante -->"
                    "<div class=\"controles\"> "
                    "<p> Volante</p>"
                       "<input class=\"volante\" data-angleOffset=-150 data-angleArc=300 data-min=-50"
                            "data-max=50 data-fgColor=\"#00EE00\" data-thickness=\"0.3\"" 
                            "data-cursor=true data-rotation=\"clockwise\" value=\"0\" data-width=\"100%\">"
                    "</div>"
                "</body>"
            "</html>",
      formulario[10000];
//      jquery[86660],
//      jqueryKnob[10805],
//      g_mandos[2100];
/****************************************************
 * g_mandos envia por POST el valor
 *  http://172.20.10.7/orden/valor:v,p
 *  donde v=posicion del volante
 *        p=posicion del acelerador
 * 
 */
File fJS1,fJS2;
// Formulario a enviar al navegador Web
//File  ficheroValores= SPIFFS.open(fichero, "r+");
char pagina [1000];
//y la parte de datos

int potencia=500;

WiFiClient cliente;
MPU6050 mpu6050;
#define pasoPotencia  100  // incrementos y decremento de potencia
#define potenciaMax  1023
int casiPotenciaMax=potenciaMax-pasoPotencia;
#define potenciaMin  200
int casiPotenciaMin=potenciaMin+pasoPotencia;
#define pasoGiro      100  // incrementos y decrementos de giro
#define giroMax      1000
int casiGiroMax=giroMax-pasoGiro;
#define kGiro         40  // constante para suavizar el giro. Cuanto mas pequenya, menos suaviza
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
  char*formulario;
  Wire.begin(SDA, SCL);           //Inicio I2C (SDA D5, SCL D6)  
  mpu6050.initialize();    //Iniciando mpu6050
  if (mpu6050.testConnection()) Serial.println("mpu6050 iniciado correctamente");
  else Serial.println("ERROR, ERROR ERROR  Error al iniciar el mpu6050");
  SPIFFS.begin();
  
  formulario=form;
              

 /****************************************************************
  * ahora me conecto a la WiFi, Esta el codigo con Acces point (comentado)
  * y con el ESP8266 haciendo de access point, 
 ******************************************************************/
  WiFi.disconnect(); // Desconecto para impiar la configuracion y que conecte mas rapido
  ssid=ssid1;
  password=password1;
  Serial.print("Connecting to ");Serial.println(ssid);
//  WiFi.config(miIP, miGateway, IPAddress(255,255,255,0));
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
    server.on("/orden", una_orden);
    server.on("/", cargaPag); 
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
  delay(100);
  server.handleClient();  //obtiene los datos de velocidad y giro del mando
  ajustaVelocidad();    // ajusta las velocidades para que giroActual sea giroDeseado
}
/******************************************************************
 *                         Funcion
 *                         Funcion                         
 *                         Funcion
*****************************************************************/ 
void cargaPag () {
    Serial.println(formulario);
    server.send(200, "text/html", formulario);
}
 
void una_orden() { 

  // el argumento debe ser "orden", en funcion de lo que llegue, asigno los valores de vMandoAhora
  if (server.arg("orden")) {
    Serial.print("argumento 1");Serial.println(server.arg(1));
    Serial.print("argumento 2");Serial.println(server.arg(2));
    Serial.print("argumento 2");Serial.println(server.arg(3));
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
        case 3:   // hacia atras recto. 
          vMandoAhora.giro=0;               
          vMandoAhora.derecha=potenciaMin;
          vMandoAhora.sentido=-1;
          break;
        case 4:   // hacia delante recto. 
          vMandoAhora.giro=0; 
          vMandoAhora.derecha=potenciaMin;
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
  // y envia respuesta al navegador
  vMandoAntes=vMandoAhora;  
/*
  sprintf(pagina,"%s %s %d %s %d %s %d %s",
  form,datosVelocidad,vMandoCorregido.derecha,
  datosGiro,vMandoCorregido.giro,
  datosGiroscopio,0,
  datosCierre);
  server.send(200, "text/html", pagina);
*/
  server.send(200, "text/html", "");
  ajustaVelocidad();
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
    if ((vDeseada.derecha*vMandoAhora.sentido)>0) digitalWrite(DIRECCION_A , HIGH);
    else digitalWrite(DIRECCION_A , LOW);
    if ((vDeseada.izquierda*vMandoAhora.sentido>0)) digitalWrite(DIRECCION_B , HIGH);
    else digitalWrite(DIRECCION_B , LOW);
  } else {
    vDeseada.derecha=0;
    vDeseada.izquierda=0;    
  }
  analogWrite(POWER_A , vDeseada.derecha);
  analogWrite(POWER_B , vDeseada.izquierda);
}  

