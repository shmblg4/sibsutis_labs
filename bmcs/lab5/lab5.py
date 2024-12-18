import matplotlib.pyplot as plt
import random


def compute_crc(data, generator):
    data = data + [0] * (len(generator) - 1)
    gen_len = len(generator)

    for i in range(len(data) - gen_len + 1):
        if data[i] == 1:
            for j in range(gen_len):
                data[i + j] ^= generator[j]

    return data[-(gen_len - 1) :]


def introduce_errors(data, num_errors):
    error_positions = random.sample(range(len(data)), num_errors)
    for pos in error_positions:
        data[pos] ^= 1
    return data


def test_crc_fixed_sequence(generator, num_tests, data_length, num_errors):
    missed_errors = 0

    for _ in range(num_tests):
        # Генерация случайной последовательности
        data = [random.randint(0, 1) for _ in range(data_length)]

        crc = compute_crc(data[:], generator)
        transmitted_data = data + crc

        corrupted_data = transmitted_data[:]
        corrupted_data = introduce_errors(corrupted_data, num_errors)

        remainder = compute_crc(corrupted_data[:], generator)
        if all(r == 0 for r in remainder):
            missed_errors += 1

    return missed_errors


def plot_stem(results, title):
    lengths = list(results.keys())
    missed = list(results.values())

    plt.figure()
    plt.stem(lengths, missed)
    print(lengths, missed)
    plt.title(title)
    plt.xlabel("Generator Length (bits)")
    plt.ylabel("Missed Errors")
    plt.grid(True)
    plt.show()


def main():
    crc_generators = {
        4: [1, 0, 1, 1],
        8: [1, 0, 1, 1, 1, 1, 1, 1],
        16: [1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1],
        24: [1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1],
    }

    num_tests = 2000  # Количество тестов
    data_length = 2000  # Длина случайной последовательности
    num_errors = 2  # Количество ошибок

    results = {}

    for bits, generator in crc_generators.items():
        missed_errors = test_crc_fixed_sequence(
            generator, num_tests, data_length, num_errors
        )
        results[bits] = missed_errors

    title = "Missed Errors vs Generator Length (Multiple Bit Errors)"
    plot_stem(results, title)


if __name__ == "__main__":
    main()
