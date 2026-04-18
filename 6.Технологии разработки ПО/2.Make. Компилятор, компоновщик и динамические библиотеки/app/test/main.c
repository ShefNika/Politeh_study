#include <stdio.h>
#include "funcs.h"

int main(void)
{
    int result = 0;

    print_hello();

    result = add_numbers(10, 20);
    printf("10 + 20 = %d\n", result);

    return 0;
}