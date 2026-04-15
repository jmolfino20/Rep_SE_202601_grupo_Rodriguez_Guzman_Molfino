# Ejercicio 1

[Volver](../README.md) - [Ejercicio 2](../Ejercicio_2/README.md)

## Introducción

Este ejercicio sirve para probar la comunicación inalámbrica usando ESP32. Al no estar seguros por cuál sería la mejor desición de protocolo inalámbrico, se decidió realizar distintos métodos.

## Métodos probados

### Mediante BLE

**[Probar método](./BLE-version/README.md)**

Este método se conecta mediante _Bluetooth Low Energy_ (BLE) para la comunicación inalámbrica. Se cree que los _pros_ pueden ser el bajo consumo energético y la ligereza en CPU y RAM, mientras que por otro lado, se espera ver los _contras_ que serían una posible inestabilidad en caso de haber muchos dispositivos usando BLE por un colapso en el canal.

### Mediante ESP32 modo AP

Probar método

### Mediante conectar ESP32 y PC por Websocket usando WiFi común

probar método

## Análisis

El método 1 usaba...

El método 2 usaba...

El método 3 usaba...

Al ver el consumo de memoria y CPU de cada uno de los métodos, se decidió optar por ${METODO-GANADOR} dado a que es crucial optimizar al máximo los recursos si se quiere ejecutar TinyML para encontrar los identificadores o bordes.