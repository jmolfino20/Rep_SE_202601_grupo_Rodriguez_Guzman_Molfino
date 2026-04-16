import socket
import numpy as np
from PIL import Image
import matplotlib.pyplot as plt
import os

os.makedirs("fotos", exist_ok=True)

s = socket.socket()
s.bind(("0.0.0.0", 1234))
s.listen(1)

conn, _ = s.accept()

contador = 0

while True:
    size_bytes = conn.recv(4)
    size = int.from_bytes(size_bytes, 'little')

    data = b''
    while len(data) < size:
        data += conn.recv(size - len(data))

    img = np.frombuffer(data, dtype=np.uint8).reshape((96, 96))

    # Convertir a imagen PIL (modo L = grayscale)
    imagen = Image.fromarray(img, mode='L')

    # Guardar PNG limpio (96x96 exacto)
    filename = f"fotos/img_{contador:04d}.png"
    imagen.save(filename)

    print("Guardada:", filename)

    contador += 1

    plt.imshow(img, cmap='gray')
    plt.pause(0.001)