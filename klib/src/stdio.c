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
uint2stack(uint64_t n, int base, char stack[20]) {
  int top = 0;

  do {
    stack[top++] = digits[n % base];
    n /= base;
  } while (n);

  return top;
}

static inline int
handle_padding(char padding, int width, int (*cb)(char ch, void *arg), void *arg) {
  int len = 0;

  for (; width > 0; width--) {
    if (cb(padding, arg)) {
      return -1;
    }
    len++;
  }
  return len;
}

static inline int
handle_uint(uint64_t n, int base, char padding, int width, int (*cb)(char ch, void *arg), void *arg) {
  char stack[20];
  int rc;
  int top = 0;
  int len = 0;

  top = uint2stack(n, base, stack);

  rc = handle_padding(padding, width - top, cb, arg);
  if (rc == -1) {
    return -1;
  }
  len += rc;

  while (top) {
    if (cb(stack[--top], arg)) {
      return -1;
    }
    len++;
  }
  return len;
}

static inline int
handle_int(int64_t n, char padding, int width, int (*cb)(char ch, void *arg), void *arg) {
  char stack[20];
  int rc;
  int top = 0;
  int len = 0;
  int neg = 0;

  if (n < 0) {
    neg = 1;
    n = -n;
  }

  top = uint2stack(n, 10, stack);

  rc = handle_padding(padding, width - top - neg, cb, arg);
  if (rc == -1) {
    return -1;
  }
  len += rc;

  if (neg) {
    if (cb('-', arg)) {
      return -1;
    }
    len++;
  }

  while (top) {
    if (cb(stack[--top], arg)) {
      return -1;
    }
    len++;
  }
  return len;
}

static inline int
handle_str(const char *s, char padding, int width, int (*cb)(char ch, void *arg), void *arg) {
  int rc;
  int len = 0;
  size_t slen = strlen(s);

  rc = handle_padding(padding, width - slen, cb, arg);
  if (rc == -1) {
    return -1;
  }
  len += rc;

  for (; *s; s++) {
    if (cb(*s, arg)) {
      return -1;
    }
    len++;
  }
  return len;
}

static inline int
handle_ptr(uintptr_t ptr, char padding, int width, int (*cb)(char ch, void *arg), void *arg) {
  int rc;
  int len = 0;
  size_t addrlen = sizeof(uintptr_t) * 2;

  if (ptr == 0) {
    return handle_str("(null)", padding, width, cb, arg);
  }

  rc = handle_str("0x", padding, width - addrlen, cb, arg);
  if (rc == -1) {
    return -1;
  }
  len += rc;

  for (int i = 0; i < sizeof(uintptr_t) * 2; i++, ptr <<= 4) {
    if (cb(digits[ptr >> (sizeof(uintptr_t) * 8 - 4)], arg)) {
      return -1;
    }
    len++;
  }
  return len;
}

static inline int
handle_char(char c, char padding, int width, int (*cb)(char ch, void *arg), void *arg) {
  int len = handle_padding(padding, width - 1, cb, arg);
  if (len == -1) {
    return -1;
  }
  if (cb(c, arg)) {
    return -1;
  }
  return len + 1;
}

static int
handle_format(const char *fmt, va_list ap, int (*cb)(char ch, void *arg), void *arg) {
  int len = 0;
  int ret;
  bool isfmt = false;
  int64_t num;
  uint64_t unum;

  for (; *fmt; fmt++) {
    char c = *fmt;
    char padding = ' ';
    size_t width = 0;
    int length = 0;
    if (isfmt) {
      if (c == '0') {
        padding = '0';
        c = *(++fmt);
      }
      while (c >= '0' && c <= '9') {
        width = width * 10 + c - '0';
        c = *(++fmt);
      }
      if (c == 'l') {
        length = 1;
        c = *(++fmt);
        if (c == 'l') {
          length = 2;
          c = *(++fmt);
        }
      } else if (c == 'z') {
        length = 3;
        c = *(++fmt);
      }
      switch (c) {
        case '%':
          ret = cb('%', arg) == -1 ? -1 : 1;
          break;
        case 'd':
        case 'i':
          switch (length) {
          case 0: num = va_arg(ap, int); break;
          case 1: num = va_arg(ap, long int); break;
          case 2: num = va_arg(ap, long long int); break;
          default: assert(0); break;
          }
          ret = handle_int(num, padding, width, cb, arg);
          break;
        case 'u':
          switch (length) {
          case 0: unum = va_arg(ap, unsigned); break;
          case 1: unum = va_arg(ap, long unsigned); break;
          case 2: unum = va_arg(ap, long long unsigned); break;
          default: assert(0); break;
          }
          ret = handle_uint(unum, 10, padding, width, cb, arg);
          break;
        case 's':
          ret = handle_str(va_arg(ap, const char *), padding, width, cb, arg);
          break;
        case 'c':
          ret = handle_char(va_arg(ap, int), padding, width, cb, arg);
          break;
        case 'x':
          switch (length) {
          case 0: unum = va_arg(ap, unsigned); break;
          case 1: unum = va_arg(ap, long unsigned); break;
          case 2: unum = va_arg(ap, long long unsigned); break;
          default: assert(0); break;
          }
          ret = handle_uint(va_arg(ap, unsigned int), 16, padding, width, cb, arg);
          break;
        case 'p':
          ret = handle_ptr(va_arg(ap, uintptr_t), padding, width, cb, arg);
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

static int vprint_handle_cb(char ch, void *arg) {
  putch(ch);
  return 0;
}

int vprintf(const char *fmt, va_list ap) {
  while (atomic_xchg(&locked, 1));
  int len = handle_format(fmt, ap, vprint_handle_cb, NULL);
  atomic_xchg(&locked, 0);
  return len;
}

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int len = vprintf(fmt, ap);
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
