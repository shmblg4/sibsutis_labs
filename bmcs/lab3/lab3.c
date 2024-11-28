#include <stdio.h>
#include <math.h>

#define N 8

// 3.2
double corr(double *x, double *y)
{
    double corr = 0;
    for (int i = 0; i < N; i++)
    {
        corr += *(x + i) * *(y + i);
    }
    return corr;
}

double sum_square(double *x)
{
    double sum = 0;
    for (int i = 0; i < N; i++)
        sum += pow(*(x + i), 2);
    return sum;
}

// 3.3
double normalized_corr(double *x, double *y)
{
    return (corr(x, y)) / sqrt(sum_square(x) * sum_square(y));
}

int main()
{
    double a[N] = {1, 2, 5, -2, -4, -2, 1, 4};
    double b[N] = {3, 6, 7, 0, -5, -4, 2, 5};
    double c[N] = {-1, 0, -3, -9, 2, -2, 5, 1};

    printf("Корреляция между a, b, c\n"
           "\ta\tb\tc\n"
           "a\t-\t%.2f\t%.2f\n"
           "b\t%.2f\t-\t%.2f\n"
           "c\t%.2f\t%.2f\t-\n",
           corr(a, b), corr(a, c), corr(b, a), corr(b, c), corr(c, a), corr(c, b));

    printf("\n===============================\n");
    printf("Нормализованная корреляция между a, b, c\n"
           "\ta\tb\tc\n"
           "a\t-\t%.2f\t%.2f\n"
           "b\t%.2f\t-\t%.2f\n"
           "c\t%.2f\t%.2f\t-\n",
           normalized_corr(a, b), normalized_corr(a, c), normalized_corr(b, a), normalized_corr(b, c), normalized_corr(c, a), normalized_corr(c, b));

    return 0;
}