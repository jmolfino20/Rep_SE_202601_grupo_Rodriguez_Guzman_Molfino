# Ejercicio 1

[Volver](../README.md) - [Ejercicio 2](../Ejercicio_2/README.md)

## Introducción

Este ejercicio es para ver el consumo de TinyML en los _end devices_, para lo que se usará el ejemplo de **Tensorflow Lite Micro** que infiere la función seno.

## Consideraciones

Aunque se espera usar el ML en el **ESP32-CAM**, debido a limitaciones técnicas por las herramientas en nuestra disposición, se medirá el consumo en el **ESP-S3**, por lo que se espera que el real sea algo menor.

## Resultados

### Ejemplo Hello World

![Resultados hello_world](./results_hello_world.jpg)

$V = 5.1287 V \pm 0.0001 V$

$I = 0.05552 A \pm 0.00002 A$

$P = 0.2842 W \pm 0.0001 W$

### Implementación directa en C
