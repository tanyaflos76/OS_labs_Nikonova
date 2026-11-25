#include "library.h"
#include <stdio.h>
#include <dlfcn.h>

int load_lib(const char *path, void **handle, gcd_func **my_gcd, pi_func **my_pi)
{
    void *new_handle = dlopen(path, RTLD_LAZY);
    if (new_handle == NULL)
    {
        printf("Loadind of library failed");
        return 1;
    }
    dlerror();
    gcd_func *new_gcd = (gcd_func *)dlsym(new_handle, "gcd");
    const char *error = dlerror();
    if (error != NULL)
    {
        printf("Loading of GCD function failed");
        dlclose(new_handle);
        return 1;
    }
    dlerror();
    pi_func *new_pi = (pi_func *)dlsym(new_handle, "pi");
    error = dlerror();
    if (error != NULL)
    {
        printf("Loading of pi function failed");
        dlclose(new_handle);
        return 1;
    }

    if (*handle != NULL)
    {
        dlclose(*handle);
    }
    *handle = new_handle;
    *my_gcd = new_gcd;
    *my_pi = new_pi;
    return 0;
}

int main()
{
    void *handle = NULL;
    gcd_func *my_gcd = NULL;
    pi_func *my_pi = NULL;
    int current_library = 1;
    if (load_lib("./libfirst.so", &handle, &my_gcd, &my_pi) != 0)
    {
        return 1;
    }
    int command;
    int status;
    while ((status = scanf("%d", &command)) != EOF)
    {
        if (status != 1)
        {
            printf("Invalid input\n");
            dlclose(handle);
            return 1;
        }
        if (command == 0)
        {
            int next_library;
            const char *path;
            if ((current_library == 1))
            {
                next_library = 2;
            }
            else
            {
                next_library = 1;
            }
            if (next_library == 1)
            {
                path = "./libfirst.so";
            }
            else
            {
                path = "./libsecond.so";
            }

            if (load_lib(path, &handle, &my_gcd, &my_pi) != 0)
            {
                dlclose(handle);
                return 1;
            }
            current_library = next_library;
            printf("Switched to library %d\n", current_library);
        }
        else if (command == 1)
        {
            int a, b;
            if (scanf("%d %d", &a, &b) != 2)
            {
                printf("Invalid input\n");
                dlclose(handle);
                return 1;
            }
            else if (a < 1 || b < 1)
            {
                printf("Argument must be positive integers\n");
                dlclose(handle);
                return 1;
            }
            else
            {
                printf("GCD for numbers %d and %d is %d\n", a, b, my_gcd(a, b));
            }
        }
        else if (command == 2)
        {
            int k;
            if (scanf("%d", &k) != 1)
            {
                printf("Invalid input\n");
                dlclose(handle);
                return 1;
            }
            else if (k <= 0)
            {
                printf("Argument must be positive\n");
                dlclose(handle);
                return 1;
            }
            else
            {
                printf("Value of π for a given series length = %d: %.6f\n", k, my_pi(k));
            }
        }
        else
        {
            printf("Unknown command\n");
            dlclose(handle);
            return 1;
        }
    }
    dlclose(handle);
    return 0;
}
