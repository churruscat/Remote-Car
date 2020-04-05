#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "ESP8266FtpServer.h"   //https://github.com/nailbuster/esp8266FTPServer
#include <FS.h>


ESP8266WebServer server(80);
FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial

const char* ssid = "WLANCasa";
const char* password = "can(M0rras).c0n5";
//const char* ssid = "Churruscat";
//const char* password = "biba1Sailing";


void handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!");
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void wifiConnect() {
 int i=0;
 Serial.print("Conectando a WiFi  "); Serial.println(ssid);
 WiFi.disconnect(); // Desconecto para impiar la configuracion y que conecte mas rapido
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(i++);Serial.print(".");
 }
 Serial.println(ssid);  Serial.print("*******Conectado; ADDR= ");
 Serial.println(WiFi.localIP());
}

void setup(void){
  FSInfo fsInfo; 
  
  Serial.begin(115200);
  wifiConnect();
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  /////FTP Setup, ensure SPIFFS is started before ftp;  /////////
 
  if (SPIFFS.begin()) {
          SPIFFS.format();
    Serial.println("SPIFFS opened!");
     /* si no habia nada, formateo, creo el fichero y pongo 
     *  los valores por defecto en el fichero "metadata"
     */

    if (!SPIFFS.info(fsInfo)) {  
      Serial.println(" formateo, y cargo valores por defecto");
      SPIFFS.format();
    } else {
      Serial.println(" SPIFFS ya estaba Formateado!"); 
    }   
    ftpSrv.begin("8266","8266");    //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)
  }    
}

void loop(void){
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!  
  server.handleClient();
 
}
