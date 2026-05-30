# Protocolo de Comunicación

## Introducción

La comunicación entre el nodo IoT y el servidor se realiza mediante HTTP sobre redes LTE.

---

# Red Celular

Operador utilizado:

Movistar Colombia

APN configurado:

internet.movistar.com.co

---

# Formato de Mensaje

Los datos se encapsulan utilizando JSON.

Ejemplo:

```json
{
  "temp_ambiente": 26.8,
  "temp_objeto": 61.5,
  "corriente": 3.12
}
```

---

# Método HTTP

POST

Endpoint:

```text
/api/data
```

---

# Intervalo de Envío

5 segundos.

---

# Flujo de Comunicación

ESP32
↓
JSON
↓
HTTP POST
↓
LTE
↓
Internet
↓
AWS EC2
↓
Node.js
↓
SQLite

---

# Ventajas

- Compatibilidad universal.
- Bajo consumo de ancho de banda.
- Fácil integración con plataformas cloud.
