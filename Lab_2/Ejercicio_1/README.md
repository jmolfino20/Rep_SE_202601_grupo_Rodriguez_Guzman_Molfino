# Ejercicio 1

[Volver](../README.md) - [Ejercicio 2](../Ejercicio_2/README.md)

## Introducción

Este ejercicio sirve para probar la comunicación inalámbrica usando ESP32. Al no estar seguros por cuál sería la mejor desición de protocolo inalámbrico, se decidió realizar distintos métodos.

## Métodos probados

### Mediante BLE

**[Probar método](./BLE-version/README.md)**

Este método se conecta mediante _Bluetooth Low Energy_ (BLE) para la comunicación inalámbrica. Se cree que los _pros_ pueden ser el bajo consumo energético y la ligereza en CPU y RAM, mientras que por otro lado, se espera ver los _contras_ que serían una posible inestabilidad en caso de haber muchos dispositivos usando BLE por un colapso en el canal.

Se dejaron las 2 versiones en el repositorio en caso de que por colapso de algun canal (Bluetooth o WiFi) el día de la carrera, se cambie en ese momento y evitar problemas.

### Mediante ESP32 modo AP

**[Probar método](./AP-version/)**
