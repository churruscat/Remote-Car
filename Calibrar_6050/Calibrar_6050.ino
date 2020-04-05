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

//Variables usadas por el filtro pasa bajos
long f_ax,f_ay, f_az;
short int p_ax, p_ay, p_az;
long f_gx,f_gy, f_gz;
short int p_gx, p_gy, p_gz;
short int counter=0;

//Valor de los offsets
int ax_o,ay_o,az_o;
int gx_o,gy_o,gz_o;

void setup() {
  Serial.begin(38400);   //Inicio puerto serie
 Wire.begin(SDA, SCL);           //Inicio I2C (SDA D1=5, SCL D2=4)  
  mpu6050.initialize();    //Inicio el mpu6050

  if (mpu6050.testConnection()) Serial.println("mpu6050 iniciado correctamente");

  // Leer los offset los offsets anteriores
  ax_o=mpu6050.getXAccelOffset();
  ay_o=mpu6050.getYAccelOffset();
  az_o=mpu6050.getZAccelOffset();
  gx_o=mpu6050.getXGyroOffset();
  gy_o=mpu6050.getYGyroOffset();
  gz_o=mpu6050.getZGyroOffset();
  
  Serial.println("Offsets:");
  Serial.print(ax_o); Serial.print("\t"); 
  Serial.print(ay_o); Serial.print("\t"); 
  Serial.print(az_o); Serial.print("\t"); 
  Serial.print(gx_o); Serial.print("\t"); 
  Serial.print(gy_o); Serial.print("\t");
  Serial.print(gz_o); Serial.print("\t");
  Serial.println("nnEnvie cualquier caracter para empezar la calibracionnn");  
  // Espera un caracter para empezar a calibrar
  while (true){if (Serial.available()) break;}  
  Serial.println("Calibrando, no mover IMU");  
  
}

void loop() {
  // Leer las aceleraciones y velocidades angulares
  mpu6050.getAcceleration(&ax, &ay, &az);
  mpu6050.getRotation(&gx, &gy, &gz);

  // Filtrar las lecturas
  f_ax = f_ax-(f_ax>>5)+ax;
  p_ax = f_ax>>5;

  f_ay = f_ay-(f_ay>>5)+ay;
  p_ay = f_ay>>5;

  f_az = f_az-(f_az>>5)+az;
  p_az = f_az>>5;

  f_gx = f_gx-(f_gx>>3)+gx;
  p_gx = f_gx>>3;

  f_gy = f_gy-(f_gy>>3)+gy;
  p_gy = f_gy>>3;

  f_gz = f_gz-(f_gz>>3)+gz;
  p_gz = f_gz>>3;

  //Cada 100 lecturas corregir el offset
  if (counter==100){
    //Mostrar las lecturas separadas por un [tab]
    Serial.print("promedio:"); Serial.print("t");
    Serial.print(p_ax); Serial.print("\t");
    Serial.print(p_ay); Serial.print("\t");
    Serial.print(p_az); Serial.print("\t");
    Serial.print(p_gx); Serial.print("\t");
    Serial.print(p_gy); Serial.print("\t");
    Serial.println(p_gz);

    //Calibrar el acelerometro a 1g en el eje z (ajustar el offset)
    if (p_ax>0) ax_o--;
    else {ax_o++;}
    if (p_ay>0) ay_o--;
    else {ay_o++;}
    if (p_az-16384>0) az_o--;
    else {az_o++;}
    
    mpu6050.setXAccelOffset(ax_o);
    mpu6050.setYAccelOffset(ay_o);
    mpu6050.setZAccelOffset(az_o);

    //Calibrar el giroscopio a 0º/s en todos los ejes (ajustar el offset)
    if (p_gx>0) gx_o--;
    else {gx_o++;}
    if (p_gy>0) gy_o--;
    else {gy_o++;}
    if (p_gz>0) gz_o--;
    else {gz_o++;}
    
    mpu6050.setXGyroOffset(gx_o);
    mpu6050.setYGyroOffset(gy_o);
    mpu6050.setZGyroOffset(gz_o);    

    counter=0;
  }
  counter++;
}

