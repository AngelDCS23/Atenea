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

  // ==========================================
  // MODO COHETE: AMPLIAR RANGOS DEL SENSOR
  // ==========================================
  sensor.setFullScaleGyroRange(MPU6050_GYRO_FS_2000); // Límite a 2000 grados/segundo
  sensor.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);  // Límite a 8G de fuerza

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
  // --- VACUNA CONTRA EL "nan" (Usar floats para evitar desbordamiento) ---
  float accelPitch = atan2(-ax, sqrt((float)ay * (float)ay + (float)az * (float)az)) * 180.0 / M_PI;

  // --- NUEVA MATEMÁTICA PARA EL GIROSCOPIO (Acorde a los 2000 grados/s) ---
  float gyroX_dps = gx / 16.4;
  float gyroY_dps = gy / 16.4;

  roll = 0.95 * (roll + gyroX_dps * dt) + 0.05 * accelRoll;
  pitch = 0.95 * (pitch + gyroY_dps * dt) + 0.05 * accelPitch;

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