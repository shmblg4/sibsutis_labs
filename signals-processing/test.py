import numpy as np
from numpy.fft import fft, fftshift, fftfreq
import matplotlib.pyplot as plt

c = 3e8

def section1(fc=2e9, F=16, V=10.0, D=1000.0, N=100, NFFT=128, alpha=0.0):
    lam = c / fc
    dx = lam / F
    x = np.arange(N) * dx
    t = x / V
    ts = dx / V
    fs = 1.0 / ts
    BSx, BSy = D*np.cos(alpha), D*np.sin(alpha)
    dist = np.sqrt((BSx - x)**2 + (BSy - 0.0)**2)
    k = 2*np.pi/lam
    r = np.exp(-1j * k * dist)
    spec = fftshift(fft(np.pad(r, (0, max(0, NFFT-len(r))), mode='constant')[:NFFT]))
    f = np.linspace(-fs/2, fs/2, NFFT, endpoint=False)
    return dict(t=t, dist=dist, r=r, amp=np.abs(r), phase=np.angle(r),
                f=f, spec=np.abs(spec), fs=fs, fd_max=V*fc/c)

def random_scatterers(num=50, radius=100.0, rmin=10.0, seed=42):
    rng = np.random.default_rng(seed)
    ang = rng.uniform(0, 2*np.pi, num)
    rad = rng.uniform(rmin, radius, num)
    scx = rad*np.cos(ang); scy = rad*np.sin(ang)
    return np.stack([scx, scy], axis=1)

def section2(fc=2e9, V=10.0, N=1000, track_len=200.0, BS=(1000.0,1000.0),
             num_sc=50, sc_radius=100.0, seed=42):
    lam = c / fc; k = 2*np.pi/lam
    x = np.linspace(0, track_len, N); y = np.zeros_like(x)
    MS = np.stack([x, y], axis=1)
    SC = random_scatterers(num_sc, sc_radius, seed=seed)
    BSv = np.array(BS)
    dBS_SC = np.linalg.norm(BSv - SC, axis=1)
    r = np.zeros(N, dtype=complex)
    for n in range(N):
        dSC_MS = np.linalg.norm(SC - MS[n], axis=1)
        d = dBS_SC + dSC_MS
        r[n] = np.sum(np.exp(-1j*k*d))
    t = (x - x[0]) / V
    NFFT = 1024
    spec = fftshift(np.abs(fft(r, NFFT))**2)
    f = fftshift(fftfreq(NFFT, d=(t[1]-t[0])))
    ac = np.correlate(r, r, mode='full'); ac = ac[ac.size//2:]; ac = ac/ac[0]
    return dict(t=t, r=r, amp=np.abs(r), phase=np.angle(r),
                f=f, spec=spec, AC=ac, MS=MS, SC=SC, BS=BSv)

def section3(fc=2e9, F=16, V=10.0, band=(1.9975e9, 2.0025e9), step=1e7, taus=[0.0, 1e-7], gains=[1.0, 0.5], N=4096, fs=1e5):
    t = np.arange(N)/fs
    fcs = np.arange(band[0], band[1] + 0.5*step, step)  # 0.01 МГц шаг = 1e4 Гц (уточните по заданию)
    sigs = []
    for fc_i in fcs:
        s = np.zeros_like(t, dtype=complex)
        for a, tau in zip(gains, taus):
            s += a*np.exp(1j*2*np.pi*fc_i*(t - tau))
        sigs.append(s)
    # Возьмем один fc для иллюстрации спектра
    S = fftshift(np.abs(fft(sigs[len(sigs)//2]))**2)
    f = fftshift(fftfreq(N, d=1/fs))*fs
    return dict(t=t, f=f, S=S, fcs=fcs, example=sigs[len(sigs)//2])

if __name__ == "__main__":
    s1 = section1()
    s2 = section2()
    s3 = section3()
    # Графики для раздела 1
    fig1, ax = plt.subplots(2,2, figsize=(10,7))
    ax[0,0].plot(s1['t'], s1['dist']); ax[0,0].set_title('Расстояние vs t')
    ax[0,1].plot(s1['t'], s1['amp']); ax[0,1].set_title('Амплитуда vs t')
    ax[1,0].plot(s1['t'], s1['phase']); ax[1,0].set_title('Фаза vs t')
    ax[1,1].plot(s1['f'], s1['spec']); ax[1,1].set_title('Доплер-спектр')
    fig1.tight_layout(); fig1.savefig('section1.png', dpi=200)

    # Сценарий многолучевого канала
    fig2, ax2 = plt.subplots(figsize=(6,5))
    ax2.scatter(s2['SC'][:,0], s2['SC'][:,1], s=20, c='tab:blue', label='SC')
    ax2.scatter([s2['BS'][0]], [s2['BS'][1]], marker='s', c='tab:red', label='BS')
    ax2.scatter([s2['MS'][0,0]], [s2['MS'][0,1]], marker='^', c='tab:green', label='MS start')
    ax2.plot(s2['MS'][:,0], s2['MS'][:,1], '--', c='tab:green', label='MS track')
    ax2.legend(); ax2.grid(True); ax2.set_title('Сценарий многолучевого канала')
    fig2.tight_layout(); fig2.savefig('section2_scene.png', dpi=200)

    # Спектр многолучевого
    fig3, ax3 = plt.subplots(1,2, figsize=(10,4))
    ax3[0].plot(s2['t'], s2['amp']); ax3[0].set_title('Амплитуда (релееевские замирания)')
    ax3[1].plot(s2['f'], 10*np.log10(s2['spec']+1e-12)); ax3[1].set_title('Доплер-спектр (дБ)')
    fig3.tight_layout(); fig3.savefig('section2.png', dpi=200)

    # Раздел 3: пример спектра
    fig4, ax4 = plt.subplots(figsize=(7,4))
    ax4.plot(s3['f'], 10*np.log10(s3['S']+1e-12)); ax4.set_title('Двухчастотный сигнал: спектр')
    fig4.tight_layout(); fig4.savefig('section3.png', dpi=200)
