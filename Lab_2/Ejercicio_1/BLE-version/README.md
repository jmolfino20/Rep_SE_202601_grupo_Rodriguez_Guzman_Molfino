# Conexión por BLE

[Volver](../README.md)

Este método conecta el "control remoto" al ESP32 mediante BLE. Envía señales sin esperar ACK para menor latencia.

## Cómo probar

### ESP

1. Compilar desde [`esp-ble`](./esp-ble/) haciendo `idf.py build`.

2. Flashear código a ESP32 haciendo `idf.py flash ${RUTA-A-COM}`

3. Monitorear usando `idf.py monitor ${RUTA-A-COM}`

```bash
idf.py build
idf.py flash ${RUTA-A-COM}
idf.py monitor ${RUTA-A-COM}
```

### Control Remoto

1. Ir al directorio [`web-ble`](./web-ble/)

2. Ejecutar archivo en servidor web haciendo `python3 -m http.server 8000` (u otro comando equivalente)

3. Abrir `http://localhost:8000` desde un navegador que permita comunicación serial (**Firefox** no lo soporta, usar basados en **Chromium**)

```
python3 -m http.server 8000
```