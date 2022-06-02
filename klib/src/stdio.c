#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

const char digits[] = "0123456789abcdef";

static int printuint(uint64_t n, int base) {
  char stack[16];
  int top = 0;
  int len = 0;

  do {
    stack[top++] = digits[n % base];
    len++;
    n /= base;
  } while (n);
  while (top) {
    putch(stack[--top]);
  }
  return len;
}

static int printint(int n) {
  int len = 0;
  if (n < 0) {
    putch('-');
    len++;
    n = -n;
  }
  len += printuint(n, 10);
  return len;
}

static int printstr(const char *s) {
  int len = 0;
  for (; *s; s++) {
    putch(*s);
    len++;
  }
  return len;
}

static int printptr(uintptr_t ptr) {
  int len = 0;
  if (ptr == 0) {
    return printstr("(null)");
  }
  len += printstr("0x");
  for (int i = 0; i < sizeof(uintptr_t) * 2; i++, ptr <<= 4) {
    putch(digits[ptr >> (sizeof(uintptr_t) * 8 - 4)]);
  }
  return len;
}

int printf(const char *fmt, ...) {
  int len = 0;
  bool isfmt = false;
  va_list ap;

  va_start(ap, fmt);
  for (; *fmt; fmt++) {
    char c = *fmt;
    if (isfmt) {
      switch (c) {
        case '%':
          putch('%');
          len++;
          break;
        case 'd':
          len += printint(va_arg(ap, int));
          break;
        case 'u':
          len += printuint(va_arg(ap, unsigned), 10);
          break;
        case 's':
          len += printstr(va_arg(ap, const char *));
          break;
        case 'c':
          putch(va_arg(ap, int));
          len++;
          break;
        case 'x':
          len += printuint(va_arg(ap, unsigned int), 16);
          break;
        case 'p':
          len += printptr(va_arg(ap, uintptr_t));
          break;
        default:
          putch('%');
          putch(c);
          len += 2;
          break;
      }
      isfmt = false;
    } else if (c == '%') {
      isfmt = true;
    } else {
      putch(c);
      len++;
    }
  }
  va_end(ap);
  return len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
