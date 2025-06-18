#include "libraries/hardware/debug.h"

static void print_dec(uint64_t value, uint32_t width, char * buf, int32_t * ptr ) {
	unsigned int n_width = 1;
	uint64_t i = 9;
	while (value > i && i < UINT64_MAX) {
		n_width += 1;
		i *= 10;
		i += 9;
	}

	int printed = 0;
	while (n_width + printed < width) {
		buf[*ptr] = '0';
		(*ptr)++;
		printed += 1;
	}

	i = n_width;
	while (i > 0) {
		uint64_t n = value / 10;
		int r = value % 10;
		buf[*ptr + i - 1] = r + '0';
		i--;
		value = n;
	}
	*ptr += n_width;
}

static void print_hex(uint64_t value, uint32_t width, char * buf, int32_t * ptr, char upper) {
	long i = width;
	char *charset = upper ? "0123456789ABCDEF" : "0123456789abcdef";

	if (i == 0) i = 8;

	uint32_t n_width = 1;
	uint64_t j = 0x0F;
	while (value > j && j < UINT64_MAX) {
		n_width += 1;
		j *= 0x10;
		j += 0x0F;
	}

	while (i > (int32_t)n_width) {
		buf[*ptr] = '0';
		(*ptr)++;
		i--;
	}

	i = (int32_t)n_width;
	while (i-- > 0) {
		buf[*ptr] = charset[(value >> (i*4)) & 0xF];
		(*ptr)++;
	}
}

size_t vasprintf(char * buf, const char *fmt, va_list args) {
	char *s;
	int32_t ptr = 0;
	while (*fmt) {
		
		if (*fmt != '%') {
			buf[ptr++] = *fmt++;
			continue;
		}
		++fmt;
		uint32_t argwid = 0;
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
				print_hex((uint64_t)va_arg(args, uint64_t), argwid, buf, &ptr, 0); 
				break;
			case 'X': 
				print_hex((uint64_t)va_arg(args, uint64_t), argwid, buf, &ptr, 1); 
				break;
			case 'd': 
				print_dec((uint64_t)va_arg(args, uint64_t), argwid, buf, &ptr); 
				break;
			case '%': 
				buf[ptr++] = '%'; 
				break;
			default: 
				buf[ptr++] = *fmt; 
				break;
		}
		fmt++;
		if(ptr >= 128) break;
	}
	buf[ptr] = '\0';
	
	return ptr;

}

void panic() {
    LOG("System halted.\n");
    while (1) {
        asm volatile("wfe");
    }
}

void kprintf(char *fmt, ...) {
	static char buf[128];
	va_list list;
	va_start(list, fmt);
	vasprintf(buf, fmt, list);
	uart0_puts(buf);
	va_end(list);
}

void kHexDump(uint8_t *buf, size_t size)
{
	for(int i = 0; i < size; i++)
	{
		if(i % 16 == 0)
			LOG("%08X | ", ((uintptr_t)buf) + i);
		LOG("%02X ", buf[i]);
		if(i % 16 == 15 || i >= size - 1) 
			LOG("\n");
	}
}

void kstrcpy(char *dest, const char *src)
{
	ASSERT((uintptr_t)dest >= KERNEL_VIRT_BASE);
	ASSERT((uintptr_t)src >= KERNEL_VIRT_BASE);
	while ((*dest++ = *src++) != '\0');
}

void kmemcpy(uint8_t *dest, uint8_t *src, size_t size)
{
	ASSERT(dest >= KERNEL_VIRT_BASE);
	ASSERT(src >= KERNEL_VIRT_BASE);
	ASSERT(size >= 0);
	for(int i = 0; i < size; i++)
		*dest++ = *src++;
}

void kmemset(uint8_t *src, size_t size)
{
	ASSERT(src >= KERNEL_VIRT_BASE);
	for(int i = 0; i < size; i++)
		*src++ = 0;
}

size_t kstrlen(const char *str)
{
	size_t size = 0;
	while(str[size] != 0) size++;
	return size;
}