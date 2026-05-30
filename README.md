# Sistema IoT para Monitoreo Remoto de Motores Eléctricos mediante Comunicación LTE
http://3.150.75.105:8501/
## Descripción

Este proyecto presenta el diseño e implementación de una plataforma IoT para el monitoreo remoto de motores eléctricos utilizando comunicación celular LTE y servicios cloud.

La solución permite adquirir variables eléctricas y térmicas del motor, transmitirlas mediante redes móviles hacia un servidor remoto alojado en AWS y visualizarlas en tiempo real mediante una interfaz web.

El sistema fue diseñado para operar en ubicaciones donde no existe conectividad Wi-Fi o infraestructura de red local, utilizando el módulo LTE A7608SA-H como medio principal de comunicación.

---

## Objetivo General

Diseñar e implementar un sistema IoT para el monitoreo remoto de motores eléctricos mediante comunicación LTE, permitiendo la adquisición, transmisión, almacenamiento y visualización de variables operativas en tiempo real.

---

## Características

- Comunicación LTE mediante A7608SA-H.
- Monitoreo remoto desde cualquier ubicación.
- Medición de corriente RMS.
- Monitoreo de temperatura ambiente.
- Monitoreo de temperatura superficial del motor.
- Almacenamiento histórico.
- Dashboard web en tiempo real.
- Infraestructura cloud en AWS.
- Diagnóstico de calidad de señal LTE.
- Identificación automática de operador celular.
- Medición de latencia de comunicación.

---

## Arquitectura General

```text
Motor Eléctrico
        │
        ▼
 ┌───────────────┐
 │   SCT-013     │
 └──────┬────────┘
        │
 ┌──────▼────────┐
 │   MLX90614    │
 └──────┬────────┘
        │
        ▼
 ┌───────────────┐
 │     ESP32     │
 └──────┬────────┘
        │ UART
        ▼
 ┌───────────────┐
 │  A7608SA-H    │
 │     LTE       │
 └──────┬────────┘
        │
     Internet
        │
        ▼
 ┌───────────────┐
 │   AWS EC2     │
 └──────┬────────┘
        │
        ▼
 ┌───────────────┐
 │ Node.js API   │
 └──────┬────────┘
        │
        ▼
 ┌───────────────┐
 │    SQLite     │
 └──────┬────────┘
        │
        ▼
 ┌───────────────┐
 │  Streamlit    │
 └───────────────┘
```

---

## Variables Monitoreadas

### Temperatura Ambiente

Sensor:

MLX90614

Unidad:

°C

---

### Temperatura Superficial del Motor

Sensor:

MLX90614

Unidad:

°C

---

### Corriente RMS

Sensor:

SCT-013

Unidad:

A

---

## Comunicación LTE

La comunicación constituye el núcleo del sistema.

El ESP32 transmite los datos utilizando el módulo celular A7608SA-H mediante solicitudes HTTP sobre la red LTE.

Los datos son enviados en formato JSON hacia una API REST desplegada en AWS.

Ejemplo:

```json
{
  "temp_ambiente": 27.4,
  "temp_objeto": 61.8,
  "corriente": 3.25
}
```

---

## Diagnóstico LTE

El firmware incorpora un módulo de diagnóstico capaz de obtener:

- Intensidad de señal RSSI.
- BER (Bit Error Rate).
- Operador celular.
- Tecnología de acceso.
- Dirección IP asignada.
- ICCID.
- IMEI.
- Latencia hacia el servidor.

Comandos utilizados:

| Función | Comando |
|----------|----------|
| Estado módulo | AT |
| Estado SIM | AT+CPIN? |
| Registro en red | AT+CREG? |
| Intensidad señal | AT+CSQ |
| Operador | AT+COPS? |
| Dirección IP | AT+CGPADDR |
| HTTP POST | AT+HTTPACTION=1 |

---

## Infraestructura Cloud

### AWS EC2

Sistema operativo:

Ubuntu Server 24.04 LTS

### Backend

Node.js + Express

Funciones:

- Recepción de datos.
- API REST.
- Consultas históricas.
- Gestión de almacenamiento.

### Base de Datos

SQLite

### Dashboard

Streamlit

Visualización en tiempo real de:

- Temperatura ambiente.
- Temperatura del motor.
- Corriente RMS.
- Estado de comunicación.

---

## Estructura del Proyecto

```text
firmware/
backend/
dashboard/
docs/
images/
```

---

## Instalación

### Backend

```bash
cd backend
npm install
node server.js
```

### Dashboard

```bash
cd dashboard
pip install -r requirements.txt
streamlit run app.py
```

---

## Tecnologías Utilizadas

- ESP32
- MLX90614
- SCT-013
- A7608SA-H
- LTE
- HTTP
- JSON
- Node.js
- SQLite
- Streamlit
- AWS EC2

---

## Autores

* Taufic Yusef Rapag Padilla
* Alejandra Tuiran Ospino
* Sebastian Pupo Solano
* Alejandro Rovira Brieva

Proyecto académico de Ingeniería Electrónica.
