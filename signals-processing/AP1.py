import numpy as np
import matplotlib.pyplot as plt


D_LAMBDA = 0.55  # Отношение расстояния между элементами к длине волны
THETA_3DB = 70.0  # Ширина луча по уровню -3 дБ в градусах
A_M = 20.0        # Максимальное ослабление в дБ


def calculate_af_linear(theta_deg, N, d_lambda, theta0_deg=0):
    theta_rad = np.deg2rad(theta_deg)
    theta0_rad = np.deg2rad(theta0_deg)
    k_array = np.arange(N)
    phase_diff = 2j * np.pi * d_lambda * (np.sin(theta_rad[:, np.newaxis]) - np.sin(theta0_rad))
    exponents = k_array[np.newaxis, :] * phase_diff
    phasors = np.exp(exponents)
    return np.sum(phasors, axis=1)


def calculate_g_element_db(theta_deg, theta_3db=THETA_3DB, a_m=A_M):
    theta_deg = (theta_deg + 180) % 360 - 180
    g = -np.minimum(12 * (theta_deg / theta_3db) ** 2, a_m)
    return g


def calculate_af_planar(theta_deg, phi_deg, Nv, Nh, dv_lambda, dh_lambda, theta0_deg=0, phi0_deg=0):
    theta_rad = np.deg2rad(theta_deg)
    phi_rad = np.deg2rad(phi_deg)
    theta0_rad = np.deg2rad(theta0_deg)
    phi0_rad = np.deg2rad(phi0_deg)

    m = np.arange(Nv)
    phase_v = 2j * np.pi * dv_lambda * (np.cos(theta_rad) - np.cos(theta0_rad))
    af_v = np.sum(np.exp(m[np.newaxis, np.newaxis, :] * phase_v[:, :, np.newaxis]), axis=2)

    n = np.arange(Nh)
    phase_h = 2j * np.pi * dh_lambda * (np.sin(theta_rad) * np.sin(phi_rad) - np.sin(theta0_rad) * np.sin(phi0_rad))
    af_h = np.sum(np.exp(n[np.newaxis, np.newaxis, :] * phase_h[:, :, np.newaxis]), axis=2)

    return af_v, af_h


def main():
    print("▶ Линейная антенная решетка...")
    N_values = [4, 8]
    colors = ['darkgreen', 'darkorange']
    theta_deg_1d = np.linspace(-180, 180, 361)
    theta_rad_1d = np.deg2rad(theta_deg_1d)

    # --- ГРАФИКИ 1 (ЛИНЕЙНАЯ АР) ---
    for i, N in enumerate(N_values):
        fig, axes = plt.subplots(1, 3, figsize=(18, 5), subplot_kw={'projection': 'polar'})
        fig.suptitle(f'Линейная антенная решетка N={N}, D/λ={D_LAMBDA}', fontsize=16)
        current_color = colors[i]

        # --- Задание 3: ДН решетки ---
        af = calculate_af_linear(theta_deg_1d, N, D_LAMBDA)
        gain_db = 10 * np.log10(np.abs(af) ** 2 / N ** 2)
        ax = axes[0]
        ax.plot(theta_rad_1d, gain_db, color=current_color)
        ax.set_title(f'ДН решетки (N={N})')
        ax.set_rlim(-40, 0)
        ax.grid(True)

        # --- Задание 4: ДН с учетом ДН элемента ---
        element_gain_db = calculate_g_element_db(theta_deg_1d)
        total_gain_db = gain_db + element_gain_db
        ax = axes[1]
        ax.plot(theta_rad_1d, total_gain_db, color=current_color)
        ax.set_title('Общая ДН')
        ax.set_rlim(-40, 15)
        ax.grid(True)

        # --- Задание 5: Поворот ДН ---
        theta0_deg = 30.0
        af_steered = calculate_af_linear(theta_deg_1d, N, D_LAMBDA, theta0_deg)
        gain_steered_db = 10 * np.log10(np.abs(af_steered) ** 2 / N ** 2)
        total_gain_steered_db = gain_steered_db + element_gain_db
        ax = axes[2]
        ax.plot(theta_rad_1d, total_gain_steered_db, color=current_color)
        ax.set_title(f'Поворот ДН на {theta0_deg}°')
        ax.set_rlim(-40, 15)
        ax.grid(True)

        fig.tight_layout(rect=[0, 0.03, 1, 0.9])
        plt.show()  # ⬅ Показываем графики по очереди

    # --- Задания 6-7: Планарная АР ---
    print("\n▶ Планарная антенная решетка...")

    Nv, Nh = 8, 8
    theta0_planar_deg, phi0_planar_deg = 30.0, 45.0
    phi_vec_deg = np.linspace(0, 360, 181)
    theta_vec_deg = np.linspace(0.001, 90, 91)
    phi_grid_deg, theta_grid_deg = np.meshgrid(phi_vec_deg, theta_vec_deg)

    phi_grid_rad = np.deg2rad(phi_grid_deg)
    theta_grid_rad = np.deg2rad(theta_grid_deg)

    # --- Задание 6 ---
    af_v, af_h = calculate_af_planar(theta_grid_deg, phi_grid_deg, Nv, Nh, D_LAMBDA, D_LAMBDA)
    gain_planar = np.abs(af_v) ** 2 * np.abs(af_h) ** 2
    gain_planar_db = 10 * np.log10(gain_planar / (Nv * Nh) ** 2)
    element_gain_planar_db = calculate_g_element_db(theta_grid_deg)
    total_gain_planar_db = gain_planar_db + element_gain_planar_db

    # --- Задание 7 ---
    af_v_s, af_h_s = calculate_af_planar(theta_grid_deg, phi_grid_deg, Nv, Nh, D_LAMBDA, D_LAMBDA,
                                         theta0_deg=theta0_planar_deg, phi0_deg=phi0_planar_deg)
    gain_steered_planar = np.abs(af_v_s) ** 2 * np.abs(af_h_s) ** 2
    gain_steered_planar_db = 10 * np.log10(gain_steered_planar / (Nv * Nh) ** 2)
    total_gain_steered_planar_db = gain_steered_planar_db + element_gain_planar_db

    # --- ГРАФИК 2: обычная планарная АР ---
    fig2 = plt.figure(figsize=(10, 6))
    fig2.suptitle(f'Планарная антенная решетка {Nv}x{Nh} (без поворота)', fontsize=16)
    ax = fig2.add_subplot(111, polar=True)
    r_proj = np.sin(theta_grid_rad)
    c = ax.pcolormesh(phi_grid_rad, r_proj, total_gain_planar_db, cmap='magma', shading='auto', vmin=-30)
    ax.set_yticks(np.sin(np.deg2rad([0, 30, 60, 90])))
    ax.set_yticklabels(['0°', '30°', '60°', '90°'])
    fig2.colorbar(c, ax=ax, label='Усиление (дБ)')
    plt.show()  # ⬅ Показываем первый график планарной решетки

    # --- ГРАФИК 3: повернутая планарная АР ---
    fig3 = plt.figure(figsize=(10, 6))
    fig3.suptitle(f'Планарная антенная решетка {Nv}x{Nh} (поворот θ={theta0_planar_deg}°, φ={phi0_planar_deg}°)', fontsize=16)
    ax = fig3.add_subplot(111, polar=True)
    r_proj = np.sin(theta_grid_rad)
    c = ax.pcolormesh(phi_grid_rad, r_proj, total_gain_steered_planar_db, cmap='magma', shading='auto', vmin=-30)
    ax.set_yticks(np.sin(np.deg2rad([0, 30, 60, 90])))
    ax.set_yticklabels(['0°', '30°', '60°', '90°'])
    fig3.colorbar(c, ax=ax, label='Усиление (дБ)')
    plt.show()  # ⬅ Показываем второй график планарной решетки


if __name__ == "__main__":
    main()
