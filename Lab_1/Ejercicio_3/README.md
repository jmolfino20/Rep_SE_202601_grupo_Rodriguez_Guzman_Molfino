# Ejercicio 3

[Ejercicio 2](../Ejercicio_2/README.md) - [Volver](../README.md) - [Ejercicio 4](../Ejercicio_4/README.md)

## Introducción

Este ejercicio busca realizar un análisis de la medida CPI al aumentar la cantidad de instrucciones en el programa además de un *profiling* de este mismo. También tiene el objetivo de entender el impacto de la frecuencia de operación del MCU elegida en diversas medidas como tiempo, energía, CPI, potencia y ciclos de reloj.

## Ejecución

Para obtener los datos utilizados para las diversas gráficas se utilizaron dos programas. Para obtener los datos del punto 3.1, se utilizó el código en `implementacion_pseudo_code/main/pseudo_code.c`. Para el 3.2 se usó aquel en la ruta `codigo_profiling_ej3/main/profiling_ej3.c`. Por último para el punto 3.3 se utilizaron los datos recopilados en ambos y mediciones de energía y potencia realizadas presencialmente en el laboratorio.

Para correr ambos programa es importante hacerlo en un ESP32-S3 ya que esa fue la placa elegida con el comando `idf.py set-target`. En caso de tener otra placa correr los siguientes comandos:

```bash
idf.py fullclean   # Para quitar todo lo relacionado a algún build anterior
idf.py set-target ${ESP_EN_USO}   # esp32, esp32s3, esp32c3, etc.
```

(En caso de tener el ESP32-S3 no es necesario lo anterior ya que la configuración se encuentra en el archivo `sdkconfig`)

Luego de seleccionar el MCU que se va a usar, hay que compilar y flashear en cada carpeta correspondiente, donde se tenga acceso a la carpeta `/main`. Para eso usar los comandos:

```bash
idf.py build
idf.py flash ${RUTA-AL-COM}
```

Una vez flasheado, podemos monitorear el MCU y verificar su funcionamiento. Para eso usar el comando:

```bash
idf.py monitor ${RUTA-AL-COM}
```

Esto entregará los valores usados para los gráficos y análisis. El valor de X debe modificarse manualmente y volver a repetir el proceso anterior.