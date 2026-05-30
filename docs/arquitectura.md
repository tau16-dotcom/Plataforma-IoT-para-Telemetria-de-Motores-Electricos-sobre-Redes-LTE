# Arquitectura del Sistema

## Introducción

El sistema implementa una arquitectura IoT distribuida orientada al monitoreo remoto de motores eléctricos mediante redes celulares LTE.

La solución fue diseñada para operar en ubicaciones donde no existe infraestructura de red local disponible, permitiendo transmitir información operacional hacia una plataforma cloud para su posterior análisis y visualización.

---

# Arquitectura General

El sistema está compuesto por cinco capas principales:

1. Adquisición de datos.
2. Procesamiento local.
3. Comunicación LTE.
4. Infraestructura Cloud.
5. Visualización.

```text
┌─────────────────────────────┐
│       Motor Eléctrico       │
└──────────────┬──────────────┘
               │
               ▼
┌─────────────────────────────┐
│         Sensores            │
│                             │
│ SCT-013                     │
│ MLX90614                    │
└──────────────┬──────────────┘
               │
               ▼
┌─────────────────────────────┐
│            ESP32            │
│                             │
│ Procesamiento Local         │
│ Cálculo RMS                 │
│ Formato JSON                │
└──────────────┬──────────────┘
               │ UART
               ▼
┌─────────────────────────────┐
│        A7608SA-H LTE        │
└──────────────┬──────────────┘
               │
               ▼
          Red Celular
               │
               ▼
┌─────────────────────────────┐
│         AWS EC2             │
└──────────────┬──────────────┘
               │
               ▼
┌─────────────────────────────┐
│        Node.js API          │
└──────────────┬──────────────┘
               │
               ▼
┌─────────────────────────────┐
│          SQLite             │
└──────────────┬──────────────┘
               │
               ▼
┌─────────────────────────────┐
│      Dashboard Web          │
│        Streamlit            │
└─────────────────────────────┘
```

---

# Flujo de Operación

1. El SCT-013 adquiere la corriente consumida por el motor.
2. El MLX90614 mide la temperatura ambiente y superficial.
3. El ESP32 procesa las muestras obtenidas.
4. Se calcula la corriente RMS.
5. Se genera un objeto JSON.
6. El módulo LTE transmite la información.
7. AWS recibe los datos.
8. Node.js procesa la solicitud.
9. SQLite almacena la información.
10. Streamlit consulta y visualiza los datos.
