#include <Arduino.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <ESP32Servo.h> 

MPU6050 sensor;

// --- CONFIGURACIÓN DEL ÚNICO SERVO ---
Servo servoPitch;
const int PIN_SERVO_PITCH = D1; 

int16_t ax, ay, az;
int16_t gx, gy, gz;

float pitch = 0.0;
float roll = 0.0;

unsigned long tiempoAnterior;

const float SETPOINT = 0.0; 
const int CENTRO_SERVO = 90;

// Constantes de "Agresividad" (Se necesita una revisión de estos valores, el servor actual (SG90) aún sigue mostrando un temblor en variaciones leves.)
float Kp = 1.0;  // Fuerza bruta
float Ki = 0.0;  // Corrección a largo plazo
float Kd = 0.05; // Freno amortiguador

// Memoria del PID
float errorPrevio = 0.0;
float errorIntegral = 0.0;

void setup() {
  Serial.begin(57600);
  Wire.begin();
  sensor.initialize();

  sensor.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);
  sensor.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);

  if (sensor.testConnection()) {
      Serial.println("Sensor iniciado correctamente");
  } else {
      Serial.println("Error al iniciar el sensor");
  }

  servoPitch.attach(PIN_SERVO_PITCH, 500, 2500); 
  servoPitch.write(CENTRO_SERVO);

  tiempoAnterior = micros();
}

void loop() {
  sensor.getAcceleration(&ax, &ay, &az);
  sensor.getRotation(&gx, &gy, &gz);

  unsigned long tiempoActual = micros();
  float dt = (tiempoActual - tiempoAnterior) / 1000000.0;
  tiempoAnterior = tiempoActual;

  float accelRoll = atan2(ay, az) * 180.0 / M_PI;
  float accelPitch = atan2(-ax, sqrt((float)ay * (float)ay + (float)az * (float)az)) * 180.0 / M_PI;

  float gyroX_dps = gx / 16.4;
  float gyroY_dps = gy / 16.4;

  roll = 0.95 * (roll + gyroX_dps * dt) + 0.05 * accelRoll;
  pitch = 0.95 * (pitch + gyroY_dps * dt) + 0.05 * accelPitch;


  float errorActual = SETPOINT - pitch;
  float proporcional = Kp * errorActual;
  
  errorIntegral = errorIntegral + (errorActual * dt);
  errorIntegral = constrain(errorIntegral, -20.0, 20.0); // Anti-Windup REVISAR

  float integral = Ki * errorIntegral;
  float derivativo = Kd * ((errorActual - errorPrevio) / dt);
  float salidaPID = proporcional + integral + derivativo;

  errorPrevio = errorActual;

  // --- FILTRO DE BANDA MUERTA ---
  // Ignoramos ruidos menores a 1 grado para evitar vibraciones REVISAR (No estoy seguro si es por los ajustes de agresividad pero el servo (SG90) tiene unos leves temblores que aún no he conseguido identificar)
  if (abs(salidaPID) < 1.0) {
      salidaPID = 0.0;
  }

  int anguloPitch = CENTRO_SERVO - salidaPID;
  anguloPitch = constrain(anguloPitch, 60, 120); // Límite mecánico de la tobera
  servoPitch.write(anguloPitch);

  
  Serial.print(pitch);
  Serial.print(",");
  Serial.print(roll);
  Serial.print(",0.0,0.0,"); // Altitud y Velocidad enviadas falsas como 0.0 por ahora
  Serial.println(anguloPitch); // Enviamos qué está decidiendo hacer el servo

  delay(10);
}