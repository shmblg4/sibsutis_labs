% Параметры
Fs = 16000;        % Частота дискретизации
T = 1;             % Длительность сигнала (сек)
N = Fs * T;        % Количество отсчетов
num_realizations = 1000; % Число реализаций для усреднения
A = 2;             % Амплитуда сигнала

% Генерация белого шума
white_noise = randn(N, 1);

% Проектирование эллиптического ФНЧ
Rp = 0.5;          % Неравномерность в полосе (дБ)
Rs = 50;           % Подавление (дБ)
n = 6;             % Порядок фильтра
Wn_FNCh = 3000 / (Fs/2); % Нормированная частота среза
[b_low, a_low] = ellip(n, Rp, Rs, Wn_FNCh, 'low');

% Проектирование эллиптического ПФ (узкополосный шум)
f1 = 3000; f2 = 6000;
Wp = [f1, f2] / (Fs/2);   % Границы полосы пропускания
Ws = [f1-500, f2+500] / (Fs/2); % Границы полосы задерживания
[n_pf, Wn_pf] = ellipord(Wp, Ws, Rp, Rs);
[b_pf, a_pf] = ellip(n_pf, Rp, Rs, Wn_pf, 'bandpass');

% Генерация сигнала (синусоида)
t = (0:N-1)/Fs;
f_signal = 4500; % Частота сигнала в полосе ПФ (между 3000 и 6000 Гц)
signal = A * cos(2*pi*f_signal*t);

% Фильтрация белого шума
filtered_low = filter(b_low, a_low, white_noise);
filtered_pf = filter(b_pf, a_pf, white_noise); % Узкополосный шум

% Смесь сигнала и узкополосного шума
mixed_signal = signal' + filtered_pf;

% Графики реализаций
figure;
subplot(3,1,1);
plot(t, white_noise);
xlim([0, 0.005]);
title('Белый шум на входе');
subplot(3,1,2);
plot(t, filtered_pf);
xlim([0, 0.005]);
title('Узкополосный шум после ПФ');
subplot(3,1,3);
plot(t, mixed_signal);
xlim([0, 0.005]);
title('Смесь сигнала и узкополосного шума');

% Вычисление огибающей смеси
analytic_signal = hilbert(mixed_signal); % Аналитический сигнал
envelope = abs(analytic_signal); % Огибающая

% Усреднение огибающей и распределения
envelope_avg = zeros(N, 1);
for i = 1:num_realizations
    noise = randn(N, 1);
    filtered_noise = filter(b_pf, a_pf, noise);
    mixed = signal' + filtered_noise;
    envelope_i = abs(hilbert(mixed));
    envelope_avg = envelope_avg + envelope_i;
end
envelope_avg = envelope_avg / num_realizations;

% Эмпирическая функция распределения огибающей
[cdf_vals, R_vals] = ecdf(envelope_avg); % CDF
mean_envelope = mean(envelope_avg); % Математическое ожидание

% График функции распределения с математическим ожиданием
figure;
plot(R_vals, cdf_vals, 'b-', 'LineWidth', 2);
hold on;
plot([mean_envelope mean_envelope], [0 1], 'r--', 'LineWidth', 2);
text(mean_envelope, 0.5, ['E[R] \approx ' num2str(mean_envelope, '%.2f')], ...
    'VerticalAlignment', 'bottom', 'HorizontalAlignment', 'right');
xlabel('Огибающая R');
ylabel('F(R)');
title('Функция распределения огибающей смеси сигнала и узкополосного шума');
grid on;

% АКФ узкополосного процесса (одна реализация)
[acf_pf, lags] = xcorr(filtered_pf, 'unbiased');
lags_sec = lags / Fs;
figure;
plot(lags_sec, acf_pf);
xlim([0, 0.005]);
title('АКФ узкополосного шума (одна реализация)');
xlabel('Время (с)');
ylabel('АКФ');

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

% Графики усреднённых АКФ
figure;
subplot(2,1,1);
plot(lags_sec, acf_low_avg);
xlim([0, 0.005]);
title('Усреднённая АКФ после ФНЧ');
xlabel('Время (с)');
ylabel('АКФ');

subplot(2,1,2);
plot(lags_sec, acf_pf_avg);
xlim([0, 0.005]);
title('Усреднённая АКФ после ПФ');
xlabel('Время (с)');
ylabel('АКФ');

% Графики усреднённых СПМ
figure;
subplot(2,1,1);
plot(f, 10*log10(pxx_low_avg));
title('Усреднённая СПМ после ФНЧ');
xlabel('Частота (Гц)');
ylabel('СПМ (дБ/Гц)');

subplot(2,1,2);
plot(f, 10*log10(pxx_pf_avg));
title('Усреднённая СПМ после ПФ');
xlabel('Частота (Гц)');
ylabel('СПМ (дБ/Гц)');

% Вывод интервала корреляции узкополосного шума
acf_pf_norm = acf_pf_avg / max(acf_pf_avg);
threshold = 0.1;
index = find(abs(acf_pf_norm) < threshold, 1);
if ~isempty(index)
    correlation_interval = lags_sec(index);
    disp(['Интервал корреляции узкополосного шума: ', num2str(correlation_interval), ' с']);
end

% Сохранение графиков
if ~exist('images', 'dir')
    mkdir('images');
end
saveas(figure(1), 'images/signals_with_narrowband_noise.png');
saveas(figure(2), 'images/envelope_cdf.png');
saveas(figure(3), 'images/acf_narrowband_noise.png');
saveas(figure(4), 'images/averaged_acf.png');
saveas(figure(5), 'images/averaged_psd.png');