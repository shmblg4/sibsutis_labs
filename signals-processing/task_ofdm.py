import numpy as np
import matplotlib.pyplot as plt

# Параметры OFDM
N = 64  # число поднесущих
Np = 8  # количество пилотных поднесущих
snr_db = 20  # Signal-to-noise ratio в dB

# Определяем позиции пилотов равномерно по спектру
pilot_idx = np.arange(0, N, N // Np)
data_idx = np.setdiff1d(np.arange(N), pilot_idx)

# Генерация данных
X = np.zeros(N, dtype=complex)
X[pilot_idx] = 2 * (np.random.randint(0, 2, Np) - 0.5)  # BPSK пилоты
X[data_idx] = np.exp(1j * 2 * np.pi * np.random.rand(N - Np))  # случайная фаза (пример)

# Имитация канала и шума
np.random.seed(1)
H_true = np.exp(1j * 2 * np.pi * np.random.rand(N))  # Фейдинг канал
noise_power = 10 ** (-snr_db / 10)
noise = np.sqrt(noise_power / 2) * (np.random.randn(N) + 1j * np.random.randn(N))
Y = H_true * X + noise

# Оценка канала на пилотах
H_pilots_est = Y[pilot_idx] / X[pilot_idx]

# Интерполяция (реальная и мнимая части отдельно)
H_est_real = np.interp(np.arange(N), pilot_idx, H_pilots_est.real)
H_est_imag = np.interp(np.arange(N), pilot_idx, H_pilots_est.imag)
H_est = H_est_real + 1j * H_est_imag

# Визуализация
plt.figure(figsize=(12, 5))
plt.plot(np.abs(H_true), label='Истинный канал')
plt.plot(np.abs(H_est), label='Оценка (интерпол.)', linestyle='--')
plt.stem(pilot_idx, np.abs(H_pilots_est), linefmt='C2-', markerfmt='C2o', basefmt=' ', label='Пилоты')
plt.legend()
plt.xlabel('Индекс поднесущей')
plt.ylabel('Модуль отклика канала')
plt.title('Оценка канала OFDM по пилотам')
plt.grid(True)
plt.show()
