#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void main()
{
    char c;
    dup2(2,1);
    while(1)
    {
        for(int i = 0; i <100 ; i++)
            printf("a");
        scanf("%c",&c);
    }
}