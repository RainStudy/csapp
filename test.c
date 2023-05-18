#include <stdio.h>

int main() {
  char op;
  unsigned long address;
  int size;
  scanf(" %c %lx,%d", &op, &address, &size);
  printf("%c %lx %d\n", op, address, size);
  return 0;
}
