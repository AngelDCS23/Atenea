#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"

MPU6050 sensor;

int16_t ax, ay, az;
int16_t gx, gy, gz;

// Nuevas variables para guardar los ángulos calculados
float pitch = 0.0;
float roll = 0.0;

// Reloj para medir el tiempo entre ciclos (necesario para el giroscopio)
unsigned long tiempoAnterior;

void setup() {
  Serial.begin(57600);
  Wire.begin();
  sensor.initialize();

  if (sensor.testConnection()) {
      Serial.println("Sensor iniciado correctamente");
  } else {
      Serial.println("Error al iniciar el sensor");
  }

  // Guardamos el tiempo justo antes de empezar a leer
  tiempoAnterior = micros();
}

void loop() {
  // 1. LECTURA (Intacta, tal como la tenías)
  sensor.getAcceleration(&ax, &ay, &az);
  sensor.getRotation(&gx, &gy, &gz);

  // 2. MATEMÁTICAS (Inyectadas en medio)
  // Calculamos la diferencia de tiempo (dt) en segundos
  unsigned long tiempoActual = micros();
  float dt = (tiempoActual - tiempoAnterior) / 1000000.0;
  tiempoAnterior = tiempoActual;

  // Calculamos la inclinación a través de la gravedad (Acelerómetro)
  float accelRoll = atan2(ay, az) * 180.0 / M_PI;
  float accelPitch = atan2(-ax, sqrt((long)ay * ay + (long)az * az)) * 180.0 / M_PI;

  // Calculamos la velocidad de rotación pura (Giroscopio)
  float gyroX_dps = gx / 131.0;
  float gyroY_dps = gy / 131.0;

  // Aplicamos el Filtro Complementario (98% Gyro + 2% Acel)
  roll = 0.98 * (roll + gyroX_dps * dt) + 0.02 * accelRoll;
  pitch = 0.98 * (pitch + gyroY_dps * dt) + 0.02 * accelPitch;

  // 3. ENVÍO DE DATOS AL DASHBOARD
  // Formato: pitch,roll
  Serial.print(pitch);
  Serial.print(",");
  Serial.println(roll);

  // Reducido de 100ms a 10ms. 
  // Esto hace que lea a ~100Hz, vital para que el horizonte artificial 
  // se mueva fluido y la integración del giroscopio no acumule error.
  delay(10);
}