#include <npc.h>
#include <klib-macros.h>

static const char mainargs[] = MAINARGS;
int main(const char *args);
// void putch(char ch) {
// }

void halt(int code) {
  asm volatile("mv a0, %0; .word 0xdead10cc" ::"r"(code));
  while (1);
}

void _trm_init() {
  int ret = main(mainargs);
  halt(ret);
}
