# Extra 1: Capturar fotos para IA

Este apartado incluye un [cliente](./client/) (ESP-CAM) y un [servidor](./server/) (PC con Python).

## Ejecución

### Cliente

```bash
idf.py build
idf.py flash ${RUTA-A-COM}
idf.py monitor ${RUTA-A-COM}   # Opcional 
```

### Servidor

Se recomienda no usar WSL para exponer más fácil el puerto

**Windows**

```bash
python3 -m venv venv
venv\Scripts\activate
pip install numpy matplotlib pillow
python3 main.py
```

**Linux** (Y me imagino que también Mac)

```bash
python3 -m venv venv
source venv/bin/activate
pip install numpy matplotlib pillow
python3 main.py
```