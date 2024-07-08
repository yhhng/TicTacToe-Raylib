#include <stdio.h>
int main()
{
    int i, n;
    double factorial;
    /*get the input*/
    printf("\n Enter a positive integer: ");
    scanf("%d", &n);
    /*compute factorial*/
    factorial = 1.;
    for (i = 1; i <= n; ++i)
    {
        factorial *= i;
    }
    /*print result*/
    printf("\n %d factorial is %0.1f\n", n, factorial);
    return 0;
}