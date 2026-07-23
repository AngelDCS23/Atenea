#include <Arduino.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <ESP32Servo.h> 
#include <TinyGPSPlus.h>

#define GPS_RX_PIN 44
#define GPS_TX_PIN 43

MPU6050 sensor;
TinyGPSPlus gps;

unsigned long ultimoAviso = 0;

// --- CONFIGURACIÓN DE LOS SERVOS ---
Servo servo_EjeX;     // El que se moverá con la inclinación X
Servo servo_EjeY_1;   // El que se moverá con la inclinación Y (Motor 1)
Servo servo_EjeY_2;   // El que se moverá con la inclinación Y (Motor 2)

const int PIN_SERVO_X = D0; 
const int PIN_SERVO_Y1 = D1;
const int PIN_SERVO_Y2 = D2;

int16_t ax, ay, az;
int16_t gx, gy, gz;

float pitch = 0.0; // Lo usaremos como Eje X
float roll = 0.0;  // Lo usaremos como Eje Y

unsigned long tiempoAnterior;

const float SETPOINT_X = 0.0; 
const float SETPOINT_Y = 0.0;
const int CENTRO_SERVO = 90;

// Constantes PID Eje X
float KpX = 1.0;
float KiX = 0.0;
float KdX = 0.05;

// Constantes PID Eje Y
float KpY = 0.8;
float KiY = 0.0;
float KdY = 0.05;

// Memoria del PID
float errorPrevioX = 0.0;
float errorIntegralX = 0.0;
float errorPrevioY = 0.0;
float errorIntegralY = 0.0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  sensor.initialize();

  Serial1.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  sensor.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);
  sensor.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);

  if (sensor.testConnection()) {
      Serial.println("Sensor iniciado correctamente");
  } else {
      Serial.println("Error al iniciar el sensor");
  }

  // 1. Asignar temporizadores independientes
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // 2. Definir la frecuencia
  servo_EjeX.setPeriodHertz(50);
  servo_EjeY_1.setPeriodHertz(50);
  servo_EjeY_2.setPeriodHertz(50);

  // 3. Conectar a los pines
  servo_EjeX.attach(PIN_SERVO_X, 500, 2500);
  servo_EjeY_1.attach(PIN_SERVO_Y1, 500, 2500);
  servo_EjeY_2.attach(PIN_SERVO_Y2, 500, 2500);
  
  // 4. Centrar los motores
  servo_EjeX.write(CENTRO_SERVO);
  servo_EjeY_1.write(CENTRO_SERVO);
  servo_EjeY_2.write(CENTRO_SERVO);

  tiempoAnterior = micros();
}

void loop() {

  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  if (gps.location.isUpdated()) {
    Serial.print("Latitud: "); 
    Serial.print(gps.location.lat(), 6);
    Serial.print(" | Longitud: "); 
    Serial.print(gps.location.lng(), 6);
    Serial.print(" | Altitud: "); 
    Serial.print(gps.altitude.meters());
    Serial.print(" m | Satélites conectados: "); 
    Serial.println(gps.satellites.value());
  }

  if (millis() - ultimoAviso > 5000 && gps.satellites.value() == 0) {
    Serial.println("Buscando satélites...");
    ultimoAviso = millis();
  }

  sensor.getAcceleration(&ax, &ay, &az);
  sensor.getRotation(&gx, &gy, &gz);

  unsigned long tiempoActual = micros();
  float dt = (tiempoActual - tiempoAnterior) / 1000000.0;
  tiempoAnterior = tiempoActual;

  // Cálculo de inclinación (X e Y)
  float accelRoll = atan2(ay, az) * 180.0 / M_PI;
  float accelPitch = atan2(-ax, sqrt((float)ay * (float)ay + (float)az * (float)az)) * 180.0 / M_PI;

  float gyroX_dps = gx / 16.4;
  float gyroY_dps = gy / 16.4;

  // Pitch representará nuestro EJE X
  pitch = 0.95 * (pitch + gyroY_dps * dt) + 0.05 * accelPitch;
  // Roll representará nuestro EJE Y
  roll = 0.95 * (roll + gyroX_dps * dt) + 0.05 * accelRoll;


  // --- CÁLCULO PID EJE X (Solo afecta al D0) ---
  float errorActualX = SETPOINT_X - pitch;
  float proporcionalX = KpX * errorActualX;
  
  errorIntegralX = errorIntegralX + (errorActualX * dt);
  errorIntegralX = constrain(errorIntegralX, -20.0, 20.0);

  float integralX = KiX * errorIntegralX;
  float derivativoX = KdX * ((errorActualX - errorPrevioX) / dt);
  float salidaPID_X = proporcionalX + integralX + derivativoX;

  errorPrevioX = errorActualX;

  // --- CÁLCULO PID EJE Y (Afecta a D1 y D2) ---
  float errorActualY = SETPOINT_Y - roll;
  float proporcionalY = KpY * errorActualY;

  errorIntegralY = errorIntegralY + (errorActualY * dt);
  errorIntegralY = constrain(errorIntegralY, -20.0, 20.0);

  float integralY = KiY * errorIntegralY;
  float derivativoY = KdY * ((errorActualY - errorPrevioY) / dt);
  float salidaPID_Y = proporcionalY + integralY + derivativoY;

  errorPrevioY = errorActualY;

  // --- BANDA MUERTA ---
  if (abs(salidaPID_X) < 1.0) salidaPID_X = 0.0;
  if (abs(salidaPID_Y) < 1.0) salidaPID_Y = 0.0;

  // 1. El motor del pin D0 se mueve SOLO con el Eje X
  int anguloEjeX = CENTRO_SERVO - salidaPID_X;
  anguloEjeX = constrain(anguloEjeX, 60, 120); 
  servo_EjeX.write(anguloEjeX);

  // 2. Los motores de los pines D1 y D2 se mueven SOLO con el Eje Y
  int anguloEjeY = CENTRO_SERVO + salidaPID_Y; 
  anguloEjeY = constrain(anguloEjeY, 60, 120); 
  
  servo_EjeY_1.write(anguloEjeY);
  servo_EjeY_2.write(anguloEjeY); 

  Serial.print("Inclinacion_X:");
  Serial.print(pitch);
  Serial.print(" | Inclinacion_Y:");
  Serial.print(roll);
  Serial.print(" | Servo_D0:");
  Serial.print(anguloEjeX);
  Serial.print(" | Servos_D1_D2:");
  Serial.println(anguloEjeY); 
}