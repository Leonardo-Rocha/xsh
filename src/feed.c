#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char const *argv[])
{
  printf("argv[1] = %s", argv[1]);
  char string[50];
  // FILE *file = fopen("input", "r");

  // FILE *out_file = fopen("output", "w+");

  scanf("%s", string);
  printf("\r\n%s + b\r\n ", string);
  perror("this is an error\r\n");
  return 0;
}
