#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  for (; s[len]; len++);
  return len;
}

char *strcpy(char *dst, const char *src) {
  size_t i;
  for (i = 0; src[i]; i++) {
    dst[i] = src[i];
  }
  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i;
  for (i = 0; i < n && src[i]; i++) {
    dst[i] = src[i];
  }
  for (; i < n; i++) {
    dst[i] = '\0';
  }
  return dst;
}

char *strcat(char *dst, const char *src) {
  char *p = dst;
  for (; *p; p++);
  strcpy(p, src);
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  for (; *s1 && *s1 == *s2; s1++, s2++);
  return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  if (n == 0) return 0;
  for (; n-- && *s1 && *s1 == *s2; s1++, s2++);
  return *s1 - *s2;
}

void *memset(void *s, int c, size_t n) {
  for (int i = 0; i < n; i++) {
    ((char *)s)[i] = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  if (src == dst) {
    return dst;
  } else if (src > dst) {
    for (size_t i = 0; i < n; i++) {
      ((char *)dst)[i] = ((char *)src)[i];
    }
  } else {
    for (size_t i = n - 1; i >= 0; i--) {
      ((char *)dst)[i] = ((char *)src)[i];
    }
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  for (size_t i = 0; i < n; i++) {
    ((char *)out)[i] = ((char *)in)[i];
  }
  return out;
}

int memcmp(const void *_s1, const void *_s2, size_t n) {
  if (n == 0) return 0;
  const char *s1 = _s1;
  const char *s2 = _s2;
  for (; n-- && *s1 == *s2; s1++, s2++);
  return *s1 - *s2;
}

#endif
