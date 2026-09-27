#include <rz_asm.h>
#include <stdio.h>

int main(void) {
  RzAsm *a = rz_asm_new();
  if (!a) {
    fputs("rz_asm_new failed\n", stderr);
    return 1;
  }
  rz_asm_free(a);
  puts("rz_asm_new OK");
  return 0;
}
