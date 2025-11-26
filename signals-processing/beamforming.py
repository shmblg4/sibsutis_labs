import numpy as np
import matplotlib.pyplot as plt

theta_deg_tx_range = (-30, 30)
theta_deg_rx_range = (-60, 60)
d = 0.5
N_ant = 8
N_beams = 20

pathloss = -40
g_0 = 10 ** (-40 * 0.1)

def generate_angles(tdtr: tuple, tdrr: tuple):
    theta_deg_tx = np.random.randint(tdtr[0], tdtr[1], 1)
    theta_deg_rx = np.random.randint(tdrr[0], tdrr[1], 1)
    return theta_deg_tx, theta_deg_rx

steering_vector_tx = []
steering_vector_rx = []

for idx in range(N_ant):
    theta_deg_tx, theta_deg_rx = generate_angles(theta_deg_tx_range, theta_deg_rx_range)
    theta_tx, theta_rx = np.radians(theta_deg_tx), np.radians(theta_deg_rx)
    steering_vector_tx.append(np.exp(-1j * 2 * np.pi * d * idx * np.sin(theta_tx)))
    steering_vector_rx.append(np.exp(-1j * 2 * np.pi * d * idx * np.sin(theta_rx)))

H = np.zeros((8, 8), dtype=np.complex128)
for idx in range(1, N_beams + 1):
    calc_H = g_0 * (np.array(steering_vector_rx) @ np.array(steering_vector_tx).conj().T)
    H += calc_H



input("Ожидаю ввод для звершения...")