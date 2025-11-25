#!/usr/bin/env python3
"""
Полная модель системы OFDM с оценкой канала на пилотных поднесущих
Основано на документе "Прием OFDM. Оценка канала"
"""

import numpy as np
import matplotlib.pyplot as plt

# ========== ПАРАМЕТРЫ СИСТЕМЫ OFDM ==========
Nfft = 64  # Число поднесущих (размер FFT)
Np = 8     # Количество пилотных поднесущих
Nps = Nfft // Np  # Интервал между пилотами D = Nfft/Np
cp_len = 16  # Длина циклического префикса (защитного интервала)
num_symbols = 5  # Количество OFDM символов для моделирования
snr_db = 15  # SNR в дБ

# ========== ФОРМИРОВАНИЕ ПИЛОТНЫХ И ДАННЫХ ПОДНЕСУЩИХ ==========
pilot_loc = np.arange(0, Nfft, Nps)  # Индексы пилотных поднесущих
data_loc = np.setdiff1d(np.arange(Nfft), pilot_loc)  # Индексы данных

print(f"Параметры системы OFDM:")
print(f"Количество поднесущих: {Nfft}")
print(f"Количество пилотов: {Np}")
print(f"Интервал между пилотами D = Nfft/Np: {Nps}")
print(f"Длина защитного префикса: {cp_len}")
print(f"Индексы пилотов: {pilot_loc}")
print(f"Количество символов данных: {len(data_loc)}")
print()

# ========== ГЕНЕРАЦИЯ ДАННЫХ ==========
# QPSK модуляция для данных
qpsk_symbols = np.array([1+1j, 1-1j, -1+1j, -1-1j]) / np.sqrt(2)

# Пилотные символы (известны на приемнике и передатчике)
pilot_symbols = np.ones(Np, dtype=complex)  # BPSK: все пилоты = 1

# Генерация OFDM символов
tx_symbols = np.zeros((num_symbols, Nfft), dtype=complex)

for sym_idx in range(num_symbols):
    # Заполнение данными (случайные QPSK символы)
    data_symbols = qpsk_symbols[np.random.randint(0, 4, len(data_loc))]

    # Размещение пилотов и данных по правилу:
    # X[k] = X[mD + d] = { Xp(m), если d = 0
    #                    { Xc,    если d = 1, 2, ..., D-1
    tx_symbols[sym_idx, pilot_loc] = pilot_symbols
    tx_symbols[sym_idx, data_loc] = data_symbols

print(f"Сформировано {num_symbols} OFDM символов")
print()

# ========== ПЕРЕДАТЧИК: IFFT И ДОБАВЛЕНИЕ ЦИКЛИЧЕСКОГО ПРЕФИКСА ==========
# Формирование сигнала во временной области через ОБРАТНОЕ ДПФ (IFFT)
tx_time_domain = np.fft.ifft(tx_symbols, axis=1)

# Добавление циклического префикса (защитного интервала)
# Берутся последние cp_len отсчетов и вставляются перед символом
tx_with_cp = np.zeros((num_symbols, Nfft + cp_len), dtype=complex)
for sym_idx in range(num_symbols):
    cp = tx_time_domain[sym_idx, -cp_len:]
    tx_with_cp[sym_idx] = np.concatenate([cp, tx_time_domain[sym_idx]])

print(f"Добавлен циклический префикс длиной {cp_len}")
print(f"Длина OFDM символа с CP: {Nfft + cp_len} отсчетов")
print()

# ========== КАНАЛ: МНОГОЛУЧЕВОЙ ФЕДИНГ И AWGN ШУМ ==========
# Моделирование частотно-селективного канала (многолучевость)
# Импульсная характеристика канала h(τ)
channel_taps = np.array([1.0, 0.5*np.exp(1j*np.pi/4), 0.3*np.exp(1j*np.pi/2)])
print(f"Импульсная характеристика канала: {len(channel_taps)} лучей")

# Частотная характеристика канала H[k] (для сравнения с оценкой)
H_true = np.fft.fft(channel_taps, Nfft)

# Передача через канал: y(t) = ∫ h(τ)x(t-τ)dτ (свертка)
rx_signal = np.zeros_like(tx_with_cp)
for sym_idx in range(num_symbols):
    rx_signal[sym_idx] = np.convolve(tx_with_cp[sym_idx], channel_taps, mode='same')

# Добавление AWGN шума
signal_power = np.mean(np.abs(rx_signal) ** 2)
noise_power = signal_power / (10 ** (snr_db / 10))
noise = np.sqrt(noise_power / 2) * (np.random.randn(*rx_signal.shape) + 
                                      1j * np.random.randn(*rx_signal.shape))
rx_signal += noise

print(f"Прохождение через канал с SNR = {snr_db} дБ")
print()

# ========== ПРИЕМНИК: УДАЛЕНИЕ CP И FFT ==========
# Удаление защитного префикса
rx_no_cp = rx_signal[:, cp_len:]

# Прямое ДПФ (FFT): Y[k] = Σ y(n)e^(-j2πnk/N) = H[k]X[k] + n[k]
rx_freq = np.fft.fft(rx_no_cp, axis=1)

print(f"На приемнике: удален CP и выполнено FFT")
print()

# ========== ОЦЕНКА КАНАЛА ПО МЕТОДУ НАИМЕНЬШИХ КВАДРАТОВ ==========
# Оценка на пилотах: H_LS[k] = Yp[k] / Xp[k]
H_est_pilots = np.zeros((num_symbols, Np), dtype=complex)

for sym_idx in range(num_symbols):
    Y_pilots = rx_freq[sym_idx, pilot_loc]
    X_pilots = tx_symbols[sym_idx, pilot_loc]
    H_est_pilots[sym_idx] = Y_pilots / X_pilots

print(f"Оценка канала на {Np} пилотных поднесущих (метод наименьших квадратов)")
print()

# ========== ЛИНЕЙНАЯ ИНТЕРПОЛЯЦИЯ ЧХ НА ВСЕ ПОДНЕСУЩИЕ ==========
# Формула: H(c) = H(a) + (H(b) - H(a)) * (c - a) / (b - a)
# или: Hc(k) = (d/D)(Hp(m+1) - Hp(m)) + Hp(m)
H_est_interp = np.zeros((num_symbols, Nfft), dtype=complex)

for sym_idx in range(num_symbols):
    # Интерполяция отдельно для реальной и мнимой части
    H_est_interp[sym_idx].real = np.interp(np.arange(Nfft), pilot_loc, 
                                             H_est_pilots[sym_idx].real)
    H_est_interp[sym_idx].imag = np.interp(np.arange(Nfft), pilot_loc, 
                                             H_est_pilots[sym_idx].imag)

print(f"Линейная интерполяция оценки канала на все {Nfft} поднесущих")
print()

# ========== КОРРЕКЦИЯ (ЭКВАЛИЗАЦИЯ) ПРИНЯТЫХ СИМВОЛОВ ==========
# Эквализация: Xeq[k] = Y[k] / H_est[k]
rx_equalized = rx_freq / H_est_interp

print(f"Эквализация принятых символов")
print()

# ========== ДЕМОДУЛЯЦИЯ И РАСЧЕТ BER ==========
# Извлекаем данные (без пилотов)
tx_data_symbols = tx_symbols[:, data_loc]
rx_data_symbols = rx_equalized[:, data_loc]

# Демодуляция QPSK (жесткое решение)
def qpsk_demod(symbols):
    decisions = np.zeros(symbols.shape, dtype=complex)
    decisions[np.logical_and(symbols.real >= 0, symbols.imag >= 0)] = qpsk_symbols[0]
    decisions[np.logical_and(symbols.real >= 0, symbols.imag < 0)] = qpsk_symbols[1]
    decisions[np.logical_and(symbols.real < 0, symbols.imag >= 0)] = qpsk_symbols[2]
    decisions[np.logical_and(symbols.real < 0, symbols.imag < 0)] = qpsk_symbols[3]
    return decisions

rx_decisions = qpsk_demod(rx_data_symbols)

# Расчет количества ошибок
errors = np.sum(tx_data_symbols != rx_decisions)
total_symbols = tx_data_symbols.size
ber = errors / total_symbols

print(f"\n=== РЕЗУЛЬТАТЫ ===")
print(f"Всего передано символов данных: {total_symbols}")
print(f"Количество ошибок: {errors}")
print(f"BER (Bit Error Rate): {ber:.6f}")
print()

# ========== ВИЗУАЛИЗАЦИЯ РЕЗУЛЬТАТОВ ==========
fig, axes = plt.subplots(2, 2, figsize=(14, 10))

# График 1: Сравнение истинной и оценённой ЧХ канала
ax1 = axes[0, 0]
ax1.plot(np.arange(Nfft), np.abs(H_true), 'b-', linewidth=2, label='Истинная ЧХ канала')
ax1.plot(np.arange(Nfft), np.abs(H_est_interp[0]), 'r--', linewidth=1.5, label='Оценка (интерполяция)')
ax1.stem(pilot_loc, np.abs(H_est_pilots[0]), linefmt='g-', markerfmt='go', 
         basefmt=' ', label='Оценка на пилотах')
ax1.set_xlabel('Индекс поднесущей k')
ax1.set_ylabel('|H[k]|')
ax1.set_title('Частотная характеристика канала')
ax1.legend()
ax1.grid(True, alpha=0.3)

# График 2: Фазовая характеристика канала
ax2 = axes[0, 1]
ax2.plot(np.arange(Nfft), np.angle(H_true), 'b-', linewidth=2, label='Истинная фаза')
ax2.plot(np.arange(Nfft), np.angle(H_est_interp[0]), 'r--', linewidth=1.5, label='Оценка фазы')
ax2.stem(pilot_loc, np.angle(H_est_pilots[0]), linefmt='g-', markerfmt='go', 
         basefmt=' ', label='Фаза на пилотах')
ax2.set_xlabel('Индекс поднесущей k')
ax2.set_ylabel('∠H[k] (радианы)')
ax2.set_title('Фазовая характеристика канала')
ax2.legend()
ax2.grid(True, alpha=0.3)

# График 3: Созвездие QPSK до эквализации
ax3 = axes[1, 0]
rx_before_eq = rx_freq[0, data_loc]
ax3.scatter(rx_before_eq.real, rx_before_eq.imag, alpha=0.5, s=20, label='Принятые')
ax3.scatter(qpsk_symbols.real, qpsk_symbols.imag, c='red', s=100, marker='x', 
            linewidths=3, label='Идеальные точки')
ax3.set_xlabel('I (действительная часть)')
ax3.set_ylabel('Q (мнимая часть)')
ax3.set_title('Созвездие до эквализации')
ax3.legend()
ax3.grid(True, alpha=0.3)
ax3.axis('equal')

# График 4: Созвездие QPSK после эквализации
ax4 = axes[1, 1]
rx_after_eq = rx_equalized[0, data_loc]
ax4.scatter(rx_after_eq.real, rx_after_eq.imag, alpha=0.5, s=20, label='После эквализации')
ax4.scatter(qpsk_symbols.real, qpsk_symbols.imag, c='red', s=100, marker='x', 
            linewidths=3, label='Идеальные точки')
ax4.set_xlabel('I (действительная часть)')
ax4.set_ylabel('Q (мнимая часть)')
ax4.set_title('Созвездие после эквализации')
ax4.legend()
ax4.grid(True, alpha=0.3)
ax4.axis('equal')

plt.tight_layout()
plt.savefig('ofdm_results.png', dpi=150)
print("\nГрафики сохранены в файл 'ofdm_results.png'")

plt.show()
print("\nМоделирование завершено успешно!")
