N = 200;
K = 200;
mu = 2;
sigma = 10;

Xi = mu + sigma * randn(N, K);

ensemble_mean = mean(Xi, 2);

time_mean = mean(Xi, 1);

% График среднего по ансамблю и среднего по времени
figure;
plot(1:N, ensemble_mean, 'r', 'LineWidth', 2); hold on;
plot(1:N, mean(Xi, 2), 'b--', 'LineWidth', 2);
legend('Среднее по ансамблю', 'Среднее по времени');
title('Эргодичность белого шума');
xlabel('n');
ylabel('Значение \xi');
grid on;

%% 2. Диаграммы рассеяния

in_values = [10, 50, 100];
jn_values = [9, 49, 99];

figure;
for i = 1:3
    subplot(1, 3, i);
    scatter(Xi(in_values(i), :), Xi(jn_values(i), :), 'filled');
    title(['Диаграмма рассеяния для n = ', num2str(in_values(i)), ', n = ', num2str(jn_values(i))]);
    xlabel(['\xi_', num2str(in_values(i))]);
    ylabel(['\xi_', num2str(jn_values(i))]);
    grid on;

    % Рассчитываем корреляцию
    corr_value = corrcoef(Xi(in_values(i), :), Xi(jn_values(i), :));
    disp(['Корреляция для n = ', num2str(in_values(i)), ', n = ', num2str(jn_values(i)), ': ', num2str(corr_value(1,2))]);
end

mu_walk = 0;
sigma_walk = 1;

Xi_walk = zeros(N, K);
for k = 1:K
    for n = 2:N
        Xi_walk(n, k) = Xi_walk(n-1, k) + sigma_walk * randn;
    end
end

ensemble_mean_walk = mean(Xi_walk, 2);

figure;
plot(1:N, Xi_walk);
title('Случайные блуждания');
xlabel('n');
ylabel('Значение \xi');
grid on;

figure;
plot(1:N, ensemble_mean_walk, 'r', 'LineWidth', 2);
title('Среднее по ансамблю для случайных блужданий');
xlabel('n');
ylabel('Значение \xi');
grid on;


% Автокорреляция по ансамблю с лагом 1
lag1_autocorr = zeros(1, N-1);
for n = 2:N
    lag1_autocorr(n-1) = mean(Xi_walk(n, :) .* Xi_walk(n-1, :));
end

% Автокорреляция по ансамблю с лагом 10
lag2_autocorr = zeros(1, N-10);
for n = 11:N
    lag2_autocorr(n-10) = mean(Xi_walk(n, :) .* Xi_walk(n-10, :));
end

% Построение автокорреляции для случайных блужданий
figure;
plot(1:N-1, lag1_autocorr, 'b-', 'LineWidth', 2);
hold on;
plot(1:N-10, lag2_autocorr, 'g--', 'LineWidth', 2);
title('Автокорреляция случайных блужданий');
xlabel('n');
ylabel('Автокорреляция');
legend('Лаг 1', 'Лаг 10');
grid on;


damping_factor = 0.9;

% Генерация блужданий с затуханием
Xi_damped = zeros(N, K);
for k = 1:K
    for n = 2:N
        Xi_damped(n, k) = damping_factor * Xi_damped(n-1, k) + sigma_walk * randn;
    end
end


figure;
plot(1:N, Xi_damped);
title('Случайные блуждания с затуханием');
xlabel('n');
ylabel('Значение \xi');
grid on;


lag1_autocorr_damped = zeros(1, N-1);
for n = 2:N
    lag1_autocorr_damped(n-1) = mean(Xi_damped(n, :) .* Xi_damped(n-1, :));
end

lag2_autocorr_damped = zeros(1, N-10);
for n = 11:N
    lag2_autocorr_damped(n-10) = mean(Xi_damped(n, :) .* Xi_damped(n-10, :));
end

figure;
plot(1:N-1, lag1_autocorr_damped, 'r-', 'LineWidth', 2);
hold on;
plot(1:N-10, lag2_autocorr_damped, 'm--', 'LineWidth', 2);
title('Автокорреляция случайных блужданий с затуханием');
xlabel('n');
ylabel('Автокорреляция');
legend('Лаг 1', 'Лаг 10');
grid on;
