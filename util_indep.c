#include <stdio.h>
#include <stdlib.h>
#include "util_indep.h"

void hexdump (const unsigned char *data, int size)
{
	while (size -- > 0)
		printf("%02x", *(data ++));
	printf("\n");
}

unsigned int read_byte_at (FILE *fp, off_t offset)
{
	int c;
	if (myfseek64(fp, offset)) {
		perror(NULL);
		exit(1);
	}
	c = getc(fp);
	if (c == EOF)
		errexit("Unexpected EOF\n");
	return c;
}

unsigned int read_2byte_at (FILE *fp, off_t offset)
{
	unsigned char buff[2];
	if (myfseek64(fp, offset)) {
		perror(NULL);
		exit(1);
	}
	if (fread(&buff, 2, 1, fp) != 1) {
		perror(NULL);
		exit(1);
	}
	return (buff[1] << 8) | buff[0];
}

unsigned int read_4byte_at (FILE *fp, off_t offset)
{
	unsigned char buff[4];
	if (myfseek64(fp, offset)) {
		perror(NULL);
		exit(1);
	}
	if (fread(&buff, 4, 1, fp) != 1) {
		perror(NULL);
		exit(1);
	}
	return (buff[3] << 24) | (buff[2] << 16) | (buff[1] << 8) | buff[0];
}

void read_bytes_at (FILE *fp, off_t offset, unsigned char *buffer, int size)
{
	if (myfseek64(fp, offset)) {
		perror(NULL);
		exit(1);
	}
	if (fread(buffer, size, 1, fp) != 1) {
		perror(NULL);
		exit(1);
	}
}
