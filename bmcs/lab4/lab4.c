#include <stdio.h>
#include <math.h>

#define N 5
#define SEQ_LENGTH ((1 << N) - 1)
#define SUM(x, y) ((x + y) % 2)
#define TOLERANCE 0.5 // Допустимая погрешность для автокорреляции

double autocorr(int x, int y, int length)
{
    int parity = 0;
    int disparity = 0;
    for (int i = 0; i < length; i++)
    {
        if (((x >> i) & 1) == ((y >> i) & 1))
            parity++;
        else
            disparity++;
    }
    return (double)(parity - disparity) / length;
}

int Gold_sequence(int x, int y)
{
    int res = 0;

    for (int i = 0; i < SEQ_LENGTH; i++)
    {
        int x_feedback = SUM(((x >> 3) & 1), (x & 2));
        int y_feedback = SUM(((y >> 2) & 1), (y & 3));

        res = (res >> 1) | (SUM((x & 1), (y & 1)) << (SEQ_LENGTH - 1));

        x = (x >> 1) | (x_feedback << (N - 1));
        y = (y >> 1) | (y_feedback << (N - 1));
    }
    return res;
}

void print_binary(int x, int length)
{
    for (int i = length - 1; i >= 0; i--)
        if (x & (1 << i))
            printf("1");
        else
            printf("0");
    printf("\t");
}

int shift_seq(int x, int length)
{
    int bit = (x & 1) << (length - 1);
    return (bit | (x >> 1)) & ((1 << length) - 1);
}

int count_ones(int sequence, int length)
{
    int count = 0;
    for (int i = 0; i < length; i++)
    {
        if ((sequence >> i) & 1)
            count++;
    }
    return count;
}

// Проверка сбалансированности
int check_balance(int sequence, int length)
{
    int ones = count_ones(sequence, length);
    if (abs(ones - (length - ones)) > 1)
    {
        printf("Сбалансированность не выполнена: 1=%d, 0=%d\n", ones, length - ones);
        return 0;
    }
    return 1;
}

int check_cycle_distribution(int seq, int length)
{
    int current_length = 1;
    int last_bit = seq & 1;
    int counts[length];
    for (int i = 0; i < length; i++)
        counts[i] = 0;

    for (int i = 1; i < length; i++)
    {
        int bit = (seq >> i) & 1;
        if (bit == last_bit)
        {
            current_length++;
        }
        else
        {
            if (current_length < length)
                counts[current_length]++;
            current_length = 1;
            last_bit = bit;
        }
    }

    for (int i = 1; i < length; i++)
    {
        if (counts[i] > 0)
            printf("Циклы длиной %d встречаются %d раз(а)\n", i, counts[i]);
    }
    return 1;
}

int check_autocorrelation(int sequence, int length)
{
    for (int shift = 0; shift < length; shift++)
    {
        int shifted_sequence = shift_seq(sequence, length);
        double correlation = autocorr(sequence, shifted_sequence, length);

        if (shift != 0 && fabs(correlation) > TOLERANCE)
        {
            printf("Автокорреляция на сдвиге %d не близка к 0: %lf\n", shift, correlation);
            return 0;
        }
    }
    printf("Автокорреляция соблюдена!\n");
    return 1;
}

// Основная функция проверки
int is_m_sequence(int sequence, int length)
{
    int is_valid = 1;

    printf("Проверка сбалансированности:\n");
    if (!check_balance(sequence, length))
        is_valid = 0;

    printf("Проверка распределения циклов:\n");
    if (!check_cycle_distribution(sequence, length))
        is_valid = 0;

    printf("Проверка автокорреляции:\n");
    if (!check_autocorrelation(sequence, length))
        is_valid = 0;

    return is_valid;
}

int main()
{
    int x = 0b10010;
    int y = 0b01010;
    int seq = Gold_sequence(x, y);
    int b = 0b0101010101010101010101010101010;
    if (is_m_sequence(b, SEQ_LENGTH))
    {
        printf("Это M последовательность\n");
    }
    else
    {
        printf("Это не M последовательость\n");
    }
    int new_seq = seq;
    printf("Сдвиг\tПоследовательность Голда\tАвтокорреляция\n");
    printf("%d\t", 0);
    print_binary(seq, SEQ_LENGTH);
    printf("%lf\n", autocorr(seq, new_seq, SEQ_LENGTH));

    for (int i = 1; i < SEQ_LENGTH + 1; i++)
    {
        printf("%d\t", i);
        new_seq = shift_seq(new_seq, SEQ_LENGTH);
        print_binary(new_seq, SEQ_LENGTH);
        printf("%lf\n", autocorr(seq, new_seq, SEQ_LENGTH));
    }
    printf("\n");
    x += 1;
    y -= 5;
    seq = Gold_sequence(x, y);
    if (is_m_sequence(seq, SEQ_LENGTH))
    {
        printf("Это M последовательность\n");
    }
    else
    {
        printf("Это не M последовательость\n");
    }
    new_seq = seq;
    printf("Сдвиг\tПоследовательность Голда\tАвтокорреляция\n");
    printf("%d\t", 0);
    print_binary(seq, SEQ_LENGTH);
    printf("%lf\n", autocorr(seq, new_seq, SEQ_LENGTH));

    for (int i = 1; i < SEQ_LENGTH + 1; i++)
    {
        printf("%d\t", i);
        new_seq = shift_seq(new_seq, SEQ_LENGTH);
        print_binary(new_seq, SEQ_LENGTH);
        printf("%lf\n", autocorr(seq, new_seq, SEQ_LENGTH));
    }
    return 0;
}