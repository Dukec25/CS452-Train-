#include <string.h>
#include <debug.h>

int atoi(char *str)
{
	int num  = 0;
	int pos = 0;
	while (str[pos] >= '0' && str[pos] <= '9') {
		num = num * 10 + str[pos] - '0';
		pos += 1;
	}
	return num;
}

char c2x(char ch)
{
	if (ch <= 9) return '0' + ch;
	return 'a' + ch - 10;
}

int a2d(char ch)
{
	if(ch >= '0' && ch <= '9') return ch - '0';
	if(ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
	if(ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
	return -1;
}

char a2i(char ch, char **src, int base, int *nump)
{
	int num, digit;
	char *p;

	p = *src; num = 0;
	while((digit = a2d(ch)) >= 0) {
		if (digit > base) break;
		num = num*base + digit;
		ch = *p++;
	}
	*src = p; *nump = num;
	return ch;
}

void ui2a(unsigned int num, unsigned int base, char *bf)
{
	int n = 0;
	int dgt;
	unsigned int d = 1;
	
	while((num / d) >= base) d *= base;
	while(d != 0) {
		dgt = num / d;
		num %= d;
		d /= base;
		if(n || dgt > 0 || d == 0) {
			*bf++ = dgt + (dgt < 10 ? '0' : 'a' - 10);
			++n;
		}
	}
	*bf = 0;
}

void i2a(int num, char *bf)
{
	if(num < 0) {
		num = -num;
		*bf++ = '-';
	}
	ui2a(num, 10, bf);
}

char toupper(char c)
{
	if ((c >= 'a') && (c <= 'z')) {
		return 'A' + c - 'a';
	}
	return c;
}

int strcmp(char *str1, char *str2, size_t num)
{
	size_t i;
	for (i = 0; i < num; i++) {
		if (str1[i] < str2[i]) return -1;
		else if (str1[i] > str2[i]) return 1;
	}
	return 0;
}

int strlen(char *str)
{
	int i = 0;
	while(str[i] != '\0') {
		i++;
	}
	return i;
}

void *memcpy(void *destination, const void *source, size_t num)
{
	void *dest_pos = (void *)destination;
	void *src_pos = (void *)source;
	debug(DEBUG_ALL, "memcpy dest_pos = 0x%x, src_pos = 0x%x, num = %x, src = %s", dest_pos, src_pos, num, src_pos);
	while(num > 0) {
	/*	if (num >= sizeof(uint64)) {
			*((uint64 *)dest_pos) = *((uint64 *)src_pos);
			debug(DEBUG_ALL, "memcpy strlen %d using uint64, *dest_pos = 0x%x, dest_pos = 0x%x", num, *((uint64 *)dest_pos), dest_pos);
			dest_pos += sizeof(uint64);	
			src_pos += sizeof(uint64);
			num -= sizeof(uint64);
		}
		else if (num >= sizeof(uint32)) {
			*((uint32 *)dest_pos) = *((uint32 *)src_pos);
			debug(DEBUG_ALL, "memcpy strlen %d using uint32, *dest_pos = 0x%x, dest_pos = 0x%x", num, *((uint32 *)dest_pos), dest_pos);
			dest_pos += sizeof(uint32);	
			src_pos += sizeof(uint32);
			num -= sizeof(uint32);
		}
		else if (num >= sizeof(uint16)) {
			*((uint16 *)dest_pos) = *((uint16 *)src_pos);
			debug(DEBUG_ALL, "memcpy strlen %d using uint16, *dest_pos = 0x%x, dest_pos = 0x%x", num, *((uint16 *)dest_pos), dest_pos);
			dest_pos += sizeof(uint16);	
			src_pos += sizeof(uint16);
			num -= sizeof(uint16);
		}
		else */ {
			*((uint8 *)dest_pos) = *((uint8 *)src_pos);
			debug(DEBUG_ALL, "memcpy strlen %d using uint8, *dest_pos = 0x%x, dest_pos = 0x%x", num, *((uint8 *)dest_pos), dest_pos);
			dest_pos += sizeof(uint8);
			src_pos += sizeof(uint8);
			num -= sizeof(uint8);
		}
	}
	return destination;
}
