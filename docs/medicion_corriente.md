# Medición de Corriente RMS

## Introducción

La corriente consumida por el motor es adquirida mediante un SCT-013.

Debido a que la señal es alterna, se utiliza el valor RMS como indicador representativo.

---

# Ecuación RMS

Irms = √[(1/N) Σ(Ii²)]

Donde:

- Ii representa cada muestra.
- N representa el número total de muestras.

---

# Implementación

El firmware utiliza:

```cpp
#define MUESTRAS 1000
```

Se adquieren 1000 muestras del ADC.

Posteriormente se calcula:

1. Offset DC.
2. Valor cuadrático medio.
3. Raíz cuadrada.

---

# Calibración

```cpp
#define FACTOR_CAL 30.0
```

Permite ajustar la conversión a amperios reales.

---

# Umbral de Ruido

```cpp
#define UMBRAL_A 0.3
```

Corrientes inferiores a este valor son consideradas cero.

---

# Aplicaciones

- Monitoreo de carga.
- Detección de sobrecorriente.
- Detección de fallas.
- Mantenimiento predictivo.
