%% Параметры распределения
f = @(x) 0.25; 
a = 0; 
b = 4;

% Функция распределения
F = @(x) integral(f, a, x, 'ArrayValued', true);
F_inv = @(x) a + (b-a)*x; 

% Генерация выборок
Arr_50 = F_inv(rand(50, 1));
Arr_200 = F_inv(rand(200, 1));
Arr_1000 = F_inv(rand(1000, 1));

%% Расчет точечных оценок
% Для всех выборок
sample_means = [mean(Arr_50), mean(Arr_200), mean(Arr_1000)];
sample_vars = [var(Arr_50), var(Arr_200), var(Arr_1000)];
sample_stds = [std(Arr_50), std(Arr_200), std(Arr_1000)];

disp('Точечные оценки среднего:');
disp(sample_means);
disp('Точечные оценки дисперсии:');
disp(sample_vars);
disp('Точечные оценки СКО:');
disp(sample_stds);

%% Расчет интервальных оценок среднего и дисперсии
alphas = [0.1, 0.05, 0.01];
samples = {Arr_50, Arr_200, Arr_1000};

results_mean_intervals = cell(length(samples), length(alphas));
results_var_intervals = cell(length(samples), length(alphas));

for k = 1:length(samples)
    sample = samples{k};
    N = length(sample);
    
    for i = 1:length(alphas)
        alpha = alphas(i);
        
        % Интервальные оценки среднего
        k_STD = tinv(1 - alpha / 2, N - 1);
        margin_of_error_mean = k_STD * (std(sample) / sqrt(N));
        mean_estimate = mean(sample);
        results_mean_intervals{k, i} = [mean_estimate - margin_of_error_mean, mean_estimate + margin_of_error_mean];
        
        % Интервальные оценки дисперсии
        chiSQLeft = chi2inv((alpha) / 2, N - 1);
        chiSQRight = chi2inv((1 - alpha) / 2, N - 1);
        distrDisp = var(sample) * (N - 1);
        results_var_intervals{k, i} = [distrDisp / chiSQLeft, distrDisp / chiSQRight];
    end
end

disp('Интервальные оценки среднего:');
disp(results_mean_intervals);
disp('Интервальные оценки дисперсии:');
disp(results_var_intervals);

%% Построение гистограмм
k_bins = ceil(1 + 3.2 * log([50, 200, 1000]));
figure;
for k = 1:length(samples)
    subplot(3, 1, k);
    histogram(samples{k}, k_bins(k), 'Normalization', 'pdf');
    hold on;
    fplot(f, [a, b], 'r', 'LineWidth', 2);  % Теоретическая плотность
    title(['Гистограмма с N=' num2str(length(samples{k}))]);
    xlabel('Значения');
    ylabel('Плотность');
    hold off;
end

%% Построение функции распределения
figure;
fplot(f, [a, b], 'r');  % Теоретическая плотность
hold on;
fplot(F, [a, b], 'b');  % Функция распределения
xlabel('x');
ylabel('P(x)');
title('Функция распределения и плотность');
legend('Теоретическая плотность', 'Функция распределения');
hold off;

%% Дискретная случайная величина
k_max = 5;
k_values = 1:k_max;
probabilities = ones(1, k_max) * 0.2; % Пример: равномерное распределение

% Генерация выборок дискретной случайной величины
N = [50, 200, 1000];
discrete_samples = cell(1, length(N));
empirical_probs = cell(1, length(N));

for i = 1:length(N)
    discrete_samples{i} = randsample(k_values, N(i), true, probabilities);
    empirical_probs{i} = histcounts(discrete_samples{i}, 'Normalization', 'probability', 'BinEdges', 0.5:1:(k_max + 0.5));
end

%% Построение графиков для дискретной случайной величины
figure;
for i = 1:length(N)
    subplot(3, 1, i);
    bar(k_values, empirical_probs{i});
    hold on;
    plot(k_values, probabilities, 'r', 'LineWidth', 2);
    hold off;
    title(['Распределение для N=' num2str(N(i))]);
    xlabel('Значение');
    ylabel('Вероятность');
    legend('Эмпирическая', 'Теоретическая');
end