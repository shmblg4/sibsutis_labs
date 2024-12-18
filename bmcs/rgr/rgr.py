import matplotlib.pyplot as plt
import numpy as np
import random

def convert_string_to_bits(input_string):
    return [int(bit) for char in input_string for bit in bin(ord(char))[2:].zfill(8)]

def display_bit_sequence(bits, title="Bit Sequence"):
    plt.figure(figsize=(10, 4))
    plt.step(range(len(bits)), bits, where='post')
    plt.ylim(-0.5, 1.5)
    plt.title(title)
    plt.xlabel("Index")
    plt.ylabel("Value")
    plt.grid(True)
    plt.show()

def compute_crc(data_bits, polynomial):
    extended_bits = data_bits + [0] * (len(polynomial) - 1)
    for i in range(len(data_bits)):
        if extended_bits[i] == 1:
            for j in range(len(polynomial)):
                extended_bits[i + j] ^= polynomial[j]
    return extended_bits[len(data_bits):]

def create_gold_sequence(init_reg1, init_reg2, length):
    reg1, reg2 = init_reg1[:], init_reg2[:]
    sequence = []
    for _ in range(length):
        output1, output2 = reg1[-1], reg2[-1]
        reg1 = [reg1[1] ^ reg1[-1]] + reg1[:-1]
        reg2 = [reg2[0] ^ reg2[1] ^ reg2[2]] + reg2[:-1]
        sequence.append(output1 ^ output2)
    return sequence

def bits_to_samples(bits, samples_per_bit):
    return [bit for bit in bits for _ in range(samples_per_bit)]

def align_signal_with_pattern(signal, pattern):
    normalized_correlation = np.correlate(np.array(signal), np.array(pattern), mode='valid')
    return normalized_correlation / (np.linalg.norm(signal) * np.linalg.norm(pattern))

def interpret_samples_as_bits(samples, samples_per_bit, threshold=0.5):
    return [1 if np.mean(samples[i:i + samples_per_bit]) > threshold else 0 for i in range(0, len(samples), samples_per_bit)]

def validate_crc(data, crc, polynomial):
    return all(bit == 0 for bit in compute_crc(data + crc, polynomial))

def decode_bits_to_string(bits):
    return ''.join(chr(int(''.join(map(str, bits[i:i+8])), 2)) for i in range(0, len(bits), 8))

def show_signal_spectrum(signal, sampling_rate, title="Spectrum"):
    frequencies = np.fft.fftfreq(len(signal), 1/sampling_rate)
    spectrum = np.abs(np.fft.fft(signal))
    plt.figure(figsize=(10, 4))
    plt.plot(frequencies, spectrum)
    plt.title(title)
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Magnitude")
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    full_name = input("Enter your name in latin letters: ")
    if full_name == "":
        full_name = "John Doe"

    bit_sequence = convert_string_to_bits(full_name)
    print("Bit sequence:", bit_sequence)
    display_bit_sequence(bit_sequence, "Input Bit Sequence")

    crc_generator = [1, 0, 1, 1, 1, 1, 1, 1]
    crc_bits = compute_crc(bit_sequence, crc_generator)
    print("CRC bits:", crc_bits)

    transmitted_sequence = bit_sequence + crc_bits

    reg1_initial = [1, 0, 1, 0, 1]
    reg2_initial = [1, 0, 1, 1, 1]
    gold_sequence = create_gold_sequence(reg1_initial, reg2_initial, 31)
    final_sequence = gold_sequence + transmitted_sequence
    
    samples_per_bit = 10
    signal_samples = bits_to_samples(final_sequence, samples_per_bit)
    noise_sigma = float(input("Enter noise standard deviation (sigma): "))
    noisy_signal = np.array(signal_samples) + np.random.normal(0, noise_sigma, len(signal_samples))
    gold_samples = bits_to_samples(gold_sequence, samples_per_bit)
    correlation = align_signal_with_pattern(noisy_signal, gold_samples)
    start_idx = np.argmax(correlation)
    received_samples = noisy_signal[start_idx:]
    decoded_bits = interpret_samples_as_bits(received_samples, samples_per_bit)
    received_data = decoded_bits[len(gold_sequence):-len(crc_bits)]
    if validate_crc(received_data, decoded_bits[-len(crc_bits):], crc_generator):
        print("No errors detected!")
        decoded_message = decode_bits_to_string(received_data)
        print("Decoded message:", decoded_message)
    else:
        print("Errors detected in received message!")
    display_bit_sequence(signal_samples, "Transmitted Signal")
    display_bit_sequence(noisy_signal, "Noisy Signal")
    show_signal_spectrum(noisy_signal, 100, "Noisy Signal Spectrum")


