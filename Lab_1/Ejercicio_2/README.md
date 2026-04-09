# Ejercicio 2

## Introducción

...

## Ejecución

Para correr el programa es importante hacerlo en un ESP32-S3 ya que esa fue la placa elegida con el comando `idf.py set-target`. En caso de tener otra placa correr los siguientes comandos:

```bash
idf.py fullclean   # Para quitar todo lo relacionado a algún build anterior
idf.py set-target ${ESP_EN_USO}   # esp32, esp32s3, esp32c3, etc.
```

(En caso de tener el ESP32-S3 no es necesario lo anterior ya que la configuración se encuentra en el archivo `sdkconfig`)

Luego de seleccionar el MCU que se va a usar, hay que compilar y flashear. Para eso usar los comandos:

```bash
idf.py build
idf.py flash ${RUTA-AL-COM}
```

Una vez flasheado, podemos monitorear el MCU y verificar su funcionamiento. Para eso usar el comando:

```bash
idf.py monitor ${RUTA-AL-COM}
```