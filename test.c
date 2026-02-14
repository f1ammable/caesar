#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  __asm__("udf #0");
  printf("survived!\n");
  return 0;
}
