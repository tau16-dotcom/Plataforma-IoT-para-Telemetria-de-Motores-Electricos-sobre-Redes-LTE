# Diagnóstico LTE

El firmware incorpora un sistema de diagnóstico que evalúa continuamente la calidad de la comunicación.

## Parámetros

### RSSI

Comando:

AT+CSQ

Conversión:

RSSI(dBm) = -113 + 2 × CSQ

### BER

Bit Error Rate.

### Operador

AT+COPS?

### Tecnología

- GSM
- UTRAN
- LTE
- NR

### IP Asignada

AT+CGPADDR

### ICCID

AT+ICCID

### IMEI

AT+CGSN

### Latencia

HTTP GET al servidor para estimar RTT.
