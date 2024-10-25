%% 1. Генерация матрицы переходов
P = [0.8, 0.1, 0.1, 0;   
     0.2, 0.7, 0.1, 0;
     0.1, 0.2, 0.5, 0.2;
     0.2, 0.2, 0.2, 0.4];

disp('Сумма каждой строки матрицы P:');
disp(sum(P, 2));

%% 2. Создание цепи Маркова
states = ["Healthy", "Unwell", "Sick", "Very sick"];
MC = dtmc(P, 'StateNames', states);

%% 3. Вывод матрицы переходов
disp('Матрица переходов цепи Маркова:');
disp(MC.P);

%% 4. Построение графа
figure;
graphplot(MC, 'ColorEdges', true);
title('Граф цепи Маркова');

%% 5. Кумулятивная матрица переходов
P_cum = cumsum(P, 2);
disp('Кумулятивная матрица переходов:');
disp(P_cum);

%% 6. Моделирование поведения цепи Маркова
iterations = 200;
z = zeros(1, iterations);
z(1) = 1;

for t = 1:iterations-1
    r = rand;
    z(t+1) = find(r < P_cum(z(t), :), 1);
end

%% 7. Построение графика изменения состояний
figure;
plot(z, 'LineWidth', 2);
title('Поведение цепи Маркова (200 итераций)');
xlabel('Номер итерации');
ylabel('Состояние');
yticks(1:4);
yticklabels(states);
grid on;

%% 8. Моделирование для 1000 и 10000 итераций
iterations_large = [1000, 10000];
for iter = iterations_large
    z_large = zeros(1, iter);
    z_large(1) = 1;

    for t = 1:iter-1
        r = rand;
        z_large(t+1) = find(r < P_cum(z_large(t), :), 1);
    end

    figure;
    plot(z_large, 'LineWidth', 2);
    title(['Поведение цепи Маркова (', num2str(iter), ' итераций)']);
    xlabel('Номер итерации');
    ylabel('Состояние');
    yticks(1:4);
    yticklabels(states);
    grid on;
end

%% 9. Оценка матрицы переходов для 200, 1000 и 10000 итераций
z_all = {z, z_large};
for idx = 1:length(z_all)
    current_z = z_all{idx};
    P_obs = zeros(4);

    for t = 1:length(current_z)-1
        P_obs(current_z(t), current_z(t+1)) = P_obs(current_z(t), current_z(t+1)) + 1;
    end

    P_obs = P_obs ./ sum(P_obs, 2);
    disp(['Матрица переходов по наблюдениям для ', num2str(length(current_z)), ' итераций:']);
    disp(P_obs);

    figure;
    MC_obs = dtmc(P_obs, 'StateNames', states);
    graphplot(MC_obs, 'ColorEdges', true);
    title(['Граф наблюдаемой цепи (', num2str(length(current_z)), ' итераций)']);
end

%% 10. Сравнение результатов

%% 12. Системы массового обслуживания
lambda = 5;
mu = 1;
n = 1;
m = 5;

rho = lambda / mu;
p0 = 1 / (sum((rho.^[0:n-1]) ./ factorial(0:n-1)) + (rho^n / factorial(n)) * sum((rho / n).^[1:m]));

P_otk = (rho^n / factorial(n)) * (rho / n)^m * p0;
Q = 1 - P_otk;
A = lambda * Q;
k_zan = A / mu;
L_och = (rho^n / factorial(n)) * (rho / n)^(m+1) * p0;
L_sist = L_och + k_zan;
T_sist = L_sist / lambda;
T_och = L_och / lambda;

disp('Результаты для системы массового обслуживания:');
fprintf('Вероятность отказа: %.4f\n', P_otk);
fprintf('Относительная пропускная способность: %.4f\n', Q);
fprintf('Абсолютная пропускная способность: %.4f\n', A);
fprintf('Среднее количество занятых каналов: %.4f\n', k_zan);
fprintf('Средняя длина очереди: %.4f\n', L_och);
fprintf('Среднее количество заявок в системе: %.4f\n', L_sist);
fprintf('Среднее время пребывания заявки в системе: %.4f\n', T_sist);
fprintf('Среднее время ожидания заявки в очереди: %.4f\n', T_och);
