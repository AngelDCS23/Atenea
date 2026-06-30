# Proyecto Cohete Casero de 2 Etapas con Aviónica Avanzada (ESP32-S3)

Este documento recopila la estructura técnica, desafíos, soluciones y presupuesto estimado para el desarrollo de un cohete casero de dos etapas controlado por un **ESP32-S3 Sense**, incluyendo sistemas de estabilidad activa, telemetría y un rastreador de antena automatizado (*Antenna Tracker*).

---

## 1. Arquitectura del Sistema de Aviónica

Para ejecutar cálculos en tiempo real (como la estimación de actitud mediante fusión de sensores), el código del ESP32-S3 se estructurará en dos tareas principales aprovechando sus dos núcleos a 240 MHz:

* **Núcleo 0 (Control de Vuelo y Telemetría):** Lectura de la IMU (giroscopio/acelerómetro) y barómetro a alta frecuencia (100Hz - 200Hz), ejecución del algoritmo PID para los alerones activos y almacenamiento de datos en la tarjeta SD.
* **Núcleo 1 (Máquina de Estados de Vuelo y Cámara):** Gestión de la seguridad, detección de eventos de la Máquina de Estados (despegue, separación, apogeo), activación de servomotores de eyección y captura de imágenes/video.

### Máquina de Estados de Vuelo (FSM)
El software debe actuar como una máquina de estados finitos ultra-robusta sin el uso de funciones bloqueantes (`delay()`):
1.  **REPOSO (Pre-lanzamiento):** Calibración de sensores y espera de umbral de aceleración.
2.  **QUEMADO ETAPA 1:** Detección de aceleración positiva continua (> 2g).
3.  **SEPARACIÓN:** Caída drástica de la aceleración axial que indica el fin del empuje de la primera etapa.
4.  **ENCENDIDO ETAPA 2 (Condicionado):** *Regla de Oro de Seguridad:* Solo se activa si el ángulo de inclinación respecto a la vertical es inferior a 15°. Si es mayor, se aborta el encendido.
5.  **APOGEO:** Detección de velocidad vertical igual a cero y tendencia de presión barométrica negativa. Activación del paracaídas.
6.  **RECUPERACIÓN:** Transmisión de coordenadas GPS de alta prioridad hasta el aterrizaje.

---

## 2. Subsistemas Clave y Soluciones Técnicas

### Estabilidad Activa mediante Alerones (Opción Seleccionada)
* **Mecanismo:** 4 aletas aerodinámicas en la parte superior (*canards*) movidas por servomotores para corregir desviaciones en pleno vuelo.
* **Hardware:** Servos digitales **MG90S** con piñonería metálica, alimentados a 6V o 7.4V para maximizar la velocidad de respuesta (~0.08s por 60°). Al requerir deflexiones pequeñas (±5° a ±10°), el tiempo de reacción físico se reduce a unos ~6 milisegundos.
* **Software:** Algoritmo PID nativo utilizando el periférico MCPWM del ESP32 configurado a 200Hz o más, evitando ruidos de la IMU mediante filtros paso bajo (Low-Pass Filter).

### Propulsión y Combustible
* Se descartan los motores comerciales costosos y la cocción inestable de azúcar al fuego en interiores (riesgo crítico de CATO/explosión y autoignición).
* **Solución económica:** Uso de **Cohetes de Agua** (liberación mecánica mediante servos, coste 0€ por lanzamiento) o **Azúcar prensado en seco** (sin cocinar, requiriendo pruebas estáticas previas en un banco de tierra sin la electrónica a bordo).

### Telemetría y Enlace de Radio
* Se descarta el uso de tecnologías móviles (5G/4G) por el elevado coste de los módulos (100€-250€), las zonas muertas en altitud y el Efecto Doppler a alta velocidad.
* **Solución:** Módulos de radiofrecuencia de bajo coste como **LoRa (SX1278 a 868/915 MHz)** o transceptores **HC-12 (433 MHz)** conectados vía UART, permitiendo un alcance de entre 800m y 3km con antenas de látigo flexibles orientadas hacia abajo.

### Rastreador de Antena Automático (*Antenna Tracker*)
* **Funcionamiento:** Una estructura móvil *Pan-Tilt* en tierra equipada con una antena direccional de alta ganancia (tipo Yagi casera) que apunta automáticamente hacia el cohete.
* **Lógica:** El cohete envía sus coordenadas GPS (módulo **NEO-6M**). Un script de Python en el portátil (o un microcontrolador secundario en tierra) realiza el cálculo de trigonometría esférica (Azimut y Elevación) comparándolo con la posición de la estación base y mueve dos servos de alta potencia (**MG996R**).

---

## 3. Desglose del Presupuesto Estimado

A continuación se detalla la tabla de precios calculada para el enfoque de desarrollo "Low-Cost", asumiendo que ya se dispone del portátil y del procesador principal (**ESP32-S3 Sense**).

| Componente / Subsistema | Descripción / Función técnica | Cantidad | Precio Unitario (€) | Precio Total Estimado (€) |
| :--- | :--- | :---: | :---: | :---: |
| **Módulo GPS NEO-6M** | Localización del cohete y guiado del rastreador | 1 | 5,00 € | 5,00 € |
| **Transmisores de Radio** | Enlace inalámbrico (Módulos LoRa o HC-12) para cohete y tierra | 2 | 5,00 € | 10,00 € |
| **Servos Digitales MG90S** | Servomotores rápidos con piñones de metal para los alerones activos | 2 | 4,00 € | 8,00 € |
| **Micro Servo SG90** | Servomotor básico para el pestillo de eyección del paracaídas | 1 | 2,00 € | 2,00 € |
| **Batería LiPo 2S (7.4V)** | Alimentación dedicada para servos y aviónica del cohete (ej. 450mAh) | 1 | 7,00 € | 7,00 € |
| **Adaptador USB a TTL** | Chip CP2102 para conectar el receptor de radio de tierra al portátil | 1 | 2,00 € | 2,00 € |
| **Microcontrolador Tierra** | Placa Arduino Nano (clon) para controlar los servos del rastreador | 1 | 3,00 € | 3,00 € |
| **Servos de Potencia MG996R** | Servomotores de alto torque para el movimiento Pan-Tilt de la antena | 2 | 5,00 € | 10,00 € |
| **Antena Yagi Casera** | Fabricación propia con varillas de metal/perchas y conectores | 1 | 2,00 € | 2,00 € |
| **Fuselaje y Estructura** | Tubos de cartón/PVC, botellas PET, tornillería y filamento 3D | - | 10,00 € | 10,00 € |
| **Material Paracaídas** | Tela ligera de paraguas viejo o bolsa resistente e hilo de pescar | 1 | 2,00 € | 2,00 € |
| **TOTAL ESTIMADO** | **Coste de hardware para el sistema aeroespacial completo** | - | - | **61,00 €** |

*Nota sobre la propulsión:* Si se opta por cohetes de agua, el coste operativo es de 0,00€. En caso de utilizar propulsor de azúcar (KNO3 + Azúcar glass), los ingredientes químicos añaden aproximadamente **10,00 €** adicionales para múltiples lotes de prueba.

---

## 4. Próximos Pasos con el Giroscopio de 6 Ejes

Dado que vas a comenzar por el sistema de medición de actitud, ten en cuenta estas tres pautas clave para tu IMU de 6 ejes:

1.  **Configuración de Rango Máximo:** Configura el giroscopio a su escala completa (típicamente **±2000 °/s**) y el acelerómetro a **±16g**. Los valores por defecto de desarrollo (como ±2g) se saturarán instantáneamente con la vibración y aceleración del despegue.
2.  **Amortiguación Mecánica:** No atornilles ni pegues la IMU directamente a la estructura rígida del cohete. El ruido de alta frecuencia del motor falseará las lecturas. Usa espuma de alta densidad o gel de silicona entre el chasis y la PCB.
3.  **Algoritmo de Fusión:** Implementa un filtro complementario (ligero y rápido) o un filtro de Kalman optimizado en el Núcleo 0 para combinar las lecturas del giroscopio (preciso a corto plazo) con el acelerómetro (estable a largo plazo) y obtener el ángulo real de inclinación en tiempo real.
