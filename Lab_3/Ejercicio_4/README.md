# Ejercicio 4

[Ejercicio 3](../Ejercicio_3/README.md) - [Volver](../README.md)

## Introducción

Este ejercicio tuvo el objetivo de integrar la comunicación entre la cámara ESP32 CAM con el MCU ESP32 S3. La comunicación utilizada fue mediante UART con los pines respectivos de cada placa.

Para el código de la ESP32 S3 se utilizó aquel creado para el Laboratorio 2 pero con modificaciones extra. Se agregó la comunicación UART y los comandos que envía la CAM, se cambiaron los pines utilizados por comodidad al soldar la placa y se modificaron las velocidades para asegurar que el robot se mantenga en el ring y no salga del área permitida.

Para el código de la CAM se implementó un filtro sobel para detección de bordes y se dividió el input de 96x96 de la cámara en una matriz 3x3. Para cada zona de la matriz se calcula un promedio de cuántos pixeles se consideran parte del borde y en base a estos cálculos, se define el siguiente movimiento del robot. Si no detecta borde, tiene la instrucción de seguir derecho. Está pensado en solo girar a la derecha ya que se solicita que recorra el ring en sentido horario.