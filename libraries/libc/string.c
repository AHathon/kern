#include "libc/string.h"

void *memcpy(void *dest, const void *src, size_t n)
{
	uint8_t *d = dest;
	const uint8_t *s = src;
	for(size_t i = 0; i < n; i++)
		d[i] = s[i];
	return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
	uint8_t *d = dest;
	const uint8_t *s = src;
	if(d < s)
		return memcpy(dest, src, n);
	while(n--)
		d[n] = s[n];
	return dest;
}

void *memset(void *dest, int c, size_t n)
{
	uint8_t *d = dest;
	for(size_t i = 0; i < n; i++)
		d[i] = (uint8_t)c;
	return dest;
}

int memcmp(const void *a, const void *b, size_t n)
{
	const uint8_t *x = a;
	const uint8_t *y = b;
	for(size_t i = 0; i < n; i++)
		if(x[i] != y[i])
			return x[i] - y[i];
	return 0;
}

void *memchr(const void *s, int c, size_t n)
{
	const uint8_t *p = s;
	for(size_t i = 0; i < n; i++)
		if(p[i] == (uint8_t)c)
			return (void*)(p + i);
	return 0;
}

size_t strlen(const char *s)
{
	size_t n = 0;
	while(s[n])
		n++;
	return n;
}

char *strcpy(char *dest, const char *src)
{
	char *d = dest;
	while((*d++ = *src++));
	return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	size_t i = 0;
	for(; i < n && src[i]; i++)
		dest[i] = src[i];
	for(; i < n; i++)
		dest[i] = 0;
	return dest;
}

int strcmp(const char *a, const char *b)
{
	while(*a && *a == *b)
	{
		a++;
		b++;
	}
	return (uint8_t)*a - (uint8_t)*b;
}

int strncmp(const char *a, const char *b, size_t n)
{
	for(size_t i = 0; i < n; i++)
	{
		if(a[i] != b[i] || !a[i])
			return (uint8_t)a[i] - (uint8_t)b[i];
	}
	return 0;
}

char *strchr(const char *s, int c)
{
	for(;; s++)
	{
		if(*s == (char)c)
			return (char*)s;
		if(!*s)
			return 0;
	}
}

char *strrchr(const char *s, int c)
{
	const char *found = 0;
	for(;; s++)
	{
		if(*s == (char)c)
			found = s;
		if(!*s)
			return (char*)found;
	}
}

char *strstr(const char *haystack, const char *needle)
{
	size_t n = strlen(needle);
	for(; *haystack; haystack++)
		if(!strncmp(haystack, needle, n))
			return (char*)haystack;
	return n ? 0 : (char*)haystack;
}
