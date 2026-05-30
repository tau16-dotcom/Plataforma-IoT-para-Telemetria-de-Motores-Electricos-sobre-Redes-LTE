# Infraestructura Cloud

## Amazon Web Services

El sistema utiliza AWS como plataforma cloud para almacenamiento y visualización remota.

---

# EC2

## Sistema Operativo

Ubuntu Server 24.04 LTS

---

# Servicios Ejecutados

## Node.js

Backend encargado de:

- Recibir solicitudes HTTP.
- Validar datos.
- Almacenar información.
- Responder consultas.

Puerto:

3000

---

## SQLite

Base de datos local utilizada para almacenar:

- Temperatura ambiente.
- Temperatura superficial.
- Corriente RMS.
- Fecha y hora.

---

## Streamlit

Dashboard web utilizado para la visualización en tiempo real.

Puerto:

8501

---

# Flujo de Datos

ESP32
↓
LTE
↓
AWS EC2
↓
Node.js
↓
SQLite
↓
Streamlit

---

# Ventajas de AWS

- Alta disponibilidad.
- Escalabilidad.
- Acceso remoto.
- Facilidad de administración.
