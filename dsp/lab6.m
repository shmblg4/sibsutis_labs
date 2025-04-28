%% --- ЧАСТЬ 1: BPSK-передача, шум, корреляция и демодуляция ---
numBits = 48;         % Количество битов для передачи
fc = 1000;            % Частота несущей (Гц)
fs = 10000;           % Частота дискретизации (Гц)
T = 1/fs;             % Период дискретизации
Tsym = 1/1000;        % Длительность символа (сек)
sps = Tsym/T;         % Отсчетов на символ
t = 0:T:(numBits/2*Tsym-T); % Временная шкала
SNR = 10;             % Соотношение сигнал/шум

% Генерация случайных данных
data = randi([0 1], 1, numBits);
signal_levels = data .* 2 - 1;

% Формирование сигнала
signal = [];
for i = 1:numBits
    signal = [signal, repelem(signal_levels(i), sps)];
end

% Добавление шума
signal_noisy = awgn(signal, SNR);

% Графики
figure("Position", [100, 100, 1000, 600]);
subplot(3,1,1);
plot(signal);
ylim([-2, 2]);
title('Передаваемый сигнал');
subplot(3,1,2);
plot(signal_noisy);
ylim([-2, 2]);
title('Принятый сигнал с шумом');

% Подсчет корреляции
corr = zeros(1, numBits);
corr_elem = 0;
for i = 1:length(signal_noisy)
    corr_elem = corr_elem + signal_noisy(i) * 1;
    corr = [corr, corr_elem];
    if mod(i, sps) == 0
        corr_elem = 0;
    end
end
corr = corr((length(corr) - length(signal_noisy)):end);

subplot(3,1,3);
plot(corr);
title('Корреляция');
saveas(gcf, 'images/lab6_1.png');

% Демодуляция 
demod = zeros(1, numBits);
for i = 1:numBits
    if corr(i * sps) > 0
        demod(i) = 1;
    else
        demod(i) = 0;
    end
end

% Проверка результата демодуляции
if (demod == data)
    fprintf('Демодуляция прошла успешно\n');
else
    fprintf('Демодуляция провалена\n');
end

%% --- ЧАСТЬ 2: BER сравнение BPSK и ASK ---
fprintf('\n--- Расчёт BER для BPSK и ASK при разных SNR ---\n');
N = 10000;
EbN0_dB = 0:2:10;
data_ber = randi([0 1], 1, N);
bpsk = 2 * data_ber - 1;
ask = data_ber;

ber_bpsk_sim = zeros(size(EbN0_dB));
ber_ask_sim = zeros(size(EbN0_dB));
ber_bpsk_theo = zeros(size(EbN0_dB));
ber_ask_theo = zeros(size(EbN0_dB));

for i = 1:length(EbN0_dB)
    EbN0 = 10^(EbN0_dB(i)/10);
    
    % BPSK
    sigma_bpsk = sqrt(1 / (2 * EbN0));
    noise_bpsk = sigma_bpsk * randn(1, N);
    received_bpsk = bpsk + noise_bpsk;
    decoded_bpsk = received_bpsk > 0;
    ber_bpsk_sim(i) = sum(decoded_bpsk ~= data_ber) / N;
    ber_bpsk_theo(i) = qfunc(sqrt(2 * EbN0));
    
    % ASK
    sigma_ask = sqrt(0.5 / (2 * EbN0));
    noise_ask = sigma_ask * randn(1, N);
    received_ask = ask + noise_ask;
    decoded_ask = received_ask > 0.5;
    ber_ask_sim(i) = sum(decoded_ask ~= data_ber) / N;
    ber_ask_theo(i) = qfunc(sqrt(EbN0));
end

% График BER
figure;
semilogy(EbN0_dB, ber_bpsk_sim, 'bo-', EbN0_dB, ber_bpsk_theo, 'b--', ...
         EbN0_dB, ber_ask_sim, 'rs-', EbN0_dB, ber_ask_theo, 'r--');
grid on; legend('BPSK сим', 'BPSK теор', 'ASK сим', 'ASK теор');
xlabel('Eb/N0 (дБ)'); ylabel('BER'); title('BER для BPSK и ASK');
saveas(gcf, 'images/lab6_2.png');

%% --- ЧАСТЬ 3: Согласованный фильтр и Баркер-коды ---
fprintf('\n--- Согласованный фильтр и Баркер-коды ---\n');
barker = [+1 +1 +1 -1 -1 +1 -1];
N_b = length(barker);
h = fliplr(barker);  % ИХ согласованного фильтра

% Корреляция с разными сигналами
s_matched = barker;
s_inverted = -barker;
s_mismatched = sign(randn(1, N_b));

corr_matched = xcorr(s_matched, barker);
corr_inverted = xcorr(s_inverted, barker);
corr_mismatched = xcorr(s_mismatched, barker);
lags = -N_b+1:N_b-1;

figure;
plot(lags, corr_matched, 'b-', lags, corr_inverted, 'r--', lags, corr_mismatched, 'g:');
legend('Совпадающий', 'Инвертированный', 'Несовпадающий');
title('Сигнал на выходе согласованного фильтра');
xlabel('Сдвиг'); ylabel('Амплитуда'); grid on;
saveas(gcf, 'images/lab6_3.png');

% Шумовое искажение
SNR_list = [20, 10, 5, 0];
signal_clean = barker;
signal_power = mean(signal_clean.^2);
figure;
hold on;
for SNR = SNR_list
    snr_lin = 10^(SNR/10);
    noise_power = signal_power / snr_lin;
    noise = sqrt(noise_power) * randn(1, N_b);
    noisy_signal = signal_clean + noise;
    y_corr = xcorr(noisy_signal, barker);
    plot(lags, y_corr, 'DisplayName', sprintf('SNR = %d дБ', SNR));
end
plot(lags, corr_matched, 'k--', 'DisplayName', 'Без шума');
legend; title('Влияние шума на Баркер-код'); xlabel('Сдвиг'); ylabel('Корреляция'); grid on;
saveas(gcf, 'images/lab6_4.png');