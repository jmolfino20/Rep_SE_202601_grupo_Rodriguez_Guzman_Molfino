# Ejercicio 6

[Ejercicio 5](../Ejercicio_5/README.md) - [Volver](../README.md) - [Ejercicio 7](../Ejercicio_7/README.md)

## Introducción

En este ejercicio se utiliza la cámara del módulo ESP32-CAM junto con el entorno Arduino IDE para capturar una imagen. Se emplea un ejemplo predefinido del framework de Arduino que permite verificar rápidamente el funcionamiento correcto de la cámara.

El objetivo es obtener una fotografía del grupo y visualizarla a través de una interfaz web generada por el ESP32.

## Configuración

1. Abrir Arduino IDE.
2. Ir a Archivo > Ejemplos > ESP32 > Camera > CameraWebServer.
3. Seleccionar la placa correcta:
   Board: AI Thinker ESP32-CAM
4. Configurar las credenciales WiFi en el código:

```cpp
const char* ssid = "WIFI";
const char* password = "PASSWORD";
```

## Ejecución

1. Conectar el ESP32-CAM al computador.
2. Poner el módulo en modo programación.
3. Subir el código desde Arduino IDE en UPLOAD.
4. Abrir el monitor serial (115200).
5. Reiniciar el módulo (RESET).
6. Copiar la dirección IP que aparece en pantalla y abrirla en el navegador.
7. En el navegador presionar Start Stream o Capture.

## Resultado esperado

Se debe visualizar una imagen capturada por la cámara del ESP32-CAM desde el navegador.
