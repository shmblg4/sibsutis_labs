fs = 1000;
f = 5;
t = 0:1/fs:1;

y = @(t) 2 * sin(2 * pi * f * t + (pi / 6));

%% График сигнала
subplot(2, 1, 1);
plot(t, y(t));
xlabel("Время t");
ylabel("Амплитуда");
grid on;
title("График y(t) сигнала");

%% Минимальная частота дискретизации по теореме Котельникова
Fs = 2 * f;
t = 0:1/Fs:1;

%% График дискретизированного сигнала
samples = y(t);
subplot(2, 1, 2);
plot(t, samples);
xlabel("Время t");
ylabel("Амплитуда");
xticks(t);
grid on;
title("Дискретизированный сигнал");
uiwait;

%% Дискретное ПФ
F = fft(samples);
stem(abs(F));
hold on;
plot(abs(F));
hold off;
xlabel("Частоты");
ylabel("Амплитуда");
grid on;
title("Спектр сигнала");
uiwait;

fprintf("Ширина спектра: %.2f\n", max(abs(F(:))));
fprintf("Занято памяти: %.2f Кб\n", whos('F').bytes / 1024);

%% Восстановленный сигнал
restored_F = ifft(F);
subplot(2, 1, 1);
plot(t, restored_F);
xlabel('Время');
ylabel('Амплитуда');
title('Восстановленный сигнал');
grid on;
subplot(2, 1, 2);
plot(t, samples);
xlabel('Время');
ylabel('Амплитуда');
title('Оригинальный сигнал');
grid on;
uiwait;

%% Увеличим частоту дискретизации в 4 раза
Fs = 4 * 2 * f;
t = 0:1/Fs:1;

samples = y(t);

t_orig = 0:1/fs:1;
subplot(2, 1, 1);
plot(t_orig, y(t_orig));
xlabel("Время t");
ylabel("Амплитуда");
grid on;
title("График y(t) сигнала");

subplot(2, 1, 2);
plot(t, samples);
xlabel("Время t");
ylabel("Амплитуда");
xticks(t);
grid on;
title("Дискретизированный сигнал");
uiwait;

F = fft(samples);
stem(abs(F));
hold on;
plot(abs(F));
hold off;
xlabel("Частоты");
ylabel("Амплитуда");
grid on;
title("Спектр сигнала");
uiwait;

fprintf("Ширина спектра: %.2f\n", max(abs(F(:))));
fprintf("Занято памяти: %.2f Кб\n", whos('F').bytes / 1024);

restored_F = ifft(F);
subplot(2, 1, 1);
plot(t, restored_F);
xlabel('Время');
ylabel('Амплитуда');
title('Восстановленный сигнал');
grid on;
subplot(2, 1, 2);
plot(t, samples);
xlabel('Время');
ylabel('Амплитуда');
title('Оригинальный сигнал');
grid on;
uiwait;

%% Работа с аудиофайлом
[y_sound, Fs] = audioread('voice.mp3');
fprintf('Расчет частоты дискредитации = %.2f\n', length(y_sound) / 3.384);
fprintf('Реальная частота дискредитации = %.2f\n', Fs);

%% Проряженный аудиофайл в 10 раз

y_downsampled = downsample(y_sound, 10);
Fs_downsampled = Fs / 10;

audio_player = audioplayer(y_downsampled, Fs_downsampled);
play(audio_player);

%% Дискретное ПФ звукового сигнала
fft_y = fft(y_sound);
fft_y_downsampled = fft(y_downsampled);

y_frequency = (0 : length(y_sound)-1);
y_amplitude = abs(fft_y) / length(y_sound);

y_frequency_downsampled = (0 : length(y_downsampled)-1);
y_amplitude_downsampled = abs(fft_y_downsampled) / length(y_downsampled);
plot(y_frequency, y_amplitude, 'DisplayName', 'Оригинальный сигнал')
hold on;
plot(y_frequency_downsampled, y_amplitude_downsampled, 'DisplayName', 'Прореженный в 10 раз сигнал')
hold off;
legend('show')
grid on;
xlabel('Частота');
ylabel('Амплитуда');
uiwait;

%% Функция для округления отсчетов сигнала
function quantization_test(y)
    fs = 1000;           % Частота дискретизации
    t = 0:1/fs:1;       % Временной вектор

    y_orig = y(t);      % Оригинальный сигнал

    bit_depths = [3, 4, 5, 6]; % Разрядности для квантования

    for bits = bit_depths
        % Квантование сигнала
        quantized_signal = quantize(y_orig, bits);
        
        % Прямое преобразование Фурье
        Y = fft(y_orig);
        Y_quantized = fft(quantized_signal);
        
        % Вычисление амплитудного спектра
        amplitude_spectrum = abs(Y);
        amplitude_spectrum_quantized = abs(Y_quantized);
        
        % Ошибка квантования
        quantization_error = y_orig - quantized_signal;
        mean_quantization_error = mean(abs(quantization_error));
        
        % Частотный вектор
        frequencies = (0:length(y_orig)-1);
        
        % Построение графика
        figure;
        plot(frequencies, amplitude_spectrum, 'b', 'DisplayName', 'Оригинальный сигнал');
        hold on;
        plot(frequencies, amplitude_spectrum_quantized, 'r', 'DisplayName', sprintf('Квантованный сигнал (биты = %d)', bits));
        hold off;
        title(sprintf('Сравнение амплитудных спектров для %d бит', bits));
        xlabel('Частота (Гц)');
        ylabel('Амплитуда');
        legend show;
        grid on;
        
        fprintf('Средняя ошибка квантования для %d бит: %.4f\n', bits, mean_quantization_error);
    end
end

function quantized_signal = quantize(signal, bits)
    % Определение параметров квантования
    levels = 2^bits;                      % Количество уровней квантования
    min_value = min(signal);              % Минимальная амплитуда
    max_value = max(signal);              % Максимальная амплитуда
    
    % Масштабирование сигнала к диапазону уровней
    scaled_signal = (signal - min_value) / (max_value - min_value) * (levels - 1);
    
    % Квантование
    quantized_scaled_signal = round(scaled_signal); 
    quantized_scaled_signal(quantized_scaled_signal >= levels) = levels - 1; % Ограничение уровня
    
    % Обратное масштабирование
    quantized_signal = quantized_scaled_signal / (levels - 1) * (max_value - min_value) + min_value; 
end

quantization_test(y);