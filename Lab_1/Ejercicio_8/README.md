# Ejercicio 1

## Introducción

...

## Ejecución

Para correr el programa es importante hacerlo en un ESP32 ya que esa fue la placa elegida con el comando `idf.py set-target` y es la placa del ESP-CAM usado en el laboratorio. Luego hay que compilar y flashear. Para eso usar los comandos:

```bash
idf.py build
idf.py flash ${RUTA-AL-COM}
```

Una vez flasheado, podemos monitorear el MCU y verificar su funcionamiento. Para eso usar el comando:

```bash
idf.py monitor ${RUTA-AL-COM}
```