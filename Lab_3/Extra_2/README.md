# Extra 2: Preparación de dataset

Script para transformar imágenes de smartphone y tagearlas para el dataset de detección de identificador.

## Uso

```bash
python prepare_dataset.py <directorio_de_imágenes>
```

## Qué hace

1. **Transforma** cada imagen para simular la salida de la cámara OV2640 del ESP32-CAM:
   - Convierte a escala de grises
   - Recorta al cuadrado (toma el lado más corto, corta el más largo simétricamente)
   - Redimensiona a 128×128
   - Reduce contraste, aplica blur leve, artefactos JPEG y ruido de sensor

2. **Muestra** cada imagen transformada con dos líneas verticales que dividen la imagen en tercios

3. **Tagging** con teclado:
   - `0` — Identificador no está en la imagen
   - `1` — Está en el tercio izquierdo
   - `2` — Está en el tercio central
   - `3` — Está en el tercio derecho
   - `q` — Saltar imagen (no se guarda)
   - `Esc` — Guardar lo que hay y salir

## Output

Crea un directorio `new/` dentro del directorio de entrada:

```
<input_dir>/new/
    000.png
    001.png
    ...
    NNN.png
    tags.txt
```

`tags.txt` tiene una línea por imagen: `NNN.png,<tag>`

Luego mueve `new/` manualmente al subdirectorio correspondiente del dataset (`easy/`, `medium/`, `hard/`, `other/`).

## Dependencias

```bash
pip install opencv-python pillow numpy
```
