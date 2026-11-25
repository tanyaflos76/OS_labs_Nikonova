#include "library.h"
#include <stdio.h>

int main()
{
    int command;
    int status;
    while ((status = scanf("%d", &command)) != EOF)
    {
        if (status != 1)
        {
            printf("Invalid input\n");
            return 1;
        }
        if (command == 1)
        {
            int a, b;
            if (scanf("%d %d", &a, &b) != 2)
            {
                printf("Invalid input\n");
                return 1;
            }
            else if (a < 1 || b < 1)
            {
                printf("Argument must be positive integers\n");
                return 1;
            }
            else
            {
                printf("GCD for numbers %d and %d is %d\n", a, b, gcd(a, b));
            }
        }
        else if (command == 2)
        {
            int k;
            if (scanf("%d", &k) != 1)
            {
                printf("Invalid input\n");
                return 1;
            }
            else if (k <= 0)
            {
                printf("Argument must be positive\n");
                return 1;
            }
            else
            {
                printf("Value of π for a given series length = %d: %.6f\n", k, pi(k));
            }
        }
        else
        {
            printf("Unknown command\n");
            return 1;
        }
    }
    return 0;
}