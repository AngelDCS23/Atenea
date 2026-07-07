#include <Arduino.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <ESP32Servo.h> 

MPU6050 sensor;

Servo servoPitch1;
Servo servoPitch2;
const int PIN_SERVO_PITCH1 = 1;
const int PIN_SERVO_PITCH2 = 2; 

Servo servoRoll;
const int PIN_SERVO_ROLL = 3; 

int16_t ax, ay, az;
int16_t gx, gy, gz;

float pitch = 0.0;
float roll = 0.0;
unsigned long tiempoAnterior;

const float SETPOINT = 0.0; 
const int CENTRO_SERVO = 90;

float Kp = 1.0;  
float Ki = 0.0;  
float Kd = 0.05; 

float errorPrevioPitch = 0.0;
float errorIntegralPitch = 0.0;

float errorPrevioRoll = 0.0;
float errorIntegralRoll = 0.0;

void setup() {
  Serial.begin(115200);
  Wire.begin(5, 6); 
  
  sensor.initialize();

  sensor.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);
  sensor.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);

  if (sensor.testConnection()) {
      Serial.println("Sensor MPU6050 iniciado correctamente con GPIO 5 y 6");
  } else {
      Serial.println("Error al iniciar el sensor MPU6050");
  }

  servoPitch1.attach(PIN_SERVO_PITCH1, 500, 2500); 
  servoPitch2.attach(PIN_SERVO_PITCH2, 500, 2500); 
  servoPitch1.write(CENTRO_SERVO);
  servoPitch2.write(CENTRO_SERVO);

  servoRoll.attach(PIN_SERVO_ROLL, 500, 2500);
  servoRoll.write(CENTRO_SERVO);

  tiempoAnterior = micros();
}

void loop() {
  // 1. LEER SENSOR
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

  float errorPitch = SETPOINT - pitch;
  
  errorIntegralPitch += (errorPitch * dt);
  errorIntegralPitch = constrain(errorIntegralPitch, -20.0, 20.0); 

  float derivativoPitch = (errorPitch - errorPrevioPitch) / dt;
  float salidaPID_Pitch = (Kp * errorPitch) + (Ki * errorIntegralPitch) + (Kd * derivativoPitch);
  errorPrevioPitch = errorPitch;

  if (abs(salidaPID_Pitch) < 1.0) salidaPID_Pitch = 0.0;

  int anguloPitch = CENTRO_SERVO - salidaPID_Pitch;
  anguloPitch = constrain(anguloPitch, 60, 120);

  servoPitch1.write(anguloPitch);
  servoPitch2.write(anguloPitch); 


  float errorRoll = SETPOINT - roll;
  
  errorIntegralRoll += (errorRoll * dt);
  errorIntegralRoll = constrain(errorIntegralRoll, -20.0, 20.0);

  float derivativoRoll = (errorRoll - errorPrevioRoll) / dt;
  float salidaPID_Roll = (Kp * errorRoll) + (Ki * errorIntegralRoll) + (Kd * derivativoRoll);
  errorPrevioRoll = errorRoll;

  if (abs(salidaPID_Roll) < 1.0) salidaPID_Roll = 0.0;

  int anguloRoll = CENTRO_SERVO + salidaPID_Roll; 
  anguloRoll = constrain(anguloRoll, 60, 120);

  servoRoll.write(anguloRoll);

  Serial.print("Pitch:"); Serial.print(pitch); Serial.print(",");
  Serial.print("Roll:");  Serial.print(roll);  Serial.print(",");
  Serial.print("Angulo_Pitch:"); Serial.print(anguloPitch); Serial.print(",");
  Serial.print("Angulo_Roll:");  Serial.println(anguloRoll);

  delay(10); 
}