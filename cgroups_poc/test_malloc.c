#include <stdlib.h>
#include <stdio.h>


int main(void)
{
    size_t alloc = 1024 * 1024;

    for (size_t i = 0; ; i++)
    {
        printf("Iteration %d\n", i);

        char* ptr = malloc(alloc);
        if (ptr == NULL)
        {
            fprintf(stderr, "No memory\n");

            return 1;
        }
    }
    return 0;
}
