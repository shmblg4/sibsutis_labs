%% Лаба 4-1

% Определяем функцию одной переменной
f = @(x) sin(x).^2 - exp(-2.*x);

% Строим график функции f(x)
fplot(f, [-2, 2]);
grid on;
title('График функции f(x)');
xlabel('x');
ylabel('f(x)');

% Производная функции f(x)
syms x;
f_sym = sin(x)^2 - exp(-2*x);
df = diff(f_sym, x);  % Вычисляем производную

% Строим график производной f'(x)
figure;
fplot(matlabFunction(df), [-2, 2]);
grid on;
title('График производной f''(x)');
xlabel('x');
ylabel('f''(x)');

% Интеграл функции f(x)
int_f = int(f_sym, x);  % Вычисляем интеграл

% Строим график интеграла ∫f(x)dx
figure;
fplot(matlabFunction(int_f), [-2, 2]);
grid on;
title('График интеграла ∫f(x)dx');
xlabel('x');
ylabel('∫f(x)dx');

% Определяем функцию двух переменных F(x, y)
F = @(x, y) sqrt(x.^2 - y.^2) ./ y;

% Определяем сетку значений для x и y
[x_grid, y_grid] = meshgrid(-2:0.1:2, -2:0.1:2);

% Вычисляем значения функции на сетке с проверкой области определения
z = F(x_grid, y_grid);

% Игнорируем комплексные значения, оставляем только вещественные
z(imag(z) ~= 0) = NaN;  % Заменяем комплексные значения на NaN

% Строим график поверхности
figure;
surf(x_grid, y_grid, real(z));  % Строим график только для вещественных значений
grid on;
title('График функции F(x, y)');
xlabel('x');
ylabel('y');
zlabel('F(x, y)');
% Определяем функцию одной переменной
f = @(x) sin(x).^2 - exp(-2*x);

% Определяем уравнение a*x + b = f(x)
a = 1;
b = 0;
eqn = @(x) a*x + b - f(x);  % Преобразуем уравнение для решения

% Используем fsolve для численного решения
initial_guess = 0;  % Начальная точка для поиска решения
solution = fsolve(eqn, initial_guess);

% Отображаем решение
disp('Решение уравнения a*x + b = f(x):');
disp(solution);

% Графическое решение
figure;
fplot(f, [-2, 2]); hold on;
fplot(@(x) a*x + b, [-2, 2]);  % Линия a*x + b
plot(solution, a*solution + b, 'ro');  % Отмечаем решение
grid on;
legend('f(x)', 'a*x + b', 'Решение');
title('Графическое решение уравнения a*x + b = f(x)');
xlabel('x');

n = 22;
m = 15;

%% Лаба 4-2

function [product, mean_value, variance_value] = lab_task(n, m)
    % Определяем случайный диапазон
    range_min = -10;
    range_max = 10;

    % Создаем вектор-столбец n на 1
    vector_column = (range_min + (range_max - range_min) * rand(n, 1));

    % Создаем вектор-строку 1 на m
    vector_row = (range_min + (range_max - range_min) * rand(1, m));

    % Перемножение векторов
    product = vector_column * vector_row;

    % Рассчитываем среднее значение произведения
    mean_value = mean(product(:));

    % Рассчитываем дисперсию произведения
    variance_value = var(product(:));

    % Выводим результаты
    disp('Произведение векторов (матрица):');
    disp(product);

    disp('Среднее значение:');
    disp(mean_value);

    disp('Дисперсия:');
    disp(variance_value);
end

lab_task(n, m);

%% Лаба 4-3
L = 15; % Размерность матрицы
T = zeros(L); % Инициализация матрицы переходов

% Обновление всех элементов матрицы для стохастичности
for i = 1:L
    for j = 1:L
        if i ~= j % Не допускаем переходы в самих себя
            T(i,j) = 1 / (L - 1); % Заполнение ненулевыми значениями так, чтобы сумма строки была 1
        end
    end
end

% Проверка, является ли матрица стохастической
function isStochastic = stochastic(matrix)
    isStochastic = all(abs(sum(matrix, 2) - 1) < 1e-5) && all(all(matrix >= 0));
end

% Проверка, является ли матрица эргодической
function isErgodic = ergodic(matrix, epsilon)
    % Проверка, является ли матрица стохастической
    if ~stochastic(matrix)
        isErgodic = false;
        return;
    end

    % Инициализация переменных
    L = size(matrix, 1);
    distribution = ones(1, L) / L; % Начальное равномерное распределение

    % Итерации для получения предельного распределения
    for iter = 1:1000  % Количество итераций
        new_distribution = distribution * matrix;
        
        % Проверка на сходимость
        if norm(new_distribution - distribution, 1) < epsilon
            break;
        end
        
        distribution = new_distribution;
    end

    % Проверка, сумма значений распределения больше нуля
    isErgodic = all(distribution > 0);
end

% Проверяем стохастичность матрицы
if stochastic(T)
    disp('Матрица переходов является стохастической.');
else
    disp('Матрица переходов не является стохастической.');
end

% Проверяем эргодичность матрицы
epsilon = 1e-5; % Установка значения epsilon
if ergodic(T, epsilon)
    disp('Матрица переходов является эргодической.');
else
    disp('Матрица переходов не является эргодической.');
end
