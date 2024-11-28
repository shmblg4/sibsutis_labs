N = 5;
SEQ_LENGTH = 2^N - 1;

x = 18;  % 0b10010
y = 10;  % 0b01010

% Функция вычисления автокорреляции
function correlation = autocorr(x, y, length)
parity = 0;
disparity = 0;
for i = 1:length
    if bitget(x, i) == bitget(y, i)
        parity = parity + 1;
    else
        disparity = disparity + 1;
    end
end
correlation = (parity - disparity) / length;
end

% Функция генерации последовательности Голда
function res = Gold_sequence(x, y, N, SEQ_LENGTH)
res = 0;
for i = 1:SEQ_LENGTH
    x_feedback = mod(bitget(x, 4) + bitget(x, 2), 2);
    y_feedback = mod(bitget(y, 3) + bitget(y, 2), 2);
    
    res = bitset(res, SEQ_LENGTH - i + 1, mod(bitget(x, 1) + bitget(y, 1), 2));
    
    x = bitshift(x, -1) + (x_feedback * 2^(N - 1));
    y = bitshift(y, -1) + (y_feedback * 2^(N - 1));
end
end

function shifted = shift_seq(x, length)
shifted = bitshift(x, -1);
shifted = bitor(shifted, bitget(x, 1) * 2^(length - 1));
end

gold_seq = Gold_sequence(x, y, N, SEQ_LENGTH);

auto_corr_values = zeros(1, SEQ_LENGTH);

for shift = 0:SEQ_LENGTH-1
    shifted_seq = bitshift(gold_seq, shift);
    auto_corr_values(shift + 1) = autocorr(gold_seq, shifted_seq, SEQ_LENGTH);
end

disp('Корреляции со сдвигами от 0 до SEQ_LENGTH-1:');
disp(auto_corr_values);

% Визуализация автокорреляции
figure;
stem(0:SEQ_LENGTH-1, auto_corr_values, 'filled');
title('Автокорреляция последовательности Голда для сдвигов от 0 до SEQ_LENGTH-1');
xlabel('Сдвиг');
ylabel('Автокорреляция');
grid on;
