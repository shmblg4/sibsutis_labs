f1 = 1;
f2 = 1 + 4;
f3 = 1 * 2 + 1;

a = [1, 2, 5, -2, -4, -2, 1, 4];
b = [3, 6, 7, 0, -5, -4, 2, 5];

t = 0:1/100:1;

s1 = cos(2 * pi * f1 * t);
s2 = cos(2 * pi * f2 * t);
s3 = cos(2 * pi * f3 * t);

at = 2 * s1 + 4 * s2 + s3;
bt = s1 + s2;

corr1 = sum(s1 .* at);
corr2 = sum(s1 .* bt);
fprintf("Корреляция s1 и а(t) = %f\n", corr1);
fprintf("Корреляция s1 и b(t) = %f\n", corr2);

norm_corr1 = corr1 / sqrt(sum(s1 .^ 2) * sum(at .^ 2));
norm_corr2 = corr2 / sqrt(sum(s1 .^ 2) * sum(bt .^ 2));
fprintf("Нормализированнная корреляция s1 и а(t) = %f\n", norm_corr1);
fprintf("Нормализированнная корреляция s1 и b(t) = %f\n", norm_corr2);

a1 = [0.3 0.2 -0.1 4.2 -2 1.5 0];
b1 = [0.3 4 -2.2 1.6 0.1 0.1 0.2];

figure;
subplot(2, 1, 1);
plot(a1, '-o');
xlabel('Индекс');
ylabel('Значение');
title('Массив a');
grid on;

subplot(2, 1, 2);
plot(b1, '-o');
xlabel('Индекс');
ylabel('Значение');
title('Массив b');
grid on;

max_lag = length(b) - 1;
corr_values = zeros(1, max_lag + 1);
lags = -max_lag:max_lag;

for lag = -max_lag:max_lag
    if lag < 0
        shifted_b1 = [zeros(1, -lag), b1(1:end + lag)]; % Сдвиг влево с добавлением нулей
        corr_values(lag + max_lag + 1) = sum(a1 .* shifted_b1);
    else
        shifted_b1 = [b1(lag + 1:end), zeros(1, lag)]; % Сдвиг вправо с добавлением нулей
        corr_values(lag + max_lag + 1) = sum(a1 .* shifted_b1);
    end
end


[max_corr, max_index] = max(corr_values);
optimal_lag = lags(max_index);

fprintf('Максимальная корреляция: %f при сдвиге: %d\n', max_corr, optimal_lag);

figure;
plot(lags, corr_values, '-o');
xlabel('Сдвиг');
ylabel('Взаимная корреляция');
title('Взаимная корреляция a и b');
grid on;

shifted_b1 = circshift(b1, optimal_lag);

figure;
plot(a1, '-o', 'DisplayName', 'a');
hold on;
plot(shifted_b1, '-x', 'DisplayName', sprintf('b сдвинутый на %d', optimal_lag));
xlabel('Индекс');
ylabel('Значение');
title('Массивы a и b (сдвинутый)');
legend show;
grid on;

