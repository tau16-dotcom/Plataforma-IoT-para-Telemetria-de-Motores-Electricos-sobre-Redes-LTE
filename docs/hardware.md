# Hardware Implementado

## ESP32

Microcontrolador principal encargado de la adquisición, procesamiento y transmisión de datos.

### Funciones

- Lectura de sensores.
- Cálculo RMS.
- Gestión de comunicaciones.
- Generación de mensajes JSON.
- Control del módulo LTE.

---

# Sensor de Corriente SCT-013

El SCT-013 es un transformador de corriente (CT) no invasivo utilizado para medir el consumo eléctrico del motor.

### Ventajas

- No requiere modificar el circuito de potencia.
- Instalación sencilla.
- Aislamiento galvánico.

### Variable Medida

Corriente RMS (A).

---

# Sensor de Temperatura MLX90614

Sensor infrarrojo de temperatura sin contacto.

### Comunicación

I2C.

### Variables

- Temperatura ambiente.
- Temperatura superficial del motor.

### Ventajas

- No requiere contacto físico.
- Alta precisión.
- Adecuado para superficies calientes.

---

# Módulo LTE A7608SA-H

Módulo de comunicación celular utilizado para transmitir los datos hacia AWS.

### Funciones

- Registro en red móvil.
- Gestión APN.
- Obtención de IP.
- Comunicación HTTP.
- Diagnóstico de red.

### Interfaz

UART.

### Velocidad

115200 bps.

---

# Alimentación

Todos los dispositivos son alimentados mediante una fuente de corriente continua adecuada para las condiciones de operación del sistema.
