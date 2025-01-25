import numpy as np
import matplotlib.pyplot as plt

# Параметры
fs = 1000000  # Частота дискретизации
T = 10      # Время в секундах
N = fs * T # Количество точек
t = np.linspace(0, T, N, endpoint=False)

# Генерация белого гауссовского шума
noise = np.random.normal(0, 1, N)

# Визуализация
plt.plot(t, noise)
plt.title('Белый гауссовский шум')
plt.xlabel('Время (с)')
plt.ylabel('Амплитуда')
plt.show()
