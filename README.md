# Sistema IoT para Monitoreo de Motores Eléctricos mediante LTE

## Descripción

Sistema IoT basado en ESP32 para el monitoreo remoto de motores eléctricos mediante sensores de corriente y temperatura.

El sistema adquiere datos en tiempo real, calcula corriente RMS, monitorea temperatura mediante un sensor infrarrojo y transmite la información a un servidor remoto utilizando conectividad LTE.

## Características

- Medición de corriente AC mediante SCT-013
- Cálculo RMS en ESP32
- Medición de temperatura sin contacto
- Comunicación LTE mediante módulo A7608
- Envío de datos vía HTTP
- Diagnóstico avanzado de red LTE
- Detección de condiciones anómalas

## Hardware Utilizado

- ESP32
- Módulo LTE A7608
- Sensor SCT-013
- Sensor MLX90614
- Fuente de alimentación 5V

## Arquitectura del Sistema

[Imagen arquitectura]

Motor → Sensores → ESP32 → LTE → API REST → Base de Datos → Dashboard

## Variables Monitoreadas

| Variable | Unidad |
|-----------|---------|
| Corriente RMS | A |
| Temperatura Ambiente | °C |
| Temperatura Objeto | °C |
| RSSI | dBm |
| RSRP | dBm |
| RSRQ | dB |
| SINR | dB |

## Funciones Implementadas

### Adquisición de Datos

- Lectura del SCT-013
- Cálculo RMS
- Lectura del MLX90614

### Comunicación LTE

- Inicialización automática
- Registro en red
- Configuración APN
- HTTP POST

### Diagnóstico LTE

- RSSI
- Operador
- Tecnología de red
- ICCID
- IMEI
- RSRP
- RSRQ
- SINR

## Instalación

1. Instalar Arduino IDE.
2. Instalar soporte ESP32.
3. Instalar librerías:
   - Adafruit MLX90614
   - Wire

4. Cargar firmware:

```cpp
sensor_lte_v2.ino
