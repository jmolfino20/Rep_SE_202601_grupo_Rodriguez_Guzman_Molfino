# Ejercicio 2

[Ejercicio 1](../Ejercicio_1/README.md) - [Volver](../README.md) - [Ejercicio 3](../Ejercicio_3/README.md)

## Introducción

Este ejercicio busca empezar a probar el uso de la ESP32-S3 y entender sus limitaciones. Es por esta razón que la imagen se debe redimensionar y dejar en escala monocromática. 

Además es un acercamiento inicial al uso de este MCU y al procesamiento de imágenes.

## Ejecución

Para obtener la misma imagen que obtuvo el grupo, se deben seguir diversos pasos ya que el único paso ejecutado en la ESP32-S3 fue el del operador Sobel.

Inicialmente se debe acceder al link del colab utilizado para redimensionarla a 96x96 y hacerla monocromática. Para esto se deba acceder a la sección de archivos del colab y directamente subir la imagen sin una ruta específica, la cual (por las dudas) es `/content/pikachu.jpg`.

Una vez subida la imagen se debe ejecutar la primera celda de código que transformará la imagen a lo solicitado en el ejercicio y generará el archivo "pikachu.h" que será el utilizado para aplicarle el operador Sobel mediante la ESP32-S3, archivo que ya está incluido en la carpeta `esp-code/main` dentro de `Ejercicio_2`. Si se quisiera crear de nuevo este archivo, se debería agregar en la carpeta `esp-code/main` dentro de `Ejercicio_2` y luego proceder a los pasos de ejecución.

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

Una vez realizado esto, el programa imprimirá en la terminal 96 listas con 96 valores en cada una, las cuales deben ser copiadas y pegadas dentro de los corchetes iniciales de la variable `raw_data` para así poder procesar de correcta manera la imagen. Se recomienda que, antes de correr el comando `idf.py monitor ${RUTA-AL-COM}`, aplicar `Ctrl + -` para poder imprimir cada lista en una sola línea y de esta manera evitar errores.

Una vez obtenida la impresión en la serial y habiéndola pegado dentro de los corchetes iniciales de `raw_data`, ejecutar la segunda celda del colab y se obtendrá la imagen de salida.