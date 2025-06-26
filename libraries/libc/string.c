#include "libraries/libc/string.h"

void strcpy(char *dest, const char *src)
{
	while ((*dest++ = *src++) != '\0');
}

void memcpy(uint8_t *dest, uint8_t *src, size_t size)
{
	ASSERT(size >= 0);
	for(int i = 0; i < size; i++)
		*dest++ = *src++;
}

void memset(uint8_t *src, size_t size)
{
	for(int i = 0; i < size; i++)
		*src++ = 0;
}

size_t strlen(const char *str)
{
	size_t size = 0;
	while(str[size] != 0) size++;
	return size;
}