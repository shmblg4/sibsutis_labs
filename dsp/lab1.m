W = @(x, sigma2, mx) 1 ./ sqrt(2 .* pi .* sigma2) .* exp((-(x - mx) .^ 2) ./ (2 .* sigma2));

x = -5:0.01:5;
mx = [0, 0, 0, -1];
sigma2 = [1, 3, 0.2, 1];

figure;
for i = 1:4
    subplot(2, 2, i);
    plot(x, W(x, sigma2(i), mx(i)));
    title(sprintf('W(x, %f, %f)', sigma2(i), mx(i)));
end

s1 = sqrt(sigma2);
disp(s1);

t = 0:3/10000:3;

figure;
for i = 1:4
    subplot(2, 2, i);
    X = randn(length(t), 1);    
    Z = mx(i) + sqrt(sigma2(i)) * X;
    
    real_mx = mean(Z);
    real_sigma2 = var(Z);

    W_func = @(x) W(x, sigma2(i), mx(i));
    mxW = integral(@(x) x .* W_func(x), -Inf, Inf);
    sigma2W = integral(@(x) x.^2 .* W_func(x), -Inf, Inf) - mxW^2;
    
    disp(['Для mx = ', num2str(mx(i)), ', sigma2 = ', num2str(sigma2(i))]);
    disp(['Реальное матожидание (выборочное): ', num2str(real_mx)]);
    disp(['Реальное матожидание (интеграл): ', num2str(mxW)]);
    disp(['Реальная дисперсия (выборочная): ', num2str(real_sigma2)]);
    disp(['Реальная дисперсия (интеграл): ', num2str(sigma2W)]);
    disp('---------------------------------------------');
    
    histogram(Z, 'Normalization', 'pdf', 'BinWidth', 0.1, 'EdgeAlpha', 0);
    hold on;
    
    plot(x, W(x, sigma2(i), mx(i)), 'LineWidth', 2, 'Color', 'r');
    title(sprintf('mx = %f, sigma2 = %f', mx(i), sigma2(i)));
    legend('Гистограмма', 'Теоретическая плотность');
    hold off;
end