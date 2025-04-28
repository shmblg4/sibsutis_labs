% Параметры
Fs = 16000;        % Частота дискретизации
T = 1;             % Длительность сигнала (сек)
N = Fs * T;        % Количество отсчетов
num_realizations = 1000; % Число реализаций для усреднения

% Генерация белого шума
white_noise = randn(N, 1);

% Проектирование эллиптического ФНЧ
Rp = 0.5;          % Неравномерность в полосе (дБ)
Rs = 50;           % Подавление (дБ)
n = 6;             % Порядок фильтра
Wn_FNCh = 3000 / (Fs/2); % Нормированная частота среза

[b_low, a_low] = ellip(n, Rp, Rs, Wn_FNCh, 'low');

% Проектирование эллиптического ПФ
f1 = 3000; f2 = 6000;
Wp = [f1, f2] / (Fs/2);   % Границы полосы пропускания
Ws = [f1-500, f2+500] / (Fs/2); % Границы полосы задерживания

[n_pf, Wn_pf] = ellipord(Wp, Ws, Rp, Rs);
[b_pf, a_pf] = ellip(n_pf, Rp, Rs, Wn_pf, 'bandpass');

% Фильтрация белого шума
filtered_low = filter(b_low, a_low, white_noise);
filtered_pf = filter(b_pf, a_pf, white_noise);

% Графики реализаций
figure;
subplot(3,1,1);
plot((0:N-1)/Fs, white_noise);
xlim([0, 0.005]);
title('Белый шум на входе');
subplot(3,1,2);
plot((0:N-1)/Fs, filtered_low);
xlim([0, 0.005]);
title('Сигнал после ФНЧ');
subplot(3,1,3);
plot((0:N-1)/Fs, filtered_pf);
xlim([0, 0.005]);
title('Сигнал после ПФ');

% АКФ и СПМ входного шума
[acf_white, lags] = xcorr(white_noise, 'coeff');
lags_sec = lags / Fs;

figure;
subplot(2,1,1);
plot(lags_sec, acf_white);
xlim([0, 0.005]);
title('АКФ белого шума');
xlabel('Время (с)');

[pxx_white, f_white] = pwelch(white_noise, [], [], [], Fs);
subplot(2,1,2);
plot(f_white, 10*log10(pxx_white));
% xlim([0, 1]);
title('СПМ белого шума');
xlabel('Частота (Гц)');

% Параметры для pwelch
window = hann(N/10); % Окно Ханна, размер окна = N/10
noverlap = round(length(window)/2); % 50% перекрытие
nfft = 2^nextpow2(length(window)); % Количество точек БПФ

% Инициализация массивов для усреднения
pxx_low_avg = zeros(nfft/2 + 1, 1); % СПМ имеет размер nfft/2 + 1
pxx_pf_avg = zeros(nfft/2 + 1, 1);
acf_low_avg = zeros(2*N-1, 1);
acf_pf_avg = zeros(2*N-1, 1);

% Усреднение АКФ и СПМ по 1000 реализациям
for i = 1:num_realizations
    noise = randn(N, 1);
    
    % Фильтрация
    sig_low = filter(b_low, a_low, noise);
    sig_pf = filter(b_pf, a_pf, noise);
    
    % АКФ
    acf_low = xcorr(sig_low, 'unbiased');
    acf_pf = xcorr(sig_pf, 'unbiased');
    
    acf_low_avg = acf_low_avg + acf_low;
    acf_pf_avg = acf_pf_avg + acf_pf;
    
    % СПМ
    [pxx_low, f] = pwelch(sig_low, window, noverlap, nfft, Fs);
    [pxx_pf, ~] = pwelch(sig_pf, window, noverlap, nfft, Fs);
    
    pxx_low_avg = pxx_low_avg + pxx_low;
    pxx_pf_avg = pxx_pf_avg + pxx_pf;
end

% Нормализация
acf_low_avg = acf_low_avg / num_realizations;
acf_pf_avg = acf_pf_avg / num_realizations;
pxx_low_avg = pxx_low_avg / num_realizations;
pxx_pf_avg = pxx_pf_avg / num_realizations;

% Графики усредненных АКФ
figure;
subplot(2,1,1);
plot(lags_sec, acf_low_avg);
xlim([0, 0.005]);
title('Усредненная АКФ после ФНЧ');
xlabel('Время (с)');

subplot(2,1,2);
plot(lags_sec, acf_pf_avg);
xlim([0, 0.005]);
title('Усредненная АКФ после ПФ');
xlabel('Время (с)');

% Графики усредненных СПМ
figure;
subplot(2,1,1);
plot(f, 10*log10(pxx_low_avg));
title('Усредненная СПМ после ФНЧ');
xlabel('Частота (Гц)');
ylabel('СПМ (дБ/Гц)');

subplot(2,1,2);
plot(f, 10*log10(pxx_pf_avg));
title('Усредненная СПМ после ПФ');
xlabel('Частота (Гц)');
ylabel('СПМ (дБ/Гц)');

% Интервал корреляции для ФНЧ
acf_low_norm = acf_low_avg / max(acf_low_avg);
threshold = 0.1; % Порог
index = find(acf_low_norm < threshold, 1);
if ~isempty(index)
    correlation_interval = lags_sec(index);
    disp(['Интервал корреляции: ', num2str(correlation_interval), ' с']);
end

% Создаем папку images, если она не существует
if ~exist('images', 'dir')
    mkdir('images');
end

% Сохранение графиков
saveas(figure(1), 'images/white_noise_and_filtered_signals.png'); % Графики сигналов
saveas(figure(2), 'images/acf_and_psd_of_white_noise.png'); % АКФ и СПМ белого шума
saveas(figure(3), 'images/averaged_acf.png'); % Усредненные АКФ
saveas(figure(4), 'images/averaged_psd.png'); % Усредненные СПМ