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

(Falta rellenar con texto)

![Video de prueba](./ej_5_test.mp4)

## Análisis

(Falta probar cambiar resolución y esas cosas pa poner un análisis)
