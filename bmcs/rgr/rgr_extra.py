import numpy as np
import matplotlib.pyplot as plt

# Генерим последовательность бит
def generate_test_bits(length=64):
    return np.random.choice([0, 1], size=length).tolist()


# Функция преобразования бит в сэмплы
def bits_to_samples(bits, samples_per_bit):
    return [bit for bit in bits for _ in range(samples_per_bit)]


# Функция добавления шума
def add_noise(signal, sigma):
    return np.array(signal) + np.random.normal(0, sigma, len(signal))


# Функция интерпретации сэмплов в биты
def interpret_samples_as_bits(samples, samples_per_bit, threshold=0.5):
    return [
        1 if np.mean(samples[i : i + samples_per_bit]) > threshold else 0
        for i in range(0, len(samples), samples_per_bit)
    ]


# Функция вычисления BER
def calculate_ber(original_bits, received_bits):
    errors = sum(o != r for o, r in zip(original_bits, received_bits))
    return errors / len(original_bits)


# Параметры симуляции
samples_per_bit_values = [5, 10, 20, 50]
sigmas = np.linspace(0.1, 2.0, 10)
num_trials = 100
bit_length = 64

plt.figure(figsize=(12, 8))

for samples_per_bit in samples_per_bit_values:
    average_bers = []

    for sigma in sigmas:
        total_ber = 0.0

        for _ in range(num_trials):
            original_bits = generate_test_bits(bit_length)
            signal_samples = bits_to_samples(original_bits, samples_per_bit)
            noisy_signal = add_noise(signal_samples, sigma)
            decoded_bits = interpret_samples_as_bits(noisy_signal, samples_per_bit)
            total_ber += calculate_ber(original_bits, decoded_bits)

        average_bers.append(total_ber / num_trials)

    plt.plot(
        sigmas,
        average_bers,
        marker="o",
        linestyle="-",
        label=f"Samples/Bit: {samples_per_bit}",
    )

plt.title("BER vs Noise Level for Different Samples Per Bit")
plt.xlabel("Noise Standard Deviation (sigma)")
plt.ylabel("Average BER")
plt.grid(True, linestyle="--", linewidth=0.5)
plt.legend()
plt.show()
