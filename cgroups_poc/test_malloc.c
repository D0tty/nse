#include <stdlib.h>
#include <stdio.h>


int main(void)
{
    size_t alloc = 1024 * 1024;

    while(1)
    {
        char* ptr = malloc(alloc);
        if (ptr == NULL)
        {
            fprintf(stderr, "No memory\n");

            return 1;
        }
    }
    return 0;
}
