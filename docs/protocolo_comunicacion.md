# Protocolo de Comunicación

## Flujo General

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
↓
Dashboard Streamlit

## Formato de Datos

```json
{
  "temp_ambiente": 25.4,
  "temp_objeto": 61.2,
  "corriente": 3.15
}
```

## Intervalo de Transmisión

5 segundos
