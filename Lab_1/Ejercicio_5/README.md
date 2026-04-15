# Ejercicio 5

[Ejercicio 4](../Ejercicio_4/README.md) - [Volver](../README.md) - [Ejercicio 6](../Ejercicio_6/README.md)

## Introducción

Este ejercicio busca poner a prueba lo que se puede hacer con sensores y actuadores mediante controlar motores según lo que se capture por un micrófono. Además sirve para entender cómo se puede aplicar las **Transformadas de Fourier** para encontrar la frecuencia dominante, y cómo afectan los parámetros a la medición.

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

## Configuración

Para este ejemplo se usó la siguiente tabla de _Pinout_:

**Puente H:**

- IN1: ESP32-S3 GPIO5
- IN2: ESP32-S3 GPIO6
- IN3: ESP32-S3 GPIO8
- IN4: ESP32-S3 GPIO9
- ENA: ESP32-S3 GPIO4
- ENB: ESP32-S3 GPIO7
- 12V: Salida positiva de las baterías
- GND: GND común

**Micrófono analógico:**

- Ponerlos aquí ~~(No me acuerdo de los nombres estos xd)~~
- La salida iba a GPIO1
- Los demás era gain a gusto
- GND a tierra comun
- había uno que se llamaba como COM que iba a tierra
- y si estaba Vdd (creo que si) va a +3.3 de ESP32-S3

## Funcionamiento

Para este ejercicio se utilizaron los diversos archivos en la carpeta `main`, donde el flujo principal se encuentra en el archivo `Ejercicio_5.c`.

Una vez hecho el `build` y el `flash` a la ESP32S3, se puede ejecutar el comando `monitor` (todos previamente mencionados) para poder interactuar con los motores y obtener los valores de medición usados para realizar los gráficos. Se utilizó un generador de tonos *online* disponible en el siguiente [Link](https://www.szynalski.com/tone-generator/). Si se acerca el teléfono al micrófono en las diversas frecuencias definidas en el código, los motores se comportarán de las diversas maneras solicitadas.

Es importante mencionar que para obtener los diversos gráficos se realizón un proceso más manual, donde al momento de ejecutar el programa, se acercaba al teléfono con la frecuencia deseada al micrófono y se veían lo valores tomados en terminal. Además, modificando el código de `config.h`, se pudo modificar la frecuencia de muestreo, el valor de N para las muestras de la FFT y el valor de bits del ADC, donde para esto último también se modificó el archivo `adc_audio.c`.

Los valores obtenidos en serial al principio del código, se pegaron en el archivo `fft.txt` y también se obtuvo el valor real de la frecuencia de muestreo mediante el código del archivo `Ejercicio_5.c`, para luego tomar estos datos y reemplazarlos en el archivo `espectrograma.py`, modificando además la variable N, dejándola con el valor establecido en config.h.

Con todas estas modificaciones se pudo obtener las mediciones y gráficos disponibles en el documento `output.pdf`.

![Video de prueba](./ej_5_test.mp4)


