# Ejercicio 2

[Ejercicio 1](../Ejercicio_1/README.md) - [Volver](../README.md) - [Ejercicio 3](../Ejercicio_3/README.md)

## Introducción

Este ejercicio tuvo el objetivo de utilizar el código de ejemplo "person detection", probarlo con las imágenes originales y ajustarlo para, primero analizar imágenes estáticas de los integrantes del grupo y luego para realizar inferencias en tiempo real.

Al probar con las imágenes de ejemplo, los porcentajes de clasificación de persona calzaban con las descripciones de las imágenes, donde un perro obtenía un porcentaje bajo de detección de persona (pero no 0), un mono alrededor de 80% y un humano un 95% en la mayoría de sus casos.

Se obtuvo una imagen de cada integrante del grupo con el código realizado para el laboratorio 1, se pasó a formato .txt y se pasó a la detección estática, obteniendo 95% en dos casos y 52% para otro. Este último resultado es atribuible a la luz de la imagen que causó confusión al modelo al momento de realizar la inferencia.

Luego se pasó al modo de inferencia en tiempo real, modificando en `esp_main.h` la línea: 

```cpp
#define CLI_ONLY_INFERENCE // 1 o 0
```

de 1 a 0. 

Con esto se modificó el código para que cada vez que detectara una persona o algo con certeza sobre 60%, prende el LED.

De esta manera se logró el comportamiento presentado en el link del video de output.pdf.

## Resultados

La **ESPCAM** fue capaz de detectar personas en tiempo real, probamos poniendonos en frente al MCU y detectó correctamente como se puede ver a continuación.

![Rodrigo Guzmán](./person_rodri.gif)
![Joaquín Molfino](./person_molfino.gif)
![Raimundo Rodríguez](./person_rai.gif)
