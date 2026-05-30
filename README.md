# Sistema de Monitoreo No Invasivo para Motores Eléctricos mediante IoT y Comunicación LTE

## Descripción General

Este proyecto implementa un sistema IoT para el monitoreo remoto y no invasivo de motores eléctricos mediante la adquisición de variables críticas de operación, específicamente temperatura y corriente RMS. La solución permite supervisar el estado del motor en tiempo real desde cualquier ubicación con acceso a Internet, facilitando la detección temprana de condiciones anómalas y contribuyendo a estrategias de mantenimiento predictivo.

El sistema está compuesto por un nodo embebido basado en ESP32, sensores de temperatura y corriente, un módulo de comunicación celular LTE A7608SA-H, una infraestructura de almacenamiento y procesamiento en la nube sobre Amazon Web Services (AWS), y una interfaz web desarrollada en Streamlit para la visualización de los datos.

---

## Objetivos

### Objetivo General

Diseñar e implementar un sistema IoT para el monitoreo remoto no invasivo de motores eléctricos mediante la adquisición, transmisión, almacenamiento y visualización de variables de operación.

### Objetivos Específicos

* Medir la corriente RMS del motor utilizando sensores de corriente AC no invasivos.
* Supervisar la temperatura ambiente y la temperatura superficial del motor.
* Transmitir datos mediante comunicación celular LTE.
* Implementar un backend para recepción y almacenamiento de información.
* Desarrollar una interfaz gráfica para visualización en tiempo real.
* Generar alertas ante condiciones anormales de operación.

---

## Arquitectura del Sistema

```text
┌─────────────────┐
│ Motor Eléctrico │
└────────┬────────┘
         │
         ▼
┌────────────────────────────┐
│ Sensores                   │
│ • Corriente RMS (SCT-013)  │
│ • Temperatura              │
└────────┬───────────────────┘
         │
         ▼
┌────────────────────────────┐
│ ESP32                      │
│ Procesamiento Local        │
└────────┬───────────────────┘
         │ UART
         ▼
┌────────────────────────────┐
│ Módulo LTE A7608SA-H       │
└────────┬───────────────────┘
         │
         ▼
      Internet
         │
         ▼
┌────────────────────────────┐
│ AWS EC2                    │
│ Backend Node.js            │
└────────┬───────────────────┘
         │
         ▼
┌────────────────────────────┐
│ SQLite                     │
│ Almacenamiento Histórico   │
└────────┬───────────────────┘
         │
         ▼
┌────────────────────────────┐
│ Dashboard Streamlit        │
└────────────────────────────┘
```

---

## Tecnologías Utilizadas

### Hardware

* ESP32
* Sensor de corriente SCT-013
* Sensor de temperatura infrarrojo
* Módulo LTE A7608SA-H

### Software

* Arduino IDE
* Python
* Streamlit
* Node.js
* Express.js
* SQLite
* AWS EC2
* Ubuntu Server 24.04 LTS

---

## Estructura del Repositorio

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
│
├── images/
│
├── LICENSE
├── .gitignore
└── README.md
```

---

## Funcionamiento

1. El ESP32 adquiere las señales provenientes de los sensores.
2. Se calcula el valor RMS de la corriente consumida por el motor.
3. Se registran las temperaturas medidas.
4. Los datos son encapsulados en formato JSON.
5. El módulo LTE transmite la información al servidor AWS mediante HTTP.
6. El backend Node.js recibe y almacena la información en SQLite.
7. Streamlit consulta la base de datos y presenta los datos en tiempo real.
8. El usuario puede visualizar tendencias históricas y el estado actual del sistema.

---

## Backend

El servidor implementado en Node.js recibe las solicitudes HTTP provenientes del dispositivo IoT.

Funciones principales:

* Recepción de datos.
* Validación de información.
* Almacenamiento en SQLite.
* Consulta de última lectura.
* Consulta de histórico de datos.

---

## Dashboard

El dashboard desarrollado en Streamlit permite:

* Visualización de temperatura ambiente.
* Visualización de temperatura del motor.
* Visualización de corriente RMS.
* Consulta histórica de mediciones.
* Detección de pérdida de comunicación.
* Monitoreo remoto en tiempo real.

---

## Base de Datos

Se utiliza SQLite para almacenar:

* Temperatura ambiente.
* Temperatura del motor.
* Corriente RMS.
* Marca temporal de cada medición.

---

## Despliegue en la Nube

La infraestructura se encuentra desplegada en Amazon Web Services (AWS) utilizando una instancia EC2 con Ubuntu Server.

Servicios implementados:

* Backend Node.js
* Base de datos SQLite
* Dashboard Streamlit

---

## Capturas del Proyecto

### Dashboard

Agregar imagen en:

```text
images/dashboard.png
```

### Montaje Experimental

Agregar imagen en:

```text
images/montaje.jpg
```

### Hardware Implementado

Agregar imágenes de:

* ESP32
* Módulo LTE
* Sensores
* Sistema final instalado

---

## Resultados

El sistema permite monitorear remotamente variables críticas del motor eléctrico sin necesidad de modificar la instalación eléctrica existente, proporcionando una solución escalable para aplicaciones de mantenimiento predictivo e Industria 4.0.

---

## Autores

* Taufic Yusef Rapag Padilla
* Alejandra Tuiran Ospino
* Sebastian Pupo Solano
* Alejandro Rovira Brieva

Proyecto académico de Ingeniería Electrónica.
