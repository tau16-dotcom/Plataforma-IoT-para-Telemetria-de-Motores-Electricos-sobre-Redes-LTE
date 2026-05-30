# Diagnóstico LTE

## Objetivo

Verificar el estado de la comunicación antes de iniciar la transmisión de datos.

---

# RSSI

Comando:

AT+CSQ

Conversión utilizada:

RSSI(dBm) = -113 + 2 × CSQ

### Clasificación

| RSSI | Calidad |
|--------|--------|
| > -70 dBm | Excelente |
| -70 a -85 dBm | Buena |
| -85 a -100 dBm | Regular |
| < -100 dBm | Mala |

---

# BER

Bit Error Rate reportado por la red celular.

Comando:

AT+CSQ

---

# Operador

Comando:

AT+COPS?

Ejemplo:

Movistar Colombia

---

# Tecnología

El sistema identifica:

- GSM (2G)
- UTRAN (3G)
- LTE (4G)
- NR (5G)

---

# Dirección IP

Comando:

AT+CGPADDR

Permite verificar que el contexto PDP fue activado correctamente.

---

# Identificación del Módulo

## IMEI

AT+CGSN

## ICCID

AT+ICCID

---

# Latencia

Se realiza una solicitud HTTP GET hacia el servidor.

La diferencia entre el tiempo de envío y recepción permite estimar el RTT de la comunicación.

---

# Beneficios del Diagnóstico

- Verificación de cobertura.
- Detección de fallas.
- Validación de acceso a Internet.
- Confirmación de conexión con el servidor.
