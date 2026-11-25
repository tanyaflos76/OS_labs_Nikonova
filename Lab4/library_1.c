#include "library.h"

int gcd(int a, int b)
{
    while (b != 0)
    {
        int ost = a % b;
        a = b;
        b = ost;
    }
    return a;
}

float pi(int k)
{
    double res = 0.0;
    for (int n = 0; n < k; n++)
    {
        double cur = 1.0 / (2.0 * n + 1.0);
        if (n % 2 == 0)
        {
            res += cur;
        }
        else
        {
            res -= cur;
        }
    }
    return (float)(4.0 * res);
}