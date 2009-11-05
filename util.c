#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "gimginfo.h"

const char *sint24_to_lat (int n)
{
	static char buffers[8][12];
	static int buffer_ptr = 0;
	char *buffer = buffers[buffer_ptr ++ % 8];

	assert(n < 0x800000 && n >= -0x800000);
	if (n < 0)
		sprintf(buffer, "%.5fS", n * (360.0 / 0x01000000));
	else
		sprintf(buffer, "%.5fN", n * (360.0 / 0x01000000));
	return buffer;
}

const char *sint24_to_lng (int n)
{
	static char buffers[8][12];
	static int buffer_ptr = 0;
	char *buffer = buffers[buffer_ptr ++ % 8];

	assert(n < 0x800000 && n >= -0x800000);
	if (n < 0)
		sprintf(buffer, "%.5fW", n * (360.0 / 0x01000000));
	else
		sprintf(buffer, "%.5fE", n * (360.0 / 0x01000000));
	return buffer;
}

const char *dump_unknown_bytes (uint8_t *bytes, int size)
{
	static char *buffer = NULL;
	static int buffer_size = 0;
	int ptr, outptr, repeat_count, repeat_byte;

	if (buffer == NULL) {
		buffer_size = size * 2 + 1 > 4096 ? size * 2 + 1 : 4096;
		buffer = (char *)malloc(buffer_size);
	} else if (buffer_size < size * 2 + 1) {
		buffer_size = size * 2 + 1;
		buffer = (char *)realloc(buffer, buffer_size);
	}

	for (repeat_byte = bytes[0], ptr = 1, repeat_count = 1, outptr = 0; ptr < size; ptr ++) {
		if (bytes[ptr] == repeat_byte) {
			repeat_count ++;
		} else {
			if (repeat_count >= 3) {
				outptr += sprintf(buffer + outptr, "%02x(%d)", repeat_byte, repeat_count);
			} else if (repeat_count == 2) {
				outptr += sprintf(buffer + outptr, "%02x%02x", repeat_byte, repeat_byte);
			} else {
				outptr += sprintf(buffer + outptr, "%02x", repeat_byte);
			}
			repeat_byte = bytes[ptr];
			repeat_count = 1;
		}
	}
	if (repeat_count >= 3) {
		outptr += sprintf(buffer + outptr, "%02x(%d)", repeat_byte, repeat_count);
	} else if (repeat_count == 2) {
		outptr += sprintf(buffer + outptr, "%02x%02x", repeat_byte, repeat_byte);
	} else {
		outptr += sprintf(buffer + outptr, "%02x", repeat_byte);
	}

	return buffer;
}

void unlockml (unsigned char *dst, const unsigned char *src,
		int size, unsigned int key)
{
	static const unsigned char shuf[] = {
		0xb, 0xc, 0xa, 0x0,
		0x8, 0xf, 0x2, 0x1,
		0x6, 0x4, 0x9, 0x3,
		0xd, 0x5, 0x7, 0xe};
	int i, ringctr;
	int key_sum = shuf[((key >> 24) + (key >> 16) + (key >> 8) + key) & 0xf];

	for (i = 0, ringctr = 16; i < size; i ++) {
		unsigned int upper = src[i] >> 4;
		unsigned int lower = src[i];

		upper -= key_sum;
		upper -= key >> ringctr;
		upper -= shuf[(key >> ringctr) & 0xf];
		ringctr = ringctr ? ringctr - 4 : 16;

		lower -= key_sum;
		lower -= key >> ringctr;
		lower -= shuf[(key >> ringctr) & 0xf];
		ringctr = ringctr ? ringctr - 4 : 16;

		dst[i] = ((upper << 4) & 0xf0) | (lower & 0xf);
	}
}

enum subtype get_subtype_id (const char *str) // only use 3 chars from str
{
	if (memcmp(str, "TRE", 3) == 0) return ST_TRE;
	if (memcmp(str, "RGN", 3) == 0) return ST_RGN;
	if (memcmp(str, "LBL", 3) == 0) return ST_LBL;
	if (memcmp(str, "NET", 3) == 0) return ST_NET;
	if (memcmp(str, "NOD", 3) == 0) return ST_NOD;
	if (memcmp(str, "SRT", 3) == 0) return ST_SRT;
	if (memcmp(str, "GMP", 3) == 0) return ST_GMP;
	if (memcmp(str, "TYP", 3) == 0) return ST_TYP;
	if (memcmp(str, "MDR", 3) == 0) return ST_MDR;
	if (memcmp(str, "TRF", 3) == 0) return ST_TRF;
	if (memcmp(str, "MPS", 3) == 0) return ST_MPS;
	if (memcmp(str, "QSI", 3) == 0) return ST_QSI;
	return ST_UNKNOWN;
}

const char *get_subtype_name (enum subtype id)
{
	const static char *type_names[] = {
		"TRE", "RGN", "LBL", "NET", "NOD",
		"SRT", "GMP", "TYP", "MDR", "TRF",
		"MPS", "QSI"};
	return id >= ST_UNKNOWN ? NULL : type_names[id];
}

/* remove space characters at the end
 * length = -1 means using strlen() */
void string_trim (char *str, int length)
{
	int i;
	if (length == -1)
		length = strlen(str);
	for (i = length - 1; i >= 0 && str[i] == ' '; i --)
		str[i] = '\0';
}
