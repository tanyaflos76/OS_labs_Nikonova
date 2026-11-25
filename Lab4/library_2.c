#include "library.h"

int gcd(int a, int b)
{
    int min_el;
    int max_del = -1;
    if (a < b)
    {
        min_el = a;
    }
    else if (b < a)
    {
        min_el = b;
    }
    else
    {
        return a;
    }
    for (int i = 1; i <= min_el; i++)
    {
        if ((a % i == 0) && (b % i == 0))
        {
            max_del = i;
        }
    }
    return max_del;
}

float pi(int k)
{
    double res = 2.0;
    for (int n = 1; n <= k; n++)
    {
        double cur = (4.0 * n * n) / (4.0 * n * n - 1.0);
        res *= cur;
    }
    return (float)res;
}