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

// --- CONSTANTES DE CONTROL ---
float Kp = 1.5; 
const int CENTRO_SERVO = 90;

void setup() {
  Serial.begin(57600);
  Wire.begin();
  sensor.initialize();

  if (sensor.testConnection()) {
      Serial.println("Sensor iniciado correctamente");
  } else {
      Serial.println("Error al iniciar el sensor");
  }

  // Inicializar y centrar el servo
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
  float accelPitch = atan2(-ax, sqrt((long)ay * ay + (long)az * az)) * 180.0 / M_PI;

  float gyroX_dps = gx / 131.0;
  float gyroY_dps = gy / 131.0;

  roll = 0.98 * (roll + gyroX_dps * dt) + 0.02 * accelRoll;
  pitch = 0.98 * (pitch + gyroY_dps * dt) + 0.02 * accelPitch;

  // ==========================================
  // CONTROL DE VUELO (1 EJE)
  // ==========================================
  int anguloPitch = CENTRO_SERVO - (pitch * Kp);
  anguloPitch = constrain(anguloPitch, 60, 120);
  servoPitch.write(anguloPitch);

  Serial.print(pitch);
  Serial.print(",");
  Serial.println(roll);

  delay(10);
}