// Librerias I2C para controlar el mpu6050
// la libreria MPU6050.h necesita I2Cdev.h, I2Cdev.h necesita Wire.h
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"

// La dirección del MPU6050 puede ser 0x68 o 0x69, dependiendo 
// del estado de AD0. Sin conectar o a GND, es 0x68
//Si no se especifica, 0x68 estará implicito
//conecta SCL a pin D2 y SDA a pin D1 del NodeMCU
#define D1 5
#define D2 4
#define SDA D1
#define SCL D2
          
MPU6050 mpu6050;

// Valores RAW (sin procesar) del acelerometro y giroscopio en los ejes x,y,z
short int ax, ay, az;
short int gx, gy, gz;
float ax_m_s2,  ay_m_s2,  az_m_s2 ;
float gx_deg_s,  gy_deg_s,  gz_deg_s ;
float  giroconv=  (250.0/32768.0);  // para 250grad/seg de sensibilidad
float  acelconv= (9.81/16384.0);    // para 2g de sensibilidad

void setup() {
  Serial.begin(38400);
  for (int i=0;i<10;i++) {
    delay(500);
    Serial.print("llevo esperando ");
    Serial.print(i*0.5);
    Serial.print( " segundos\n");
  }


  Wire.begin(SDA, SCL);           //Inicio I2C (SDA D1=5, SCL D2=4)  
  mpu6050.initialize();    //Iniciando mpu6050

  if (mpu6050.testConnection()) Serial.println("mpu6050 iniciado correctamente");
  else Serial.println("ERROR, ERROR ERROR  Error al iniciar el mpu6050");
}

void loop() {
  // Leer las aceleraciones y velocidades angulares
  mpu6050.getAcceleration(&ax, &ay, &az);
  mpu6050.getRotation(&gx, &gy, &gz);
  //tambin podemos leer la tempertura
  //int16_t rawtemp= mpu6050.getTemperature();
  //temp=(rawTemp/340.)+36.53;

  //Mostrar las lecturas separadas por un [tab]
  float ax_m_s2 = ax * acelconv;
  float ay_m_s2 = ay * acelconv;
  float az_m_s2 = az * acelconv;
  float gx_deg_s = gx * giroconv;
  float gy_deg_s = gy * giroconv;
  float gz_deg_s = gz * giroconv;
  //Mostrar las lecturas separadas por un [tab]
  Serial.print("a[x y z](m/s2) g[x y z](deg/s):\t");
  Serial.print(ax_m_s2); Serial.print("\t");
  Serial.print(ay_m_s2); Serial.print("\t");
  Serial.print(az_m_s2); Serial.print("\t");
  Serial.print(gx_deg_s); Serial.print("\t");
  Serial.print(gy_deg_s); Serial.print("\t");
  Serial.println(gz_deg_s);

  delay(100); // y me espero 100 mseg
}

