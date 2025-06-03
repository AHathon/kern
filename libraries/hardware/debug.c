#include "libraries/hardware/debug.h"

static void print_dec(unsigned long value, unsigned int width, char * buf, int * ptr ) {
	unsigned int n_width = 1;
	unsigned long i = 9;
	while (value > i && i < UINT64_MAX) {
		n_width += 1;
		i *= 10;
		i += 9;
	}

	int printed = 0;
	while (n_width + printed < width) {
		buf[*ptr] = '0';
		*ptr += 1;
		printed += 1;
	}

	i = n_width;
	while (i > 0) {
		unsigned long n = value / 10;
		int r = value % 10;
		buf[*ptr + i - 1] = r + '0';
		i--;
		value = n;
	}
	*ptr += n_width;
}

static void print_hex(unsigned long value, unsigned int width, char * buf, int * ptr, char upper) {
	long i = width;
	char *charset = upper ? "0123456789ABCDEF" : "0123456789abcdef";

	if (i == 0) i = 8;

	unsigned int n_width = 1;
	unsigned long j = 0x0F;
	while (value > j && j < UINT64_MAX) {
		n_width += 1;
		j *= 0x10;
		j += 0x0F;
	}

	while (i > (int)n_width) {
		buf[*ptr] = '0';
		*ptr += 1;
		i--;
	}

	i = (int)n_width;
	while (i-- > 0) {
		buf[*ptr] = charset[(value>>(i*4))&0xF];
		*ptr += + 1;
	}
}

size_t vasprintf(char * buf, const char *fmt, va_list args) {
	char *s;
	int ptr = 0;
	while (*fmt) {
		
		if (*fmt != '%') {
			buf[ptr++] = *fmt++;
			continue;
		}
		++fmt;
		unsigned argwid = 0;
		while (*fmt >= '0' && *fmt <= '9') {
			argwid *= 10;
			argwid += *fmt - '0';
			++fmt;
		}
		
		switch (*fmt) {
			case 's':
				s = (char *)va_arg(args, char *);
				while (*s) {
					buf[ptr++] = *s++;
				}
				break;
			case 'c': 
				buf[ptr++] = (char)va_arg(args, int); 
				break;
			case 'x': 
				print_hex((unsigned long)va_arg(args, unsigned long), argwid, buf, &ptr, 0); 
				break;
			case 'X': 
				print_hex((unsigned long)va_arg(args, unsigned long), argwid, buf, &ptr, 1); 
				break;
			case 'd': 
				print_dec((unsigned long)va_arg(args, unsigned long), argwid, buf, &ptr); 
				break;
			case '%': 
				buf[ptr++] = '%'; 
				break;
			default: 
				buf[ptr++] = *fmt; 
				break;
		}
		fmt++;
		if(ptr >= 0xFF) break;
	}
	buf[ptr] = '\0';
	
	return ptr;

}

void kprintf(char *fmt, ...) {
	char buf[256];
	va_list list;
	va_start(list, fmt);
	vasprintf(buf, fmt, list);
	uart1_puts(buf);
	va_end(list);
}

void kHexDump(uint8_t *buf, size_t size)
{
	for(int i = 0; i < size; i++)
	{
		if(i % 16 == 0)
			kprintf("%08X | ", ((uintptr_t)buf) + i);
		kprintf("%02X ", buf[i]);
		if(i % 16 == 15) 
			kprintf("\n");
	}
}

void kstrcpy(uint8_t *dest, uint8_t *src)
{
	while ((*dest++ = *src++) != '\0');
}

void kmemcpy(uint8_t *dest, uint8_t *src, size_t size)
{
	for(int i = 0; i < size; i++)
		*dest++ = *src++;
}

void kmemset(uint8_t *src, size_t size)
{
	for(int i = 0; i < size; i++)
		*src++ = 0;
}