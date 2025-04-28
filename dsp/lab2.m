clc; clear; close all;

% Количество выборок
M = 1000; 
N = 100; % Число случайных величин в сумме

% Генерация равномерно распределенных случайных величин
xn = rand(N, M);

% Гистограмма распределения xn
figure;
histogram(xn(:), 'Normalization', 'pdf');
title('Гистограмма распределения X_n');
xlabel('Значение');
ylabel('Плотность вероятности');
grid on;

% Вычисление суммы
Yn = sum(xn, 1);

% Гистограмма распределения Yn
figure;
histogram(Yn, 'Normalization', 'pdf');
title('Гистограмма распределения Y_n');
xlabel('Значение');
ylabel('Плотность вероятности');
grid on;

% Параметры нормального распределения
m1 = 0; 
s1 = 1; 
N = 100; % Размер каждой реализации
M = 1000; % Количество реализаций

% Генерация нормальных случайных величин
xn = m1 + s1 * randn(N, M);

% Фильтрация для получения коррелированного СП
h = [1, 0.7, 0.3, 0.1, 0.05]; % Коэффициенты фильтрации
xn1 = filter(h, 1, xn);

% Выбор случайного времени t0
t0_idx = randi(N);

% Формирование выборки значений в момент t0
slice_values = xn1(t0_idx, :);

% Временная диаграмма порожденной реализации
figure;
plot(xn1(:, 1:5)); % Отобразим 5 примеров реализаций
title('Временная диаграмма порожденных реализаций');
xlabel('Время');
ylabel('Значение СП');
grid on;

% Гистограмма значений в момент времени t0
figure;
histogram(slice_values, 'Normalization', 'pdf');
title('Гистограмма значений СП в момент времени t_0');
xlabel('Значение');
ylabel('Плотность вероятности');
grid on;

% Число реализаций
M = 1000;      
% Длина каждой реализации
N = 200;       
% Параметры белого шума (нормальное распределение)
mu = 0;        
sigma = 1;     

% Параметры "фильтра" (задаёт коррелированность)
b = [1, 0.8, 0.4]; 
a = 1;

X = sigma * randn(N, M) + mu;
Xf = filter(b, a, X);

maxLag = N-1;   % Максимальное значение tau, которое можно рассматривать
Rxx = zeros(maxLag+1,1);  % Вектор для АКФ (tau от 0 до maxLag)

for tau = 0:maxLag
    kMax = N - tau; 
    sumOverRealizations = 0;
    for m = 1:M
        x_m = Xf(:, m);
        sumOverRealizations = sumOverRealizations + sum(x_m(1:kMax).* x_m(tau+1:tau+kMax));
    end
    Rxx(tau+1) = sumOverRealizations / (M * kMax);
end

Rxx_normalized = Rxx / Rxx(1);

lags = 0:maxLag;
figure;
plot(lags, Rxx_normalized, 'o-','LineWidth',1.2);
title('Выборочная автокорреляционная функция (нормированная)');
xlabel('\tau');
ylabel('R_{xx}(\tau)/R_{xx}(0)');
grid on;

Tcorr = sum(Rxx_normalized);  % упрощённая оценка, без учёта шага по времени
fprintf('Оценка времени корреляции (дискретная сумма): %.2f\n', Tcorr);

