import numpy as np
import matplotlib.pyplot as plt
# Параметры OFDM
N = 64  # Количество поднесущих
cp_len = 16  # Длина циклического префикса
num_symbols = 4  # Количество OFDM символов
# Генерация случайных бит
data_bits = np.random.randint(0, 2, N * num_symbols)
# Преобразование бит в символы (например, QPSK)
data_symbols = (2 * data_bits[0::2] - 1) + 1j * (2 * data_bits[1::2] - 1)
# OFDM модуляция
ofdm_symbols = np.fft.ifft(data_symbols, N)
# Добавление циклического префикса
ofdm_with_cp = np.concatenate((ofdm_symbols[-cp_len:], ofdm_symbols))
# Модель канала (например, добавление гауссовского шума)
snr = 20  # Уровень шума в дБ
noise_power = 1 / (10 ** (snr / 10))
noise = np.sqrt(noise_power / 2) * (np.random.randn(len(ofdm_with_cp)) + 1j * np.random.randn(len(ofdm_with_cp)))
received_signal = ofdm_with_cp# + noise
# Удаление циклического префикса
received_signal_no_cp = received_signal[cp_len:]
# OFDM демодуляция
demodulated_symbols = np.fft.fft(received_signal_no_cp, N)
# Преобразование символов обратно в биты
demodulated_bits = np.zeros(N * num_symbols, dtype=int)
# Преобразуем символы обратно в биты
# demodulated_bits[0::2] = (np.real(demodulated_symbols) > 0).astype(int)
# demodulated_bits[1::2] = (np.imag(demodulated_symbols) > 0).astype(int)
# Вывод ресурсной сетки
plt.figure(figsize=(10, 6))
plt.imshow(np.abs(demodulated_symbols).reshape(1, N), aspect='auto', cmap='hot')  # Отображаем как одномерный массив
plt.title('Ресурсная сетка')
plt.xlabel('Поднесущие')
plt.ylabel('OFDM символы')
plt.colorbar(label='Амплитуда')
plt.show()
# Сравнение отправленных и полученных бит
bit_errors = np.sum(data_bits != demodulated_bits)
print(f'Количество ошибок: {bit_errors}')
