#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  sleep(5);
  __asm__("udf #0");
  printf("survived!\n");
  return 0;
}
