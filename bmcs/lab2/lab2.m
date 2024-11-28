% Параметры передатчиков и шумов
TxPowerBS = 46; % дБм, мощность передатчика BS
TxPowerUE = 24; % дБм, мощность передатчика пользователя UE
AntGainBS = 21; % дБи, коэффициент усиления антенны BS
PenetrationM = 15; % дБ, запас мощности сигнала на проникновение
IM = 1; % дБ, запас мощности на интерференцию
FR = 1.8; % ГГц, диапазон частот
BW_UL = 10; % МГц, полоса частот в UL
BW_DL = 20; % МГц, полоса частот в DL
NoiseFigure_BS = 2.4; % дБ, коэффициент шума приемника BS
NoiseFigure_UE = 10; % дБ, коэффициент шума приемника UE
RequiredSINR_DL = 2; % дБ, требуемое SINR для DL
RequiredSINR_UL = 4; % дБ, требуемое SINR для UL

% 1. Расчет бюджета восходящего канала
FeederLoss = 2.9; % дБ, потери в фидере
MIMOGain = 3; % дБ, выигрыш от MIMO
ThermalNoise_UL = -174 + 10 * log10(BW_UL * 10e6); % Тепловой шум в UL
RxSensBS = NoiseFigure_BS + ThermalNoise_UL + RequiredSINR_UL; % дБм, чувствительность приемника

MAPL_UL = TxPowerUE - FeederLoss + AntGainBS + MIMOGain - IM - PenetrationM - RxSensBS;
fprintf("MAPL_UL: %f\n", MAPL_UL);

% 2. Расчет бюджета нисходящего канала
ThermalNoise_DL = -174 + 10 * log10(BW_DL * 10e6); % Тепловой шум в DL
RxSensUE = NoiseFigure_UE + ThermalNoise_DL + RequiredSINR_DL; % дБм, чувствительность приемника

MAPL_DL = TxPowerBS - FeederLoss + AntGainBS + MIMOGain - IM - PenetrationM - RxSensUE;
fprintf("MAPL_DL: %f\n", MAPL_DL);

% 3. Модели потерь сигнала
d = linspace(0, 10000, 10000); % расстояние
figure;

% Модель UMiNLOS
PL_UMiNLOS = 26 * log10(FR) + 22.7 + 36.7 * log10(d);
plot(d, PL_UMiNLOS, "DisplayName", "UMiNLOS"); hold on;
xlabel('Расстояние, м');
ylabel('Потери, дБ');
title("Модели");
grid on;

% Модель Окумура-Хата
A = 46.3;
B = 33.9;
hBS = 50; % высота антенны BS
hms = 2; % высота антенны пользователя
a = 3.2 * ((log10(11.75 * hms))^2) - 4.97; % коэффициент

%Т.к. у нас плотная застройка
Lclutter = 3;

% s = (47.88 + 13.9 * log10(FR) - 13.9 * log10(hBS)) * (1 / log10(50));
s = 44.9 - 6.55 * log10(FR);
PL_Oku_Hata = A + B * log10(FR * 10e3) - 13.82 * log10(hBS) - a + s * log10(d / 10e3) + Lclutter;
plot(d, PL_Oku_Hata, 'DisplayName', 'Окумура-Хата');
xlabel('Расстояние, м');
ylabel('Потери, дБ');
grid on;

% Модель Walfish-Ikegami
L_LOS = 42.6 + 20 * log10(FR) + 26 * log10(d);
plot(d, L_LOS, "DisplayName","Модель Walfish-Ikegami (LOS)");
xlabel('Расстояние, м');
ylabel('Потери, дБ');
grid on;

% Параметры для NLOS
h_r = 30; % средняя высота зданий
w = 25; % ширина улиц
b = 30; % расстояние между зданиями

L_0 = 32.44 + 20 * log10(d) + 20 * log10(FR);
L_2 = -18.05 - 10 * log10(w) + 10 * log10(FR) + 20 * log10(hms);
L_1 = 54 + 54 - 0.8 * hBS * (d / 0.5) + 18 * log10(d / 10.^3) + (-4 + 0.7 * ((FR / 925) - 1)) * log10(FR) - 9 * log10(b);


if ((L_1 + L_2) <= 0)
    L_NLOS = L_0;
else
    L_NLOS = L_0 + L_1 + L_2;
end
plot(d, L_NLOS, "DisplayName", "Модель Walfish-Ikegami (NLOS)"); hold off;
xlabel('Расстояние, м');
ylabel('Потери, дБ');
legend show;
grid on;

% 4. Сравнение и вывод результатов
R_1 = 260;
figure;
hold on;
plot(d, PL_UMiNLOS, 'ro-', 'DisplayName', 'UMiNLOS(2.4ГГц)');
yline(MAPL_UL, 'b-', 'DisplayName', 'MAPL_UL=66дБ');
yline(MAPL_DL, 'g--', 'DisplayName', 'MAPL_DL=84дБ');
legend('show');
xline(R_1, 'b-', 'LineWidth', 1);
xline(721, 'g-', 'LineWidth', 1);
xlabel('Расстояние, м');
ylabel('Потери, дБ');
title('Потери сигнала в зависимости от расстояния');
grid on;
hold off;

R_2 = 620;
figure;
hold on;
plot(d, PL_Oku_Hata, 'ro-', 'DisplayName', 'Oku-Hata');
yline(MAPL_UL, 'b-', 'DisplayName', 'MAPL_UL=66дБ');
yline(MAPL_DL, 'g--', 'DisplayName', 'MAPL_DL=84дБ');
legend('show');
xline(R_2, 'b-', 'LineWidth', 1);
xline(1560, 'g-', 'LineWidth', 1);
xlabel('Расстояние, м');
ylabel('Потери, дБ');
title('Потери сигнала в зависимости от расстояния');
grid on;
hold off;

% 5. Вычисление площадей
S_1 = 1.95 * R_1.^2;
S_2 = 1.95 * R_2.^2;
fprintf("Площадь 1: %f\nПлощадь 2: %f\n", S_1, S_2);
