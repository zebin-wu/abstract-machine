#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

struct vsnprint_handle_cb_arg {
  size_t n;
  char *p;
};

static const char digits[] = "0123456789abcdef";
static int locked;

static inline int
handle_uint(uint64_t n, int base, int (*cb)(char ch, void *arg), void *arg) {
  char stack[16];
  int top = 0;
  int len = 0;

  do {
    stack[top++] = digits[n % base];
    len++;
    n /= base;
  } while (n);
  while (top) {
    if (cb(stack[--top], arg)) {
      return -1;
    }
  }
  return len;
}

static inline int
handle_int(int n, int (*cb)(char ch, void *arg), void *arg) {
  int len = 0;
  if (n < 0) {
    if (cb('-', arg)) {
      return -1;
    }
    len++;
    n = -n;
  }
  len += handle_uint(n, 10, cb, arg);
  return len;
}

static inline int
handle_str(const char *s, int (*cb)(char ch, void *arg), void *arg) {
  int len = 0;
  for (; *s; s++) {
    if (cb(*s, arg)) {
      return -1;
    }
    len++;
  }
  return len;
}

static inline int
handle_ptr(uintptr_t ptr, int (*cb)(char ch, void *arg), void *arg) {
  int len = 0;
  if (ptr == 0) {
    return handle_str("(null)", cb, arg);
  }
  len += handle_str("0x", cb, arg);
  for (int i = 0; i < sizeof(uintptr_t) * 2; i++, ptr <<= 4) {
    if (cb(digits[ptr >> (sizeof(uintptr_t) * 8 - 4)], arg)) {
      return -1;
    }
    len++;
  }
  return len;
}

static int
handle_format(const char *fmt, va_list ap, int (*cb)(char ch, void *arg), void *arg) {
  int len = 0;
  int ret;
  bool isfmt = false;

  for (; *fmt; fmt++) {
    char c = *fmt;
    if (isfmt) {
      switch (c) {
        case '%':
          ret = cb('%', arg) == -1 ? -1 : 1;
          break;
        case 'd':
          ret = handle_int(va_arg(ap, int), cb, arg);
          break;
        case 'u':
          ret = handle_uint(va_arg(ap, unsigned), 10, cb, arg);
          break;
        case 's':
          ret = handle_str(va_arg(ap, const char *), cb, arg);
          break;
        case 'c':
          ret = cb(va_arg(ap, int), arg) == -1 ? -1 : 1;
          break;
        case 'x':
          ret = handle_uint(va_arg(ap, unsigned int), 16, cb, arg);
          break;
        case 'p':
          ret = handle_ptr(va_arg(ap, uintptr_t), cb, arg);
          break;
        default:
          ret = cb('%', arg) == -1 ? -1 : 1;
          break;
      }
      if (ret == -1) {
        return len;
      }
      len += ret;
      isfmt = false;
    } else if (c == '%') {
      isfmt = true;
    } else {
      if (cb(c, arg)) {
        return len;
      }
      len++;
    }
  }

  return len;
}

static int print_handle_cb(char ch, void *arg) {
  putch(ch);
  return 0;
}

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  while (atomic_xchg(&locked, 1));
  int len = handle_format(fmt, ap, print_handle_cb, NULL);
  atomic_xchg(&locked, 0);
  va_end(ap);
  return len;
}

static int vsprint_handle_cb(char ch, void *arg) {
  char **pout = arg;
  char *p = *pout;
  *p = ch;
  *pout = p + 1;
  return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  int len = handle_format(fmt, ap, vsprint_handle_cb, &out);
  *out = '\0';
  return len;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int len = vsprintf(out, fmt, ap);
  va_end(ap);
  return len;
}

static int vsnprint_handle_cb(char ch, void *_arg) {
  struct vsnprint_handle_cb_arg *arg = _arg;
  if (arg->n == 1) {
    return -1;
  }
  *((arg->p)++) = ch;
  (arg->n)--;
  return 0;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int len = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return len;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  struct vsnprint_handle_cb_arg arg = {
    .p = out,
    .n = n,
  };
  int len = handle_format(fmt, ap, vsnprint_handle_cb, &arg);
  *(arg.p) = '\0';
  return len;
}

#endif
