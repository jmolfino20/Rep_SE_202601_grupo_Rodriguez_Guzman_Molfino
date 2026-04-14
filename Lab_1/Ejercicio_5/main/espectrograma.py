import numpy as np
import matplotlib.pyplot as plt

fs = 1805.41 # usa tu real_fs si quieres ser más preciso
N = 512   # FFT_SIZE

data = []

with open("fft.txt") as f:
    for line in f:
        if "END" in line:
            break
        data.append(float(line.strip()))

data = np.array(data)

freqs = np.linspace(0, fs/2, len(data))

plt.plot(freqs, data)
plt.xlabel("Frecuencia (Hz)")
plt.ylabel("Magnitud")
plt.title("Espectro de la señal")
plt.grid()
plt.savefig("espectrograma.png")