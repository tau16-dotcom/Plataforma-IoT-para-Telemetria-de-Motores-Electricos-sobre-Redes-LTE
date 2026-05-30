# Medición de Corriente RMS

La corriente del motor es adquirida mediante un sensor SCT-013 conectado al ESP32.

Para obtener una medida representativa de la corriente alterna consumida por el motor se calcula el valor RMS (Root Mean Square).

## Ecuación

Irms = √[(1/N) Σ(Ii²)]

Donde:

- Ii corresponde a cada muestra adquirida.
- N es el número total de muestras.

## Ventajas

- Permite estimar la carga del motor.
- Detecta variaciones anormales de consumo.
- Facilita estrategias de mantenimiento predictivo.
