# Extra 1: Modelos IA para detección de identificador

Este directorio incluye el/los modelos usados para la detección de identificador.

## Modelo

El modelo seguirá la siguiente manera de construirse:

1. Entrenamiento de modelo Teacher (Modelo grande)

2. Entrenamiento de modelo Student (Modelo pequeño):

    - Usar Knowledge Distilation

    - Usar Quantization Aware Training

    - Usar Curriculum Learning

    - Intentar usar Structured Pruning para eliminar algunos canales/filtros completamente según sea posible por desempeño

3. Cuantización de modelo pequeño

## Dataset

El dataset tendrá la siguiente estructura:

```bash
- dataset/
    - easy/   # Imágenes con el identificador sólo y fácil de identificar
        - 001.png
        - 002.png
        - ...
        - tags.txt
    - medium/   # Imágenes con algo de ruido extra pero relativamente simples
        - 001.png
        - 002.png
        - ...
        - tags.png
    - hard/   # Imágenes del entorno real que se espera
        - 001.png
        - 002.png
        - ...
        - tags.png
    - other/   # Imágenes del entorno real pero tomadas por otro grupo para verificar generalización
        - 001.png
        - 002.png
        - ...
        - tags.png
```

La separación será:

- Train: 90% del total de `easy/`, 90% del total de `medium/`, 70% del total de `hard/` y 0% de other.

- Validation: 10% `easy/`, 10% `medium`, 15% `hard/` y 0% de `other/`.

- Test: 0% `easy/`, 0% `medium`, 15% `hard/` y 100% de `other/`.

## Hardware

El modelo final se ejecutará directamente en el ESPCAM-AITHINKER.
