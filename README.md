# Atenea — Cohete de 2 etapas con aviónica avanzada

![Logo del proyecto](docs/logo.png)

Atenea es un proyecto de cohete casero de dos etapas orientado al desarrollo de una aviónica robusta, modular y experimental. El objetivo es integrar sensores, control de vuelo, telemetría y seguimiento de antena para conseguir un sistema capaz de monitorizar y gestionar el vuelo de forma segura.

El proyecto está organizado en distintas áreas de trabajo:

- firmware: software embebido para la electrónica de vuelo y la estación terrestre.
- hardware: diseño y construcción de estructuras, electrónica y componentes mecánicos.
- ground_station: herramientas para recepción y procesamiento de datos.
- docs: documentación técnica y referencias del proyecto.

## Características principales

- Cohete de dos etapas con lógica de vuelo supervisada.
- Aviónica basada en ESP32-S3 para adquisición de datos y control.
- Integración de sensores como giroscopio, acelerómetro y barómetro.
- Máquina de estados para gestionar fases como despegue, separación, apogeo y recuperación.
- Telemetría por radio para seguimiento en tiempo real.
- Sistema de rastreo de antena para mejorar la recepción durante el vuelo.
- Enfoque modular para facilitar pruebas, desarrollo y futuras mejoras.

## Arquitectura general

El sistema se organiza en varios subsistemas clave:

1. Sensado y control
   - Lectura continua de sensores de actitud y presión.
   - Procesado en tiempo real para estimar el comportamiento del vehículo.
   - Implementación de estrategias de estabilidad activa.

2. Máquina de estados de vuelo
   - Gestión de eventos críticos del vuelo.
   - Transiciones seguras entre distintas fases.
   - Supervisión para prevenir acciones no deseadas.

3. Telemetría y comunicación
   - Transmisión de datos desde el cohete hacia la estación en tierra.
   - Uso de módulos de radio de bajo coste y alta simplicidad para enlaces de corto y medio alcance.

4. Seguimiento de antena
   - Posicionamiento automático de una antena direccional para mantener la comunicación.
   - Integración con datos de localización del cohete.

## Estructura del repositorio

- [docs](docs): documentación técnica general del proyecto.
- [firmware](firmware): código fuente para la aviónica del cohete y la estación.
- [ground_station](ground_station): aplicaciones o scripts para la estación terrestre.
- [hardware](hardware): diseños, modelos 3D y esquemas.

## Documentación relacionada

La documentación técnica más completa se encuentra en [docs/Proyecto_Cohete_Avionica.md](docs/Proyecto_Cohete_Avionica.md).

## Estado del desarrollo

El proyecto se encuentra en una fase inicial de desarrollo, centrada en la definición de la arquitectura, la selección de sensores y la implementación de los primeros módulos de firmware y hardware.

## Próximos pasos

- Definir y validar la lógica de la máquina de estados.
- Implementar la lectura y fusión de sensores para estimación de actitud.
- Integrar telemetría con la estación terrestre.
- Probar los subsistemas de control y recuperación.
- Continuar el desarrollo del hardware y la estructura del cohete.

## Licencia

Este proyecto se encuentra en desarrollo y su uso está orientado a fines educativos y experimentales.
