# ===========================================================
# Лабораторная работа: Моделирование разнесённого приёма
# с когерентным сложением в антенной решётке (MRC)
# СибГУТИ, 2025
# ===========================================================

import numpy as np
import matplotlib.pyplot as plt
from scipy.special import erfc

# ---------------------- Параметры модели ----------------------
theta_deg = 30                          # угол прихода сигнала
theta = np.radians(theta_deg)
d = 0.5                                 # расстояние между элементами, λ/2
path_loss_dB = -40                      # потери в канале
path_loss_lin = 10**(path_loss_dB / 10)

N_bits_ber = 200_000                    # для надёжной статистики BER
N_bits_const = 1000                     # сколько точек на созвездии
SNR_const_dB = 10                       # SNR для сигнальных диаграмм (п.4, п.6)

modulation = 'BPSK'                     # 'BPSK' или 'QPSK'

SNR_dB_range = np.arange(0, 13)         # 0..12 дБ как в задании
SNR_lin_range = 10**(SNR_dB_range / 10)

# ---------------------- Модуляция ----------------------
if modulation == 'BPSK':
    bits_per_sym = 1
    def modulate(bits):
        return 2*bits - 1 + 0j
    const_points = np.array([-1, 1])
    Es = 1.0
else:  # QPSK
    bits_per_sym = 2
    gray = {0: -1-1j, 1: -1+1j, 2: 1-1j, 3: 1+1j}
    def modulate(bits):
        syms = []
        for i in range(0, len(bits), 2):
            idx = bits[i]*2 + bits[i+1]
            syms.append(gray[idx])
        return np.array(syms) / np.sqrt(2)      # нормировка мощности = 1
    const_points = np.array([-1-1j, -1+1j, 1-1j, 1+1j]) / np.sqrt(2)
    Es = 1.0

# ---------------------- Управляющий вектор ----------------------
def steering_vector(N):
    n = np.arange(N)
    return np.exp(-1j * 2 * np.pi * d * n * np.sin(theta)).reshape(-1, 1)

# ---------------------- Генерация созвездий (п.6) ----------------------
def generate_constellation(N_ant):
    a = steering_vector(N_ant)
    h = np.sqrt(path_loss_lin) * a                     # вектор канала
    w = h                                               # MRC: w = h

    sigma2 = Es / (SNR_const_dB_lin := 10**(SNR_const_dB/10))
    sigma = np.sqrt(sigma2 / 2)

    bits = np.random.randint(0, 2, N_bits_const * bits_per_sym)
    s = modulate(bits)
    s = s / np.sqrt(np.mean(np.abs(s)**2))             # точная нормировка

    noise = sigma * (np.random.randn(N_ant, len(s)) + 1j*np.random.randn(N_ant, len(s)))
    r = h @ s.reshape(1, -1) + noise                    # N_ant × N_sym

    # Одна антенна (первая) + компенсация фазы
    r_single = r[0, :]
    r_single_comp = r_single * np.exp(-1j * np.angle(h[0]))

    # MRC выход
    y_mrc = (w.conj().T @ r).flatten()

    # Компенсация общей фазы для красивого вида
    phase_ref = np.angle(np.mean(y_mrc * s.conj()))
    y_mrc_comp = y_mrc * np.exp(-1j * phase_ref)

    return r_single_comp, y_mrc_comp, s

# ---------------------- Моделирование BER ----------------------
def simulate_ber(N_ant):
    a = steering_vector(N_ant)
    h = np.sqrt(path_loss_lin) * a
    w = h

    ber_single = []
    ber_mrc = []

    n_sym = N_bits_ber // bits_per_sym

    for SNR_dB in SNR_dB_range:
        SNR_lin = 10**(SNR_dB/10)
        sigma2 = Es / SNR_lin
        sigma = np.sqrt(sigma2 / 2)

        bits = np.random.randint(0, 2, N_bits_ber)
        s = modulate(bits)
        s = s / np.sqrt(np.mean(np.abs(s)**2))

        noise = sigma * (np.random.randn(N_ant, n_sym) + 1j*np.random.randn(N_ant, n_sym))
        r = h @ s.reshape(1, -1) + noise

        # --- Приём на одной антенне ---
        r1 = r[0, :] * np.exp(-1j * np.angle(h[0]))
        if modulation == 'BPSK':
            dec_bits1 = (r1.real > 0).astype(int)
        else:
            dec_bits1 = np.concatenate(((r1.real > 0).astype(int), (r1.imag > 0).astype(int)))

        # --- MRC ---
        y = (w.conj().T @ r).flatten()
        phase_ref = np.angle(np.mean(y * s.conj()))
        y = y * np.exp(-1j * phase_ref)
        if modulation == 'BPSK':
            dec_bits_mrc = (y.real > 0).astype(int)
        else:
            dec_bits_mrc = np.concatenate(((y.real > 0).astype(int), (y.imag > 0).astype(int)))

        err1 = np.sum(bits != dec_bits1)
        err_mrc = np.sum(bits != dec_bits_mrc)

        ber_single.append(err1 / N_bits_ber)
        ber_mrc.append(err_mrc / N_bits_ber)

    return np.array(ber_single), np.array(ber_mrc)

# ===========================================================
print("1. Управляющие векторы (п.1)")
print("N=4:", steering_vector(4).flatten())
print("N=8:", steering_vector(8).flatten())

print("\nЗапуск моделирования BER...")
ber_single, ber_mrc4 = simulate_ber(4)
_, ber_mrc8 = simulate_ber(8)

print("Генерация сигнальных диаграмм при SNR=10 дБ...")
r_single_c, y_mrc4_c, _ = generate_constellation(4)
_, y_mrc8_c, _ = generate_constellation(8)

# ---------------------- Графики -------------------------

# 1. Сигнальные диаграммы (п.6)
plt.figure(figsize=(15, 5))
plt.subplot(1, 3, 1)
plt.scatter(r_single_c.real, r_single_c.imag, c='tab:blue', alpha=0.6, s=25)
plt.scatter(const_points.real, const_points.imag, c='red', s=120, marker='x', linewidths=3)
plt.title('Одна антенна\n(SNR = 10 дБ)')
plt.xlabel('Re'); plt.ylabel('Im'); plt.grid(True); plt.axis('equal')

plt.subplot(1, 3, 2)
plt.scatter(y_mrc4_c.real, y_mrc4_c.imag, c='tab:green', alpha=0.6, s=25)
plt.scatter(const_points.real, const_points.imag, c='red', s=120, marker='x', linewidths=3)
plt.title('MRC, N=4\n(выигрыш ≈6 дБ)')
plt.xlabel('Re'); plt.grid(True); plt.axis('equal')

plt.subplot(1, 3, 3)
plt.scatter(y_mrc8_c.real, y_mrc8_c.imag, c='tab:purple', alpha=0.6, s=25)
plt.scatter(const_points.real, const_points.imag, c='red', s=120, marker='x', linewidths=3)
plt.title('MRC, N=8\n(выигрыш ≈9 дБ)')
plt.xlabel('Re'); plt.grid(True); plt.axis('equal')

plt.suptitle(f'Сигнальные диаграммы ({modulation}), θ = {theta_deg}°, потери –40 дБ', fontsize=16)
plt.tight_layout()
plt.show()

# 2. График BER(SNR) (п.7)
plt.figure(figsize=(10, 7))
plt.semilogy(SNR_dB_range, ber_single, 'b^-', label='1 антенна')
plt.semilogy(SNR_dB_range, ber_mrc4, 'go-', label='MRC, N=4')
plt.semilogy(SNR_dB_range, ber_mrc8, 'rs-', label='MRC, N=8')

# Теоретическая кривая для SISO
if modulation == 'BPSK':
    ber_theory = 0.5 * erfc(np.sqrt(SNR_lin_range))
else:
    ber_theory = erfc(np.sqrt(SNR_lin_range / 2)) / 2   # приближение для Gray-кодированной QPSK
plt.semilogy(SNR_dB_range, ber_theory, 'k--', linewidth=2, label='Теория SISO')

plt.xlabel('SNR, дБ')
plt.ylabel('Вероятность битовой ошибки (BER)')
plt.title(f'Зависимость BER от SNR\nMRC при θ = {theta_deg}°, {modulation}')
plt.grid(True, which="both", ls="--")
plt.legend()
plt.ylim(1e-6, 1)
plt.xlim(0, 12)
plt.show()

# ---------------------- Выигрыш (п.7) ----------------------
def gain_at_ber(target=1e-3):
    snr1 = np.interp(np.log10(target), np.log10(ber_single[::-1]), SNR_dB_range[::-1])
    snr4 = np.interp(np.log10(target), np.log10(ber_mrc4[::-1]), SNR_dB_range[::-1])
    snr8 = np.interp(np.log10(target), np.log10(ber_mrc8[::-1]), SNR_dB_range[::-1])
    return snr1, snr1-snr4, snr1-snr8

snr_ref, g4, g8 = gain_at_ber(1e-3)
print(f"\n=== Выигрыш антенной решётки при BER = 10⁻³ ===")
print(f"SNR для 1 антенны: {snr_ref:.1f} дБ")
print(f"Выигрыш при N=4:   {g4:.1f} дБ  (теория: {10*np.log10(4):.1f} дБ)")
print(f"Выигрыш при N=8:   {g8:.1f} дБ  (теория: {10*np.log10(8):.1f} дБ)")

print("\nРабота полностью выполнена!")