# Sistema IoT de Monitoreo Remoto para Motores Eléctricos mediante Comunicación LTE

## Descripción

Este proyecto presenta el diseño e implementación de un sistema IoT para el monitoreo remoto de motores eléctricos utilizando comunicación celular LTE.

La solución permite adquirir variables operativas críticas del motor, transmitirlas mediante una red celular hacia una infraestructura en la nube y visualizarlas en tiempo real a través de una plataforma web.

A diferencia de los sistemas de monitoreo tradicionales basados en redes locales o Wi-Fi, esta propuesta utiliza comunicación LTE mediante el módulo A7608SA-H, permitiendo supervisar equipos ubicados en zonas remotas sin depender de infraestructura de red existente.

El sistema está compuesto por un nodo de adquisición basado en ESP32, sensores industriales, un módulo de comunicación LTE, una infraestructura de procesamiento en AWS y una interfaz de visualización desarrollada en Streamlit.

---

# Objetivo General

Diseñar e implementar un sistema IoT para el monitoreo remoto de motores eléctricos mediante comunicación LTE, permitiendo la adquisición, transmisión, almacenamiento y visualización de variables operativas en tiempo real.

---

# Características Principales

✅ Comunicación LTE independiente de redes Wi-Fi.

✅ Monitoreo remoto desde cualquier ubicación.

✅ Adquisición de corriente RMS.

✅ Monitoreo de temperatura ambiente.

✅ Monitoreo de temperatura superficial del motor.

✅ Almacenamiento histórico de datos.

✅ Dashboard web en tiempo real.

✅ Arquitectura basada en servicios cloud.

✅ Detección de pérdida de comunicación.

---

# Arquitectura General

```text
┌─────────────────────────┐
│     Motor Eléctrico     │
└────────────┬────────────┘
             │
             ▼
┌─────────────────────────┐
│ Sensores                │
│                         │
│ SCT-013                 │
│ MLX90614                │
└────────────┬────────────┘
             │
             ▼
┌─────────────────────────┐
│ ESP32                   │
│ Procesamiento Local     │
└────────────┬────────────┘
             │ UART
             ▼
┌─────────────────────────┐
│ A7608SA-H LTE           │
│ Comunicación Celular    │
└────────────┬────────────┘
             │
             ▼
         Internet
             │
             ▼
┌─────────────────────────┐
│ AWS EC2                 │
│ Ubuntu Server           │
└────────────┬────────────┘
             │
             ▼
┌─────────────────────────┐
│ Backend Node.js         │
│ API REST                │
└────────────┬────────────┘
             │
             ▼
┌─────────────────────────┐
│ SQLite                  │
└────────────┬────────────┘
             │
             ▼
┌─────────────────────────┐
│ Dashboard Streamlit     │
└─────────────────────────┘
```

---

# Comunicación LTE

La comunicación remota constituye el núcleo de la solución.

El sistema emplea un módulo LTE A7608SA-H conectado al ESP32 mediante interfaz UART.

Las variables adquiridas son encapsuladas en formato JSON y transmitidas mediante solicitudes HTTP hacia un servidor desplegado en AWS EC2.

Esta arquitectura permite:

- Operación en ubicaciones remotas.
- Eliminación de dependencia de redes Wi-Fi.
- Cobertura geográfica ampliada.
- Monitoreo en tiempo real.
- Escalabilidad para múltiples dispositivos.

---

# Hardware Utilizado

## ESP32

Microcontrolador encargado de:

- Adquisición de sensores.
- Procesamiento local.
- Cálculo RMS.
- Gestión de comunicaciones.

## SCT-013

Sensor de corriente no invasivo utilizado para medir el consumo eléctrico del motor.

Características:

- Medición AC.
- Transformador de corriente.
- Instalación sin modificar el circuito de potencia.

## MLX90614

Sensor infrarrojo utilizado para monitorear:

- Temperatura ambiente.
- Temperatura superficial del motor.

Características:

- Comunicación I2C.
- Medición sin contacto.
- Alta precisión.

## A7608SA-H

Módulo LTE utilizado para:

- Conexión a Internet mediante red celular.
- Transmisión HTTP.
- Comunicación remota con el servidor.

---

# Procesamiento de Corriente

La corriente adquirida mediante el SCT-013 es procesada en el ESP32 para obtener el valor RMS.

Fórmula utilizada:

I_RMS = √[(1/N) Σ(Ii²)]

Donde:

- Ii corresponde a cada muestra adquirida.
- N es el número total de muestras.

El cálculo RMS permite estimar la carga eléctrica del motor y detectar variaciones de consumo.

---

# Infraestructura Cloud

La solución utiliza Amazon Web Services (AWS).

## Servidor

- Amazon EC2
- Ubuntu Server 24.04 LTS

## Servicios Ejecutados

### Backend

Node.js + Express

Funciones:

- Recepción de datos.
- API REST.
- Almacenamiento.
- Consulta histórica.

### Base de Datos

SQLite

Variables almacenadas:

- Temperatura ambiente.
- Temperatura del motor.
- Corriente RMS.
- Marca temporal.

### Dashboard

Streamlit

Funciones:

- Visualización en tiempo real.
- Históricos.
- Estado del sistema.
- Detección de pérdida de señal.

---

# Estructura del Repositorio

```text
SISTEMA-DE-MONITOREO-no-invasivo-MOTOR
│
├── firmware/
│   └── sensor_lte_v2.ino
│
├── backend/
│   ├── server.js
│   ├── package.json
│   └── package-lock.json
│
├── dashboard/
│   ├── app.py
│   └── requirements.txt
│
├── docs/
│   ├── api.md
│   ├── aws.md
│   ├── arquitectura.md
│   ├── hardware.md
│   └── medicion_corriente.md
│
├── images/
│
├── LICENSE
├── .gitignore
└── README.md
```

---

# Instalación

## Backend

```bash
cd backend
npm install
node server.js
```

## Dashboard

```bash
cd dashboard
pip install -r requirements.txt
streamlit run app.py
```

---

# Resultados

El sistema permite supervisar remotamente el estado operativo de motores eléctricos mediante comunicación LTE, proporcionando acceso a información en tiempo real desde cualquier ubicación con cobertura celular.

La arquitectura implementada demuestra la viabilidad de integrar tecnologías IoT, comunicaciones móviles y servicios cloud para aplicaciones de monitoreo industrial y mantenimiento predictivo.

---

# Autores

* Taufic Yusef Rapag Padilla
* Alejandra Tuiran Ospino
* Sebastian Pupo Solano
* Alejandro Rovira Brieva

Proyecto académico de Ingeniería Electrónica.
